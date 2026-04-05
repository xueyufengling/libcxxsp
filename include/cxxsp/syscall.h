#ifndef _CXXSP_SYSCALL
#define _CXXSP_SYSCALL

#include <cxxsp/call.h>

#if defined(__OS_WIN__)
#include <windows.h>
#endif

namespace cxxsp
{

#if defined(__OS_WIN__)

IMAGE_DOS_HEADER* __win_ntdll_dos();

#endif

}

#endif //_CXXSP_SYSCALL
