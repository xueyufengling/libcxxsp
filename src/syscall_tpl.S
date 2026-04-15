#include "syscall_tpl.h"

#位置无关的syscall调用函数

.text:

.globl __syscall_tpl

#if defined(__OS_WIN64__)

__syscall_tpl:
	movq %rcx, %r10															#Windows调用约定参数为rcx, rdx, r8, r9，剩余参数压栈；syscall调用约定为r10, rdx, r8, r9，剩余参数压栈.因此需要手动将函数参数rcx移入syscall参数寄存器r10
	movl $__SYSCALL_NUM_PLACEHOLDER__, %eax									#移入调用号
	testb $0x1, %ds:__WIN_KUSER_SHARED_DATA__ + __WIN_SYSCALL_TEST_OFFSET__	#检查DS段寄存器0x7FFE0308位置处的数据_KUSER_SHARED_DATA*+8与0x1的与运算结果，即检查最低位是否为0
	jnz 1f																	#最低位ZF非0，则跳转到中断实现的syscall，而非直接使用syscall指令
	syscall																	#现代使用syscall指令进行系统调用
	retq
1:
	int $0x2E																#旧时代遗留的兼容方案，使用中断进行系统调用，中断号固定为0x2E
	retq

#endif
