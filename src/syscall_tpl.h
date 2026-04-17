#ifndef _CXXSP_SYSCALLTPL
#define _CXXSP_SYSCALLTPL

#include <cxxsp/arch.h>

// 系统调用号的占位符，运行时需要替换该占位符为正确的调用号
#define __SYSCALL_NO_PLACEHOLDER__ (('s' << 24) | ('c' << 16) |  ('n' << 8) | 'o')

#define __SYSCALL_CLEAN_PLACEHOLDER__ (0xc1ea)

#define __WIN_KUSER_SHARED_DATA__ 0x7FFE0300
#define __WIN_SYSCALL_TEST_OFFSET__ 8

#if defined(__OS_WIN__)
#define __SYSCALL_RET_NUM__ 2
#endif

#endif //_CXXSP_SYSCALLTPL
