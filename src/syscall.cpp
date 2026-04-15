#include <cxxsp/syscall.h>

#include <llvmmci/mci.h>
#include <cxxsp/call.h>
#include <regex>
#include "syscall_tpl.h"

using namespace cxxsp;

extern "C"
{
syscall_ret_t __syscall_tpl(...);
}

//非基本类型或指针的全局变量需要调用构造函数，属于动态初始化，在静态初始化之后执行，这期间动态初始化、constructor都会执行，因此需要设定好优先级以先执行动态初始化
__attribute__((init_priority(101))) static std::unordered_map<std::string, int> syscall_nums;
__attribute__((init_priority(101))) static os_memory syscalls_mem;
__attribute__((init_priority(101))) static std::unordered_map<std::string, syscall_t> syscalls;

//初始化syscall。llvmmci.so库必须在此函数之前先加载，如果使用导入表，则操作系统保证so库先于本库初始化
__attribute__((constructor(101))) void __init_syscall()
{
	retrieve_syscall_nums(syscall_nums);
	syscalls_mem = retrieve_syscalls(syscall_nums, syscalls);
}

#if defined(__OS_WIN__)

IMAGE_DOS_HEADER* cxxsp::__win_ntdll_dos()
{
	return (IMAGE_DOS_HEADER*)(__win_peb_load_order_module(__win_peb(), 1)->DllBase); //ntdll始终为进程PEB里加载的第二个模块
}

#endif

void cxxsp::retrieve_syscall_addrs(std::unordered_map<std::string, void*>& syscall_addrs)
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
	retrieve_syscall_addrs(syscall_addrs);
	for(auto& sc : syscall_addrs)
	{
		uint64_t func_base = (uint64_t)(sc.second);
		uint64_t func_size = disassembler_find_return(host_disassembler, sc.second, __OS_PAGE_SIZE__, func_base, 1) - func_base + 1; //函数长度，最长支持__OS_PAGE_SIZE__
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

os_memory cxxsp::retrieve_syscalls(std::unordered_map<std::string, int>& syscall_nums, std::unordered_map<std::string, syscall_t>& syscalls)
{
	uint8_t* func_addr = (uint8_t*)(&__syscall_tpl);
	uint64_t func_base = (uint64_t)func_addr;
	uint64_t func_size = disassembler_find_return(host_disassembler, func_addr, __OS_PAGE_SIZE__, func_base, 2) - func_base + 1; //windows下第二个ret才是函数结尾
	uint64_t syscall_num_offset = 0; //调用号的偏移量
	for(; syscall_num_offset < func_size; ++syscall_num_offset)
	{
		uint8_t* addr = func_addr + syscall_num_offset;
		if(*(int*)addr == __SYSCALL_NUM_PLACEHOLDER__)
			break;
	}
	os_memory mem = os_memory::os_alloc(nullptr, syscall_nums.size() * func_size, memory_flag::MEM_PROT_RWX); //只分配一次内存，该内存包含全部的syscall函数
	size_t idx = 0;
	for(auto& scn : syscall_nums)
	{
		uint8_t* syscall_stub_addr = (uint8_t*)mem + idx * func_size;
		memcpy(syscall_stub_addr, func_addr, func_size);
		*(int*)(syscall_stub_addr + syscall_num_offset) = scn.second; //替换模板中的syscall number
		syscalls[scn.first] = (syscall_t)syscall_stub_addr; //存入对应名称的syscall函数及其stub地址
		++idx;
	}
	return mem;
}

syscall_t cxxsp::syscall(const char* name)
{
	return syscalls[name];
}
