#ifndef _CXXSP_CALL
#define _CXXSP_CALL

#include <stddef.h>
#include <cxxsp/arch.h>

namespace cxxsp
{
/**
 * @brief 从Instruction Pointer获取该指令所属的函数的函数指针地址
 */
extern void* ip_function(void* ip);

#if defined(__OS_WIN__)

extern void* __win_teb();

extern void* __win_peb();

#endif

/**
 * @brief 拥有执行权限的内存，内存区间内的机器码可以执行。
 * 		  标准库的内存分配只能分配具有RW权限的数据内存，无法分配具有E权限的可执行内存，这必须依赖操作系统API。
 */
class exec_memory
{
private:
	void* mem = nullptr;
	size_t mem_size = 0;

	exec_memory() = default;

public:
	static exec_memory alloc(size_t size);

	template<typename _T>
	inline operator _T*()
	{
		return (_T*)mem;
	}

	inline size_t size()
	{
		return mem_size;
	}

	void free();
};

/**
 * @brief 栈帧回溯
 */
class call_stack
{
public:
	typedef struct frame
	{
		friend call_stack;

	private:
		int frame_depth;
		void** bp_addr;
		void** ip_addr;

	public:
		inline int depth()
		{
			return frame_depth;
		}

		/**
		 * @brief 栈帧rbp。
		 */
		inline void* bp()
		{
			return *bp_addr;
		}

		inline void set_bp(void* bp)
		{
			if(frame_depth)
				*bp_addr = bp;
		}

		/**
		 * @brief 栈帧ip。如果是depth=0的顶层栈帧，则是调用call_stack构造函数处的ip
		 */
		inline void* ip()
		{
			// https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
			// 一些机器上储存的返回地址经过了编码，需要使用内建函数解码真实返回地址。
			return __builtin_extract_return_addr(*ip_addr);
		}

		inline void set_ip(void* ip)
		{
			if(frame_depth)
				*ip_addr = ip; //顶层栈帧不能设置ip。当从构造函数返回回到顶层栈帧时，顶层栈帧的ip已经从栈中弹出。
		}
	} frame;

private:
	int frame_count = 0;  //实际的栈帧数目
	frame* frames = nullptr; //栈帧数组

public:
	/**
	 * @brief 栈帧回溯。回溯得到的顶部栈帧是调用此构造函数的函数，构造函数本身的栈帧不包含在内。
	 * 		  得到的call_stack对象仅在调用此构造函数的函数内有效，随着外部函数的逐级返回，栈帧数据从顶至底依次失效。
	 * @param unwind_depth 回溯的最大深度
	 */
	__attribute__((noinline, optimize("no-omit-frame-pointer"), optimize("O0"))) call_stack(int unwind_depth);

	/**
	 * @brief 释放资源
	 */
	~call_stack();

	inline int count()
	{
		return frame_count;
	}

	/**
	 * @brief 获取指定栈深度的栈帧函数指针
	 */
	inline frame& at(int depth)
	{
		return frames[depth];
	}

	inline frame& operator[](int depth)
	{
		return at(depth);
	}

	inline void* ip(int depth)
	{
		return at(depth).ip();
	}

	inline void* function(int depth)
	{
		return ip_function(ip(depth));
	}

	/**
	 * @brief 当前执行跳转到目标ip
	 */
	static void jmp(void* ip);
};

}

#endif //_CXXSP_CALL
