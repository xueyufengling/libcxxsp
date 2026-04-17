#include "syscall_tpl.h"

.rodata:
//导出的汇编时常量
.globl __syscall_no_offset
__syscall_no_offset:
	.quad (__syscall_no_offset_value)

.globl __syscall_size
__syscall_size:
	.quad (__syscall_size_value)

.globl __syscall_cl_offset
__syscall_cl_offset:
	.quad (__syscall_cl_offset_value)

.text:

//记录当前位置向前n字节的偏移量
#define __offset_markb__(out, n) .equ out, . - __syscall_tpl - n
//标记函数大小
#define __syscall_endp__ __offset_markb__(__syscall_size_value, 0)
//标记调用号偏移量
#define __syscall_noff__(size) __offset_markb__(__syscall_no_offset_value, size)
//标记不需要被调用者清理栈
#define __caller_clean__ .equ __syscall_cl_offset_value, 0
//标记被调用者自行清理栈
#define __callee_clean__ __offset_markb__(__syscall_cl_offset_value, 2)

.globl __syscall_tpl

#位置无关的syscall调用函数，参考ntdll.dll反汇编结果
__syscall_tpl:

#if defined(__ARCH_X86__)
#64位架构，64位Windows
#if defined(__OS_WIN64__)
	movq %rcx, %r10															#Windows调用约定参数为rcx, rdx, r8, r9，剩余参数压栈；syscall调用约定为r10, rdx, r8, r9，剩余参数压栈.因此需要手动将函数参数rcx移入syscall参数寄存器r10
	movl $__SYSCALL_NO_PLACEHOLDER__, %eax									#移入调用号
	__syscall_noff__(4)														#记录syscall number偏移量
	testb $0x1, %ds:__WIN_KUSER_SHARED_DATA__ + __WIN_SYSCALL_TEST_OFFSET__	#检查DS段寄存器0x7FFE0308位置处的数据_KUSER_SHARED_DATA*+8与0x1的与运算结果，即检查最低位是否为0
	jnz 1f																	#最低位ZF非0，则跳转到中断实现的syscall，而非直接使用syscall指令
	syscall																	#现代使用syscall指令进行系统调用
	retq
1:
	int $0x2E																#旧时代遗留的兼容方案，使用中断进行系统调用，中断号固定为0x2E
	retq
	__caller_clean__														#x64 windows不需要被调用者清理栈

#32位Windows
#elif defined(__OS_WIN32__)
#stdcall，参数全压栈
	movl $__SYSCALL_NO_PLACEHOLDER__, %eax
	__syscall_noff__(4)
	testb $0x1, %ds:__WIN_KUSER_SHARED_DATA__ + __WIN_SYSCALL_TEST_OFFSET__
	jnz 1f
	sysenter
	jmp 2f
1:
	int $0x2E
2:
	ret $__SYSCALL_CLEAN_PLACEHOLDER__										#stdcall需要被调用者清理栈，执行后esp+=clean_size
	__callee_clean__														#ret指令接收2字节整数作为清理栈的大小

#endif
#endif
	__syscall_endp__
