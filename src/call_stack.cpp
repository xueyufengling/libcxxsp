#include <cxxsp/call_stack.h>

#include <cxxsp/asm_def.h>

#include <malloc.h>
#include <unwind.h>
#include<iostream>
using namespace cxxsp;

/**
 * @brief 从Instruction Pointer获取该指令所属的函数的函数指针地址
 */
void* cxxsp::ip_function(void* ip)
{
	return ::_Unwind_FindEnclosingFunction(ip); //Itanium ABI提供的语言无关实现
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
