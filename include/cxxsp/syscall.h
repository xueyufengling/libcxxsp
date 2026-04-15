#ifndef _CXXSP_SYSCALL
#define _CXXSP_SYSCALL

#include <cxxsp/arch.h>
#include <cxxsp/mem.h>
#include <string>
#include <unordered_map>

#if defined(__OS_WIN__)
#include <windows.h>
#endif

namespace cxxsp
{

#if defined(__OS_WIN__)

IMAGE_DOS_HEADER* __win_ntdll_dos();

#endif

extern void retrieve_syscall_addrs(std::unordered_map<std::string, void*>& syscall_addrs);

extern void retrieve_syscall_nums(std::unordered_map<std::string, int>& syscall_nums);

extern int syscall_num(const char* name);

#if defined(__OS_WIN__)
typedef NTSTATUS syscall_ret_t;
#else
typedef long syscall_ret_t;
#endif

typedef syscall_ret_t (*syscall_t)(...);

extern os_memory retrieve_syscalls(std::unordered_map<std::string, int>& syscall_nums, std::unordered_map<std::string, syscall_t>& syscalls);

extern syscall_t syscall(const char* name);

}

#endif //_CXXSP_SYSCALL
