#include <cxxsp/syscall.h>

#include <llvmmci/mci.h>
#include <cxxsp/call.h>
#include <regex>
#include "syscall_tpl.h"

using namespace cxxsp;

// syscall_tpl.S导出符号
extern "C"
{
//.rodata
extern const long __syscall_no_offset; //调用号的偏移量
extern const long __syscall_size; //syscall函数大小
extern const long __syscall_cl_offset;

//.text
syscall_ret_t __syscall_tpl(...);
}

syscall_t cxxsp::mem_alloc_syscall = nullptr;

//非基本类型或指针的全局变量需要调用构造函数，属于动态初始化，在静态初始化之后执行，这期间动态初始化、constructor都会执行，因此需要设定好优先级以先执行动态初始化
static uint8_t* syscall_tpl_addr = (uint8_t*)(&__syscall_tpl); //syscall模板函数体的地址
__attribute__((init_priority(101))) static std::unordered_map<std::string, int> syscall_nums;
__attribute__((init_priority(101))) static std::unordered_map<std::string, syscall_t> syscalls_by_name;
__attribute__((init_priority(101))) static std::unordered_map<int, syscall_t> syscalls_by_num;

static inline syscall_t __build_os_syscall(long syscall_num)
{
	return syscall(os_memory::os_alloc(nullptr, __syscall_size, memory_flag::MEM_PROT_RWX), syscall_num);
}

static inline syscall_t __build_os_syscall(const char* name)
{
	return __build_os_syscall(syscall_num(name));
}

static inline syscall_t __build_syscall(long syscall_num)
{
	return syscall(os_memory::syscall_alloc(nullptr, __syscall_size, memory_flag::MEM_PROT_RWX), syscall_num);
}

//初始化syscall。llvmmci.so库必须在此函数之前先加载，如果使用导入表，则操作系统保证so库先于本库初始化
__attribute__((constructor(101))) void __init_syscall()
{
	retrieve_syscall_nums(syscall_nums);
	//全局内存分配函数
	//使用操作系统API分配NtAllocateVirtualMemory函数内存作为启动点，后续分配内存都只通过此全局函数指针分配
#ifdef __OS_WIN__
	cxxsp::mem_alloc_syscall = __build_os_syscall("NtAllocateVirtualMemory");
#endif
	//构造syscall函数，重新构造而非直接调用系统库的syscall的原因为防止系统库被外部注入
	for(auto& scn : syscall_nums)
	{
		syscall_t sc = __build_syscall(scn.second);
		//存入对应名称的syscall函数及其stub地址
		syscalls_by_name[scn.first] = sc;
		syscalls_by_num[scn.second] = sc;
	}
}

#if defined(__OS_WIN__)

IMAGE_DOS_HEADER* cxxsp::__win_ntdll_dos()
{
	return (IMAGE_DOS_HEADER*)(__win_peb_load_order_module(__win_peb(), 1)->DllBase); //ntdll始终为进程PEB里加载的第二个模块
}

#endif

void cxxsp::retrieve_os_syscall_addrs(std::unordered_map<std::string, void*>& syscall_addrs)
{
#if defined(__OS_WIN__)
	IMAGE_DOS_HEADER* dos_header = __win_ntdll_dos();
	unsigned char* base = (unsigned char*)dos_header;
	IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(base + dos_header->e_lfanew);
	DWORD export_entry_offset = nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if(export_entry_offset)
	{
		IMAGE_EXPORT_DIRECTORY* export_entries = (IMAGE_EXPORT_DIRECTORY*)(base + export_entry_offset);
		DWORD name_count = export_entries->NumberOfNames;
		USHORT* ordinals = (USHORT*)(base + export_entries->AddressOfNameOrdinals);
		DWORD* names = (DWORD*)(base + export_entries->AddressOfNames);
		DWORD* func_addrs = (DWORD*)(base + export_entries->AddressOfFunctions);
		for(int ascii_idx = 0; ascii_idx < name_count; ++ascii_idx)
		{
			const char* name = (const char*)(base + names[ascii_idx]);
			if(!strncmp(name, "Nt", 2) && strncmp(name, "Ntdll", 5))
			{
				syscall_addrs[name] = base + func_addrs[ordinals[ascii_idx]]; //对应函数的指针
			}
		}
	}
#endif
}

void cxxsp::retrieve_syscall_nums(std::unordered_map<std::string, int>& syscall_nums)
{
	std::unordered_map<std::string, void*> syscall_addrs;
	retrieve_os_syscall_addrs(syscall_addrs);
	for(auto& sc : syscall_addrs)
	{
		uint64_t func_base = (uint64_t)(sc.second);
		uint64_t func_size = disassembler_find_return(host_disassembler, sc.second, __OS_PAGE_SIZE__, func_base, __SYSCALL_RET_NUM__) - func_base + 1; //函数长度，最长支持__OS_PAGE_SIZE__
		array* src = disassemble_text(host_disassembler, sc.second, func_size, func_base);
		std::regex pattern(R"(movl?\s*\$(\d+),\s*%[er]ax[\s\S]*?syscall)", //提取syscall前最后一个mov $xxx, %eax中的xxx，即调用号
				std::regex::ECMAScript | std::regex::icase); //跨行匹配
		std::smatch match;
		std::string asm_src((const char*)src->data);
		if(std::regex_search(asm_src, match, pattern))
		{
			syscall_nums[sc.first] = std::stoi(match[1].str());
		}
		free_array(src);
	}
}

int cxxsp::syscall_num(const char* name)
{
	return syscall_nums[name];
}

syscall_t cxxsp::syscall(void* mem, long syscall_num, unsigned short syscall_clean)
{
	memcpy(mem, syscall_tpl_addr, __syscall_size); //每个调用号都有自己的syscall函数内存
	*(int*)((uint8_t*)mem + __syscall_no_offset) = syscall_num; //替换模板中的syscall number
	if(__syscall_cl_offset)
		*(unsigned short*)((uint8_t*)mem + __syscall_cl_offset) = syscall_clean; //如果需要自己清理栈，则填充清理的大小
	return (syscall_t)mem;
}

syscall_t cxxsp::syscall(long syscall_num)
{
	syscall_t sc = syscalls_by_num[syscall_num]; //先获取缓存的已创建syscall函数
	if(!sc)
	{
		sc = __build_syscall(syscall_num); //缓存函数不存在则新建
		syscalls_by_num[syscall_num] = sc;
	}
	return sc;
}

syscall_t cxxsp::syscall(const char* name)
{
	return syscalls_by_name[name];
}
