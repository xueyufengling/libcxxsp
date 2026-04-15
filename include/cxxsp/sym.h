#ifndef _CXXSP_SYM
#define _CXXSP_SYM

/**
 * 提供so库的加载、符号查找和栈帧回溯
 * 编译可执行文件时，需要添加-export-dynamic导出符号供查询，链接时需要添加-ldl，其库文件位于msys64/mingw64/lib/libdl.a
 */
#include <tplmp/tplmp.h>
#include <typeinfo>

#include <cxxsp/arch.h>

#if defined(__OS_WIN__)
#include <windows.h>
#elif defined(__OS_UNIX__)
#endif

namespace cxxsp
{
/**
 * @brief 将C++经过name mangling的名字恢复
 * @return 成功返回name mangling之前的名字，失败返回nullptr
 */
extern char* cxx_name_demangling(const char* mangled_name);

__attribute__((always_inline)) inline char* cxx_name_demangling(const std::type_info& t)
{
	return cxx_name_demangling(t.name());
}

template<typename _T>
__attribute__((always_inline)) inline char* cxx_name_demangling()
{
	return cxx_name_demangling(typeid(tplmp::decl<_T>::ref()));
}

#if defined(__OS_WIN__)

int __win_find_addr_export_ordinal(IMAGE_DOS_HEADER* dos_header, long long target_offset);

int __win_find_ordinal_export_name_idx(IMAGE_DOS_HEADER* dos_header, int ordinal);

#endif

/**
 * 符号相关信息
 */
typedef struct symbol
{
	const char* binary = nullptr; //符号所属二进制文件路径
	const char* name = nullptr; //符号名称，如果是C++名称则是mangled name
	void* base = nullptr; //符号所属二进制文件的.text段基地址，等同于text_segment_base(unwind_frame)
	void* address = nullptr; //符号地址

	template<typename _T>
	inline operator _T*()
	{
		return (_T*)address;
	}

	/**
	 * @brief 符号的偏移量
	 */
	inline long long offset()
	{
		return (unsigned char*)base - (unsigned char*)address;
	}

	/**
	 * @brief 获取C++符号名称
	 */
	inline const char* cxx_name()
	{
		return cxx_name_demangling(name);
	}

	/**
	 * @brief 解析指定地址的符号信息
	 */
	static symbol resolve(void* symbol_addr);

	void free();
} symbol;

typedef struct shared_object
{
private:
	void* handle = nullptr;

	shared_object(void* handle) :
			handle(handle)
	{
	}

public:
	static shared_object dlopen(const char* so_path);

	/**
	 * @brief 加载的基地址
	 */
	void* base();

	inline bool loaded()
	{
		return handle != nullptr;
	}

	void dlclose();

	/**
	 * @brief 查找符号，变量或函数
	 */
	void* dlsym(const char* symbol_name);

	/**
	 * @brief 查找符号并转换为指定类型的指针，可以查找变量或函数
	 */
	template<typename _SymbolType>
	__attribute__((always_inline)) inline _SymbolType* dlsym(const char* symbol_name)
	{
		return (_SymbolType*)dlsym(symbol_name);
	}
} shared_object;

}

#endif //_CXXSP_SYM
