#include <cxxsp/syscall.h>

using namespace cxxsp;

#if defined(__OS_WIN__)

IMAGE_DOS_HEADER* cxxsp::__win_ntdll_dos()
{
	return (IMAGE_DOS_HEADER*)(__win_peb_load_order_module(__win_peb(), 1)->DllBase); //ntdll始终为进程PEB里加载的第二个模块
}

#endif
