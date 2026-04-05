#ifndef _CXXSP_ASMDEF
#define _CXXSP_ASMDEF

/**
 * 指令操作
 */

#include <cxxsp/asm.h>
#include <stdint.h>

namespace cxxsp
{

#if defined(__ARCH_X86_64__) || defined(__ARCH_X86__)
/**
 * @brief 将rip或eip寄存器中的指令地址储存到C/C++变量
 * 		  ip寄存器不能直接读写，只能通过jmp写，通过pop读
 */
#define __rip__(dest)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(),\
			"call 1f",\
			"1: pop %" __asm_var__(dest)\
		)

/**
 * @brief jmp指令将C/C++变量值写入ip寄存器
 */
#define __jmp__(dest)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(r, (void*)dest)),\
			"jmp *%0"\
		)

/**
 * @brief 加载值到指定位置
 * @param dest 目标位置
 * @param src 要加载的值，可以是立即值或C/C++变量，这些src值都会被加载进临时寄存器，随后再从临时寄存器移入目标位置
 */
#define __ldb__(dest, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(r, (uint8_t)src)),\
			"movb %0, %" __str__(dest)\
		)

#define __ldw__(dest, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(r, (uint16_t)src)),\
			"movw %0, %" __str__(dest)\
		)

#define __ldl__(dest, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(r, (uint32_t)src)),\
			"movl %0, %" __str__(dest)\
		)

#define __ldq__(dest, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(r, (uint64_t)src)),\
			"movq %0, %" __str__(dest)\
		)

/**
 * @brief 从指定位置读取值
 * @param src 要读取的位置
 * @param dest 目标位置，必须是C/C++变量
 */
#define __stb__(dest, src)\
		__asm_inline__(volatile, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(),\
			"movb %" __str__(src) ", %" __asm_var__(dest)\
		)

#define __stw__(dest, src)\
		__asm_inline__(volatile, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(),\
			"movw %" __str__(src) ", %" __asm_var__(dest)\
		)

#define __stl__(dest, src)\
		__asm_inline__(volatile, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(),\
			"movl %" __str__(src) ", %" __asm_var__(dest)\
		)

#define __stq__(dest, src)\
		__asm_inline__(volatile, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(),\
			"movq %" __str__(src) ", %" __asm_var__(dest)\
		)

/**
 * @brief 读段寄存器，寄存器以%开头
 * 		  有效段寄存器为CS, SS, DS, ES, FS, GS
 */
#define __stsrb__(dest, sr, off)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(__asm_inp__(m, off)),\
			"movb %" __str__(sr) ":%0, %" __asm_var__(dest)\
		)

/**
 * @brief 写段寄存器
 */
#define __ldsrb__(sr, off, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(m, off), __asm_inp__(m, (uint8_t)src)),\
			"movb %1, %" __str__(sr) ":%0"\
		)

#define __stsrw__(dest, sr, off)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(__asm_inp__(m, off)),\
			"movw %" __str__(sr) ":%0, %" __asm_var__(dest)\
		)

#define __ldsrw__(sr, off, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(m, off), __asm_inp__(m, (uint16_t)src)),\
			"movw %1, %" __str__(sr) ":%0"\
		)

#define __stsrl__(dest, sr, off)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(__asm_out__(dest, m)),\
			__asm_list__(__asm_inp__(m, off)),\
			"movl %" __str__(sr) ":%0, %" __asm_var__(dest)\
		)

#define __ldsrl__(sr, off, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(m, off), __asm_inp__(m, (uint32_t)src)),\
			"movl %1, %" __str__(sr) ":%0"\
		)

#define __stsrq__(dest, sr, off)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(__asm_outp__(r, dest)),\
			__asm_list__(__asm_inp__(m, off)),\
			"movq %" __str__(sr) ":%1, %0"\
		)

#define __ldsrq__(sr, off, src)\
		__asm_inline__(optimized, var)(\
			__asm_list__(),\
			__asm_list__(),\
			__asm_list__(__asm_inp__(m, off), __asm_inp__(m, (uint64_t)src)),\
			"movq %1, %" __str__(sr) ":%0"\
		)

/**
 * @brief 32位寻址访问段寄存器。
 * 		  在64位模式下，FS段寄存器和GS段寄存器寻址方式不同，读取GS段寄存器只能使用32位寄存器间接寻址，即使用eax等32位寄存器。且输出也必须是寄存器不能是内存地址。
 */
#define __stsrq32__(dest, sr, off)\
		__asm_inline__(optimized, var)(\
			__asm_list__("eax"),\
			__asm_list__(__asm_outp__(r, dest)),\
			__asm_list__(__asm_inp__(m, off)),\
			"movl %1, %%eax",\
			"movq %" __str__(sr) ":(%%eax), %0"\
		)

#define __stsrl32__(dest, sr, off)\
		__asm_inline__(optimized, var)(\
			__asm_list__("eax"),\
			__asm_list__(__asm_outp__(r, dest)),\
			__asm_list__(__asm_inp__(m, off)),\
			"movl %1, %%eax",\
			"movl %" __str__(sr) ":(%%eax), %0"\
		)

