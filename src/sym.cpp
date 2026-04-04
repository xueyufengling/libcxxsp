#include <cxxsp/sym.h>

#include <cxxsp/arch.h>

#if defined(__OS_WIN__)
#include <windows.h>
#elif defined(__OS_UNIX__)
#include <dlfcn.h>
#endif

#include <malloc.h>
#include <string.h>
#include <cxxabi.h>

using namespace cxxsp;

char* cxxsp::cxx_name_demangling(const char* mangled_name)
{
	int status = 0;
	char* name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
	if(status)
		return nullptr;
	else
		return name;
}

/**
 * @brief Windows系统查找一个偏移量对应的导出符号的序号
 * @param export_entries 导出条目的指针
 * @param 目标偏移量，即符号地址-dll加载基地址
 */
static int __win_find_addr_export_ordinal(IMAGE_DOS_HEADER* dos_header, long long target_offset)
{
	unsigned char* base = (unsigned char*)dos_header;
	IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(base + dos_header->e_lfanew); //base指针实际指向DOS头，通过DOS头访问NT头
	DWORD exprot_entry_offset = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if(exprot_entry_offset) //有导出表
	{
		IMAGE_EXPORT_DIRECTORY* export_entries = (IMAGE_EXPORT_DIRECTORY*)(base + exprot_entry_offset);
		DWORD* sym_addrs = (DWORD*)(base + export_entries->AddressOfFunctions); //导出的全部函数、变量偏移量，采用的索引是AddressOfNameOrdinals中储存的序号
		DWORD sym_count = export_entries->NumberOfFunctions; //导出的地址个数
		DWORD match_offset = 0; //最佳匹配的符号偏移量
		int match_ordinal = -1;
		for(DWORD sym_ordinal = 0; sym_ordinal < sym_count; ++sym_ordinal) //必须遍历全部符号，符号地址可能是乱序的
		{
			DWORD current_sym = sym_addrs[sym_ordinal];
			if(match_offset < current_sym && current_sym <= target_offset) //判断符号地址位于哪个导出符号的区间
			{
				match_offset = current_sym;
				match_ordinal = sym_ordinal;
			}
		}
		return match_ordinal;
	}
	else
	{
		return -1;
	}
}

static int __win_find_ordinal_export_name_idx(IMAGE_DOS_HEADER* dos_header, int ordinal)
{
	unsigned char* base = (unsigned char*)dos_header;
	IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(base + dos_header->e_lfanew);
	DWORD exprot_entry_offset = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if(exprot_entry_offset) //有导出表
	{
		IMAGE_EXPORT_DIRECTORY* export_entries = (IMAGE_EXPORT_DIRECTORY*)(base + exprot_entry_offset);
		DWORD name_count = export_entries->NumberOfNames; //导出的名称个数
		USHORT* ordinals = (USHORT*)(base + export_entries->AddressOfNameOrdinals); //导出的符号序号，16位无符号整数，索引采用字典序，对应的值是ordinal
		for(int ascii_idx = 0; ascii_idx < name_count; ++ascii_idx)
		{
			if(ordinals[ascii_idx] == ordinal)
			{
				return ascii_idx;
			}
		}
		return -1; //未找到
	}
	else
	{
		return -1;
	}
}

symbol symbol::resolve(void* symbol_addr)
{
	symbol sym;
#if defined(__OS_WIN__)
	MEMORY_BASIC_INFORMATION mbi;
	if(::VirtualQuery(symbol_addr, &mbi, sizeof(mbi))) //查找地址所属模块
	{
		sym.base = mbi.AllocationBase;
		unsigned char* base = (unsigned char*)sym.base;
		char bin_path_buf[MAX_PATH] = {0};
		::GetModuleFileNameA((HMODULE)base, bin_path_buf, MAX_PATH);
		size_t bin_path_len = strlen(bin_path_buf);
		char* bin_path = (char*)malloc(bin_path_len + 1);
		bin_path[bin_path_len] = '\0';
		sym.binary = strcpy(bin_path, bin_path_buf);
		IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew); //base指针实际指向DOS头，通过DOS头访问NT头
		DWORD exprot_entry_offset = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		if(exprot_entry_offset) //有导出表
		{
			IMAGE_EXPORT_DIRECTORY* export_entries = (IMAGE_EXPORT_DIRECTORY*)(base + exprot_entry_offset);
			int match_ordinal = __win_find_addr_export_ordinal((IMAGE_DOS_HEADER*)base, (unsigned char*)symbol_addr - base);
			if(match_ordinal >= 0)
			{
				sym.address = base + ((DWORD*)(base + export_entries->AddressOfFunctions))[match_ordinal]; //加载基地址+符号偏移量
				int ascii_idx = __win_find_ordinal_export_name_idx((IMAGE_DOS_HEADER*)base, match_ordinal);
				if(ascii_idx >= 0)
				{
					//AddressOfNames为导出的全部函数、变量名称偏移量，索引采用ASCII字典序而不是ordinal
					sym.name = (const char*)(base + ((DWORD*)(base + export_entries->AddressOfNames))[ascii_idx]); //直接返回库内存驻留的名称，而不是重新分配
				};
			}
		}
	}
#elif defined(__OS_UNIX__)
	Dl_info dl_info;
	if(::dladdr(symbol_addr, &dl_info))
	{
		sym.binary = dl_info.dli_fname; //线程不安全，每次调用时重复使用
		sym.name = dl_info.dli_sname; //直接返回的系统驻留的Module中的符号名称指针
		sym.base = (void*)dl_info.dli_fbase;
		sym.address = (void*)dl_info.dli_saddr;
	}
#endif
	return sym;
}

void symbol::free()
{
#if defined(__OS_WIN__)
	::free((void*)binary); //二进制文件路径为resolve()新分配的字符串，需要释放
#elif defined(__OS_UNIX__)
	//unix下binary、name都是内存驻留的字符串，不需要释放
#endif
}

shared_object shared_object::dlopen(const char* so_path)
{
#if defined(__OS_WIN__)
	return shared_object(::LoadLibraryExA(so_path, NULL, DONT_RESOLVE_DLL_REFERENCES));
#elif defined(__OS_UNIX__)
	return shared_object(::dlopen(so_path, RTLD_LAZY));
#endif
}

void shared_object::dlclose()
{
	if(handle)
	{
#if defined(__OS_WIN__)
		::FreeLibrary((HMODULE)handle);
#elif defined(__OS_UNIX__)
		::dlclose(handle);
#endif
		handle = nullptr;
	}
}

void* shared_object::dlsym(const char* symbol_name)
{
	if(handle)
	{
#if defined(__OS_WIN__)
		return (void*)::GetProcAddress((HMODULE)handle, symbol_name);
#elif defined(__OS_UNIX__)
	return ::dlsym(handle, symbol_name);
#endif
	}
	else
	{
		return nullptr;
	}
}

void* shared_object::base()
{
#if defined(__OS_WIN__)
	return handle; //Windows下handle就是基地址
#elif defined(__OS_UNIX__)
#endif
}
