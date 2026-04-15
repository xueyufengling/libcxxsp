#ifndef _CXXSP_FILESYS
#define _CXXSP_FILESYS

#include <string>
#include <cxxsp/arch.h>

#define __WIN32_PATH_SEPARATOR__ '\\'
#define __UNIX_PATH_SEPARATOR__ '/'

#if defined(__OS_WIN__)
#define __PATH_SEPARATOR__ __WIN32_PATH_SEPARATOR__
#elif defined(__OS_UNIX__)
#define __PATH_SEPARATOR__ __UNIX_PATH_SEPARATOR__
#endif

namespace cxxsp
{
/**
 * @brief 以二进制读取文件内容到内存
 * @param filename 文件路径
 * @param data_length 记录返回数组长度，读取到的文件实际大小
 * @param read_length read_length<0则读取全部内容
 * @param reserve_length 读取完毕后内存末尾多分配的空字节数
 */
extern unsigned char* read(const std::string filename, size_t* data_length, long long int read_length = -1, size_t reserve_length = 0);

/**
 * @brief 以文本形式读取文件内容
 * @param filename 文件路径
 * @param read_length read_length=<0则读取全部内容
 * @param reserve_length 读取完毕后内存末尾多分配的空字节数
 */
extern std::string read_string(const std::string filename, long long int read_length = -1, size_t reserve_length = 0);

extern void write(const std::string filename, const void* data, size_t length);

extern void append(const std::string filename, const void* data, size_t length);

extern void append_string(const std::string filename, const std::string& str);

extern void append_string_newline(const std::string filename, const std::string& str);

extern std::string directory_of(const std::string file_path);

extern void clear(const std::string file_name);

extern const char* get_filename(const char* path);
}

#endif // _CXXSP_FILESYS
