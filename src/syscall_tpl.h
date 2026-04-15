#ifndef _CXXSP_SYSCALLTPL
#define _CXXSP_SYSCALLTPL

#include <cxxsp/arch.h>

// 系统调用号的占位符，运行时需要替换该占位符为正确的调用号
#define __SYSCALL_NUM_PLACEHOLDER__ 0xDEADBEEF

#define __WIN_KUSER_SHARED_DATA__ 0x7FFE0300
#define __WIN_SYSCALL_TEST_OFFSET__ 0x8

#endif //_CXXSP_SYSCALLTPL
