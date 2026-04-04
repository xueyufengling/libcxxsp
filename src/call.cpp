#include <cxxsp/call.h>

#include <cxxsp/asm_def.h>

#include <malloc.h>
#include <unwind.h>

#if defined(__OS_WIN__)
#include <windows.h>
#elif defined(__OS_UNIX__)
#include <sys/mman.h>
#endif

using namespace cxxsp;

/**
 * @brief 从Instruction Pointer获取该指令所属的函数的函数指针地址
 */
void* cxxsp::ip_function(void* ip)
{
	return ::_Unwind_FindEnclosingFunction(ip); //Itanium ABI提供的语言无关实现
}

#if defined(__OS_WIN__)

void* cxxsp::__win_teb()
{
#if defined(__OS_WIN32__)
	return __rfs(0x18);
#elif defined(__OS_WIN64__)
	void* teb;
	__rgs(teb, 0x30);
	return teb;
#endif
}

void* cxxsp::__win_peb()
{
#if defined(__OS_WIN32__)
	return __rfs(0x30);
#elif defined(__OS_WIN64__)
	void* peb;
	__rgs(peb, 0x60);
	return peb;
#endif
}

#endif

exec_memory exec_memory::alloc(size_t size)
{
	exec_memory mem;
	mem.mem_size = size;
#if defined(__OS_WIN__)
	mem.mem = ::VirtualAlloc(
			NULL, size,
			MEM_COMMIT | MEM_RESERVE,
			PAGE_EXECUTE_READWRITE
			);
#elif defined(__OS_UNIX__)
	mem.mem= mmap(
			NULL, size,
			PROT_EXEC | PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0
	);
#endif
	return mem;
}

void exec_memory::free()
{
#if defined(__OS_WIN__)
	::VirtualFree(mem, 0, MEM_RELEASE);
#elif defined(__OS_UNIX__)
	munmap(mem, mem_size);
#endif
	mem = nullptr;
	mem_size = 0;
}

/**
 * @brief 强制关闭内联且必须生成栈帧，关闭优化，防止构造函数没有栈帧导致错误。
 * 		  如果不强制保留栈帧指针rbp，则构造函数序言为
 * 		  push    rsi
 * 		  push    rbx
 * 		  sub     rsp, 38h
 * 		  此种形式未将rbp压栈，而是直接移动rsp，将导致rbp依然是caller的rbp，但ip却是该构造函数的ip
 */
__attribute__((noinline, optimize("no-omit-frame-pointer"), optimize("O0"))) call_stack::call_stack(int unwind_depth)
{
	frame_count = 0;
	frames = (frame*)malloc(sizeof(frame) * unwind_depth);
	stack::frame* stack_frame = stack::__bp(); //本构造函数的栈帧
	while(frame_count < unwind_depth)
	{
		frame& unwind_frame = frames[frame_count];
		unwind_frame.frame_depth = frame_count;
		unwind_frame.bp_addr = (void**)&stack_frame->caller;
		unwind_frame.ip_addr = &stack_frame->ret_ip;
		++frame_count;
		// rbp < __OS_RESERVED_ADDR__则代表当前已经是最底层的栈帧
		stack::frame* caller = stack_frame->caller;
		if(caller > __OS_RESERVED_ADDR__ && stack_frame->ret_ip)
		{
			stack_frame = caller; //如果该栈帧还有caller则继续迭代
		}
		else
		{
			break;
		}
	}
}

call_stack::~call_stack()
{
	free(frames);
	frames = nullptr;
}

void call_stack::jmp(void* ip)
{
	__jmp(ip);
}
