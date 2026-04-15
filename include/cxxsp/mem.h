#ifndef _CXXSP_MEM
#define _CXXSP_MEM

#include <stddef.h>

namespace cxxsp
{
template<typename _T>
inline constexpr _T flag(int bit)
{
	return 0x1 << bit;
}

template<typename _T>
inline constexpr bool flag_bit(_T flags, _T flag)
{
	return flags & flag;
}

template<typename _T>
inline constexpr _T set_flag(_T flags, _T flag, bool mark)
{
	return mark ? flags | flag : flags & (~flag);
}

typedef int memory_flag_t;

enum memory_flag : memory_flag_t
{
	MEM_PROT_NOACC = 0x0,
	MEM_PROT_READ = flag<memory_flag_t>(0),
	MEM_PROT_WRITE = flag<memory_flag_t>(1),
	MEM_PROT_EXEC = flag<memory_flag_t>(2), //拥有执行权限的内存，内存区间内的机器码可以执行。标准库的内存分配只能分配具有RW权限的数据内存，无法分配具有E权限的可执行内存，这必须依赖操作系统API
	MEM_PROT_RW = MEM_PROT_READ | MEM_PROT_WRITE,
	MEM_PROT_RX = MEM_PROT_READ | MEM_PROT_EXEC,
	MEM_PROT_RWX = MEM_PROT_READ | MEM_PROT_WRITE | MEM_PROT_EXEC,

	// Windows标志
	MEM_FLAG_TOP_DOWN = flag<memory_flag_t>(3),   // 高地址优先
	MEM_FLAG_LARGE_PAGES = flag<memory_flag_t>(4),   // 大页
	MEM_FLAG_PHYSICAL = flag<memory_flag_t>(5),   // 物理内存
	MEM_FLAG_RESET = flag<memory_flag_t>(6),   // 重置内存

	// Linux标志
	MEM_FLAG_LOCKED = flag<memory_flag_t>(7),   // 锁定内存
	MEM_FLAG_HUGETLB = flag<memory_flag_t>(8),   // 大页
	MEM_FLAG_POPULATE = flag<memory_flag_t>(9),   // 预填充
	MEM_FLAG_FIXED = flag<memory_flag_t>(10),  // 固定地址
	MEM_FLAG_SHARED = flag<memory_flag_t>(11),  // 共享内存
};

/**
 * @brief 使用操作系统API分配内存
 * @param base 期望分配的地址，传入nullptr则由操作系统选择空闲地址
 * @param size 期望分配的内存大小
 * @param flags 分配的内存标志
 */
void* os_allocate_memory(void* base, size_t size, memory_flag flags = memory_flag::MEM_PROT_RWX);

/**
 * @brief 使用syscall分配内存
 * @param base 期望分配的地址，传入nullptr则由操作系统选择空闲地址
 * @param size 期望分配的内存大小，本函数返回后为考虑到对齐以后的实际大小
 * @param flags 分配的内存标志
 * @return 错误码
 */
long syscall_allocate_memory(void*& base, size_t& size, memory_flag flags = memory_flag::MEM_PROT_RWX);

/**
 * @brief 操作系统内存包装，应当使用此API进行操作系统的内存分配
 */
class os_memory
{
private:
	void* mem = nullptr;

public:
	static os_memory os_alloc(void* base, size_t size, memory_flag flags = memory_flag::MEM_PROT_RWX);

	static os_memory syscall_alloc(void* base, size_t size, memory_flag flags = memory_flag::MEM_PROT_RWX);

	template<typename _T>
	inline operator _T*()
	{
		return (_T*)mem;
	}

	inline void* header()
	{
		return (size_t*)mem - 1;
	}

	inline size_t size()
	{
		return *(size_t*)header();
	}

	void free();
};

}

#endif //_CXXSP_MEM
