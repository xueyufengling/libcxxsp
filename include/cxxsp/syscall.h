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

extern void retrieve_os_syscall_addrs(std::unordered_map<std::string, void*>& syscall_addrs);

extern void retrieve_syscall_nums(std::unordered_map<std::string, int>& syscall_nums);

extern int syscall_num(const char* name);

#if defined(__OS_WIN__)
typedef NTSTATUS syscall_ret_t;
#else
typedef long syscall_ret_t;
#endif

typedef syscall_ret_t (*syscall_t)(...);

extern syscall_t mem_alloc_syscall; //内存分配函数的syscall

/**
 * @brief 根据调用号在指定内存位置构造一个syscall函数。
 * 		  该内存必须有执行权限才能调用。
 */
extern syscall_t syscall(void* mem, long syscall_num);

/**
 * @brief 根据调用号构造一个syscall函数，如果已经获取过该函数则返回缓存副本
 * 		  直接使用syscall指令进行内存分配，如果操作系统库被注入，此函数也不会受到影响
 */
extern syscall_t syscall(long syscall_num);

/**
 * @brief 根据名称查找syscall函数，指定名称不存在则返回nullptr
 */
extern syscall_t syscall(const char* name);
}

#endif //_CXXSP_SYSCALL