#endif

/**
 * @brief	比较并交换1、2、4、8字节的值
 * @detail	待比较的期望值必须送入寄存器a，这是因为cmpxchg指令的比较操作数只从寄存器a取。src操作数必须是寄存器
 * 			注：不使用inline函数是因为inline函数虽然内联函数体，但每个inline内联展开的参数和局部变量都会独立地、额外地占用栈空间，且也会进行对这些栈内存进行初始化和参数传递，从而生成一堆不必要的mov指令
 */
#if defined(__ARCH_X86_64__) || defined(__ARCH_X86__)
#define __cmpxchgb__(dest, expected, src, prev)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(prev, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgb %" __asm_var__(src) ", %" __asm_var__(dest),\
				"movb %%al, %" __asm_var__(prev)\
		)

#define __casb__(dest, expected, src, success)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(success, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgb %" __asm_var__(src) ", %" __asm_var__(dest),\
				"setz %" __asm_var__(success)\
		)

#define __cmpxchgw__(dest, expected, src, prev)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(prev, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgw %" __asm_var__(src) ", %" __asm_var__(dest),\
				"movw %%ax, %" __asm_var__(prev)\
		)

#define __casw__(dest, expected, src, success)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(success, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgw %" __asm_var__(src) ", %" __asm_var__(dest),\
				"setz %" __asm_var__(success)\
		)

#define __cmpxchgl__(dest, expected, src, prev)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(prev, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgl %" __asm_var__(src) ", %" __asm_var__(dest),\
				"movl %%eax, %" __asm_var__(prev)\
		)

#define __casl__(dest, expected, src, success)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(success, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgl %" __asm_var__(src) ", %" __asm_var__(dest),\
				"setz %" __asm_var__(success)\
		)

#define __cmpxchgq__(dest, expected, src, prev)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(prev, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgq %" __asm_var__(src) ", %" __asm_var__(dest),\
				"movq %%rax, %" __asm_var__(prev)\
		)

#define __casq__(dest, expected, src, success)\
		__asm_inline__(volatile, var)(\
				__asm_list__("memory", "cc"),\
				__asm_list__(__asm_inout__(dest, m), __asm_out__(success, m)),\
				__asm_list__(__asm_in__(expected, a), __asm_in__(src, r)),\
				"lock cmpxchgq %" __asm_var__(src) ", %" __asm_var__(dest),\
				"setz %" __asm_var__(success)\
		)
#endif

// ***** 封装指令函数 *****

/**
 * @brief 编译器屏障，强制编译器在优化阶段指令重排时在此语句之前完成内存写入，且此语句之后的内存读取不得提前。
 * 		  原理：使用memory标志通知编译器此语句会破坏内存，防止指令重排后内存访问顺序更改
 */
__attribute__((always_inline)) inline void __cb(void)
{
	__asm_inline__(volatile, var)(
			__asm_list__("memory"),
			__asm_list__(),
			__asm_list__(),
			""
	);
}

/**
 * @brief 全内存屏障
 */
__attribute__((always_inline)) inline void __rwmb(void)
{
	__asm_inline__(volatile, var)(
			__asm_list__("memory"),
			__asm_list__(),
			__asm_list__(),
#if defined(__ARCH_X86_64__) || defined(__ARCH_X86__)
			"mfence"
#elif defined(__ARCH_AARCH_64__)
			"dmb sy"
#elif defined(__ARCH_POWERPC_64__)
			"lwsync"
#else
			static_assert(false, "no full memory barrier instruction for current architecture");
#endif
	);
}

/**
 * @brief 读内存屏障
 */
__attribute__((always_inline)) inline void __rmb(void)
{
	__asm_inline__(volatile, var)(
			__asm_list__("memory"),
			__asm_list__(),
			__asm_list__(),
#if defined(__ARCH_X86_64__) || defined(__ARCH_X86__)
			"lfence"
#elif defined(__ARCH_AARCH_64__)
			"dmb ld"
#else
			static_assert(false, "no read memory barrier instruction for current architecture");
#endif
	);
}

/**
 * @brief 写内存屏障
 */
__attribute__((always_inline)) inline void __wmb(void)
{
	__asm_inline__(volatile, var)(
			__asm_list__("memory"),
			__asm_list__(),
			__asm_list__(),
#if defined(__ARCH_X86_64__) || defined(__ARCH_X86__)
			"sfence"
#elif defined(__ARCH_AARCH_64__)
			"dmb st"
#else
			static_assert(false, "no write memory barrier instruction for current architecture");
#endif
	);
}

/**
 * @brief 获取当前栈帧正在执行的指令地址。
 * 		  必须将此函数内联到目标函数中，否则获取到的ip就不是目标函数的ip。
 */
__attribute__((always_inline)) inline void* __ip()
{
	void* ip;
#if defined(__ARCH_X86_64__) || defined(__ARCH_X86__)
	__rip__(ip);
#elif defined(__ARCH_AARCH_64__)
	__stq__(ip, %pc);
#endif
	return ip;
}

