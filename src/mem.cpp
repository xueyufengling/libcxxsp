#include <cxxsp/mem.h>

#include <cxxsp/arch.h>

#if defined(__OS_WIN__)
#include <windows.h>
#elif defined(__OS_UNIX__)
#include <sys/mman.h>
#endif

#include <cxxsp/syscall.h>

using namespace cxxsp;

#if defined(__OS_WIN__)
static DWORD __to_os_mem_protect(memory_flag flags)
{
	bool read = flag_bit(flags, memory_flag::MEM_PROT_READ);
	bool write = flag_bit(flags, memory_flag::MEM_PROT_WRITE);
	bool exec = flag_bit(flags, memory_flag::MEM_PROT_EXEC);
	if(read && write && exec)
		return PAGE_EXECUTE_READWRITE;
	if(read && write && !exec)
		return PAGE_READWRITE;
	if(read && !write && exec)
		return PAGE_EXECUTE_READ;
	if(read && !write && !exec)
		return PAGE_READONLY;
	if(!read && !write && exec)
		return PAGE_EXECUTE;
	return PAGE_NOACCESS;
}

static DWORD __to_os_mem_flags(memory_flag flags)
{
	DWORD result = MEM_COMMIT | MEM_RESERVE; //预留内存并立即分配
	if(flag_bit(flags, memory_flag::MEM_FLAG_TOP_DOWN))
		result |= MEM_TOP_DOWN;
	if(flag_bit(flags, memory_flag::MEM_FLAG_LARGE_PAGES))
		result |= MEM_LARGE_PAGES;
	if(flag_bit(flags, memory_flag::MEM_FLAG_PHYSICAL))
		result |= MEM_PHYSICAL;
	if(flag_bit(flags, memory_flag::MEM_FLAG_RESET))
		result |= MEM_RESET;
	return result;
}
#elif defined(__OS_UNIX__)
static int __to_os_mem_protect(memory_flag flags)
{
	bool read = flag_bit(flags, memory_flag::MEM_PROT_READ);
	bool write = flag_bit(flags, memory_flag::MEM_PROT_WRITE);
	bool exec = flag_bit(flags, memory_flag::MEM_PROT_EXEC);
	int prot = 0;
	if(read)
		prot |= PROT_READ;
	if(write)
		prot |= PROT_WRITE;
	if(exec)
		prot |= PROT_EXEC;
	return prot;
}

int __to_os_mem_flags(memory_flag flags)
{
    int result = MAP_PRIVATE | MAP_ANONYMOUS;
    if (flag_bit(flags, MEM_FLAG_LOCKED))   result |= MAP_LOCKED;
    if (flag_bit(flags, MEM_FLAG_HUGETLB))  result |= MAP_HUGETLB;
    if (flag_bit(flags, MEM_FLAG_POPULATE)) result |= MAP_POPULATE;
    if (flag_bit(flags, MEM_FLAG_FIXED))    result |= MAP_FIXED;
    if (flag_bit(flags, MEM_FLAG_SHARED))   result |= MAP_SHARED;
    if (flag_bit(flags, MEM_FLAG_SHARED)) {
        result &= ~MAP_PRIVATE;
    }
    return result;
}
#endif

void* cxxsp::os_allocate_memory(void* base, size_t size, memory_flag flags)
{
#if defined(__OS_WIN__)
	return ::VirtualAlloc(
			base, size,
			__to_os_mem_flags(flags),
			__to_os_mem_protect(flags));
#elif defined(__OS_UNIX__)
	return mmap(
			base, size,
			__to_os_mem_protect(flags),
			__to_os_mem_flags(flags),
			-1, 0);
#endif
}

long cxxsp::syscall_allocate_memory(void*& base, size_t& size, memory_flag flags)
{
#if defined(__OS_WIN__)
	static syscall_t NtAllocateVirtualMemory = cxxsp::syscall("NtAllocateVirtualMemory");
	return NtAllocateVirtualMemory(
			::GetCurrentProcess(), //当前进程
			&base,
			(ULONG_PTR)0,
			(PSIZE_T)&size,
			__to_os_mem_flags(flags),
			__to_os_mem_protect(flags));
#endif
}

os_memory os_memory::os_alloc(void* base, size_t size, memory_flag flags)
{
	os_memory mem;
	size_t* os_mem = (size_t*)os_allocate_memory(base, sizeof(size_t) + size, flags);
	*os_mem = size;
	mem.mem = os_mem + 1;
	return mem;
}

os_memory os_memory::syscall_alloc(void* base, size_t size, memory_flag flags)
{
	os_memory mem;
	size_t actual_size = sizeof(size_t) + size; //右边为期望大小，分配实际大小可能大于此值
	syscall_allocate_memory(mem.mem, actual_size, flags);
	size_t* os_mem = (size_t*)mem.mem;
	*os_mem = actual_size;
	mem.mem = os_mem + 1;
	return mem;
}

void os_memory::free()
{
#if defined(__OS_WIN__)
	::VirtualFree(header(), 0, MEM_RELEASE);
#elif defined(__OS_UNIX__)
	munmap(header(), size());
#endif
	mem = nullptr;
}
