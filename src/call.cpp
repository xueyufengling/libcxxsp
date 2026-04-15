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

TEB* cxxsp::__win_teb()
{
#if defined(__OS_WIN32__)
	return (TEB*)__rfs(0x18);
#elif defined(__OS_WIN64__)
	return (TEB*)__rgs(0x30);
#endif
}

PEB* cxxsp::__win_peb()
{
#if defined(__OS_WIN32__)
	return (PEB*)__rfs(0x30);
#elif defined(__OS_WIN64__)
	return (PEB*)__rgs(0x60);
#endif
}

LDR_DATA_TABLE_ENTRY* cxxsp::__win_peb_load_order_module(PEB* peb, int load_order)
{
	/**
	 * [Type: _PEB_LDR_DATA *]
	 * [+0x000] Length           : 0x58 [Type: unsigned long]
	 * [+0x004] Initialized      : 0x1 [Type: unsigned char]
	 * [+0x008] SsHandle         : 0x0 [Type: void *]
	 * [+0x010] InLoadOrderModuleList [Type: _LIST_ENTRY]
	 * [+0x020] InMemoryOrderModuleList [Type: _LIST_ENTRY]
	 * [+0x030] InInitializationOrderModuleList [Type: _LIST_ENTRY]
	 * [+0x040] EntryInProgress  : 0x0 [Type: void *]
	 * [+0x048] ShutdownInProgress : 0x0 [Type: unsigned char]
	 * [+0x050] ShutdownThreadId : 0x0 [Type: void *]
	 */
	LIST_ENTRY& InLoadOrderModuleList = *(LIST_ENTRY*)((unsigned char*)&peb->Ldr->InMemoryOrderModuleList - sizeof(LIST_ENTRY)); //InLoadOrderModuleList未公开，需要根据WinDbg查看的实际结构体手动计算偏移量
	LIST_ENTRY* module = InLoadOrderModuleList.Flink;
	for(int idx = 0; idx < load_order; ++idx)
	{
		module = module->Flink;
	}
	return (LDR_DATA_TABLE_ENTRY*)module; //直接将链表指针强转即可
}

#endif

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