__attribute__((always_inline)) inline void __jmp(void* target_ip)
{
	__jmp__(target_ip);
}

#if defined(__ARCH_X86_64__) || defined(__ARCH_X86__)

/**
 * @brief 读FS寄存器
 */
__attribute__((always_inline)) inline void* __rfs(uint64_t offset)
{
	void* fsv;
#if defined(__ARCH_X86_64__)
	__stsrq__(fsv, %fs, offset);
#elif defined(__ARCH_X86__)
	__stsrl__(fsv, %fs, offset);
#elif defined(__ARCH_AARCH_64__)

#endif
	return fsv;
}

/**
 * @brief 写FS寄存器
 */
__attribute__((always_inline)) inline void __wfs(uint64_t offset, void* value)
{
#if defined(__ARCH_X86_64__)
	__ldsrq__(%fs, offset, value);
#elif defined(__ARCH_X86__)
	__ldsrl__(%fs, offset, value);
#elif defined(__ARCH_AARCH_64__)

#endif
}


__attribute__((always_inline)) inline void* __rdgsbase()
{
	void* base;
	__asm_inline__(volatile, var)(
			__asm_list__(),
			__asm_list__(__asm_out__(base, m)),
			__asm_list__(),
			"rdgsbase %" __asm_var__(base)
	);
	return base;
}

/**
 * @brief 读GS寄存器中的指针
 */
__attribute__((always_inline)) inline void* __rgs(uint32_t offset)
{
	void* gsv;
#if defined(__ARCH_X86_64__)
	__stsrq32__(gsv, %gs, offset);
#elif defined(__ARCH_X86__)
	__stsrl32__(gsv, %gs, offset);
#elif defined(__ARCH_AARCH_64__)

#endif
	return gsv;
}

/**
 * @brief 写GS寄存器
 */
__attribute__((always_inline)) inline void __wgs(uint64_t offset, void* value)
{
#if defined(__ARCH_X86_64__)
	__ldsrq__(%gs, offset, value);
#elif defined(__ARCH_X86__)
	__ldsrl__(%gs, offset, value);
#elif defined(__ARCH_AARCH_64__)

#endif
}

#endif

/**
 * @brief 机器的栈，栈是反向生长的，栈顶在低地址，栈底在高地址
 */
namespace stack
{
/**
 * @brief 栈帧
 */
typedef struct frame
{
	frame* caller; //当前读到的rbp寄存器值，指向上一个栈帧的栈底
	void* ret_ip; //高地址，实际对应栈中prev下方的值，即返回地址，该地址是caller调用函数结束后的下一条指令地址，即caller的ip

	//不允许构造和析构，只能通过读取rbp寄存器的值来解析栈
	frame() = delete;
	~frame() = delete;

	/**
	 * @brief 栈帧的局部变量区域，使用字节作为偏移量单位。局部变量按照声明顺序从高地址到低地址排列。
	 */
	__attribute__((always_inline)) inline void* local_addr(long long offset)
	{
		return ((unsigned char*)this - offset);
	}

	__attribute__((always_inline)) inline long long caller_size()
	{
		return (unsigned char*)caller - (unsigned char*)this; //两个栈底之间的距离即栈帧空间
	}
} frame;

/**
 * @brief 当前栈底，即当前的栈帧。当前栈底的地址正是caller栈顶的地址。
 */
__attribute__((always_inline)) inline frame* __bp()
{
	frame* bottom;
#if defined(__ARCH_X86_64__)
	__stq__(bottom, %rbp);
#elif defined(__ARCH_X86__)
	__stl__(bottom, %ebp);
#elif defined(__ARCH_AARCH_64__)
	__stq__(bottom, %fp);
#endif
	return bottom;
}

/**
 * @brief 栈顶，在push %rbp;压栈保存caller栈地址后，会先压栈调用前要保存的寄存器值，以便执行完毕后恢复现场，在此期间rsp寄存器的栈底持续增长，这部分是函数序言。
 * 		  函数序言之后，才为局部变量分配内存。栈帧内的内存是连续的，局部变量、保存的现场寄存器值、rbp、返回值地址都在栈上。
 * 		  函数结束返回时进入函数尾声，mov %ebp, %esp;将结束的这个函数栈帧的栈底ebp作为caller的栈顶esp。
 */
__attribute__((always_inline)) inline void* __sp()
{
	void* top;
#if defined(__ARCH_X86_64__)
	__stq__(top, %rsp);
#elif defined(__ARCH_X86__)
	__stl__(top, %esp);
#elif defined(__ARCH_AARCH_64__)
	__stq__(top, %sp);
#endif
	return top;
}

/**
 * @brief 当前执行的栈帧的当前长度，rsp寄存器在调用过程中会随着函数序言压栈现场寄存器值和分配局部变量内存而增长，进入函数本体后保持不变。
 */
__attribute__((always_inline)) inline long long callee_size()
{
	return (unsigned char*)__bp() - (unsigned char*)__sp();
}

}

}

#endif //_CXXSP_ASMDEF
