#include <cxxsp/filesys.h>

#include <fstream>
#include <iostream>
#include <string.h>

#ifdef __cpp_lib_filesystem
#include <filesystem>
using namespace std::filesystem;
#elif __cplusplus >= 201103L
#include <experimental/filesystem>
using namespace std::experimental::filesystem;
#endif

unsigned char* cxxsp::read(const std::string filename, size_t* data_length, long long int read_length, size_t reserve_length)
{
	std::ifstream file(u8path(filename).generic_u8string(), std::ifstream::ate | std::ios::binary);
	if(file.is_open())
	{
		size_t file_length = file.tellg();
		file.seekg(std::ios::beg);
		if(read_length < 0)
			read_length = file_length;
		*data_length = (read_length + reserve_length);
		unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * *data_length);
		file.read((char*)data, read_length);
		file.close();
		return data;
	}
	return nullptr;
}

std::string cxxsp::read_string(const std::string filename, long long int read_length, size_t reserve_length)
{
	size_t str_length = 0;
	char* str = (char*)cxxsp::read(filename, &str_length, read_length, reserve_length + 1);
	str[str_length] = '\0';
	return str;
}

void cxxsp::write(const std::string filename, const void* data, size_t length)
{
	std::ofstream file(u8path(filename).generic_u8string(), std::ios::out | std::ios::binary);
	file.write((const char*)data, length);
	file.close();
}

void cxxsp::append(const std::string filename, const void* data, size_t length)
{
	std::ofstream file(u8path(filename).generic_u8string(), std::ios::app | std::ios::binary);
	file.write((const char*)data, length);
	file.close();
}

void cxxsp::append_string(const std::string filename, const std::string& str)
{
	std::ofstream file(u8path(filename).generic_u8string(), std::ios::app);
	file << str;
	file.close();
}

void cxxsp::append_string_newline(const std::string filename, const std::string& str)
{
	std::ofstream file(u8path(filename).generic_u8string(), std::ios::app);
	file << '\n' << str;
	file.close();
}

void cxxsp::clear(const std::string file_name)
{
	std::ofstream f(file_name, std::ios::out | std::ios::binary);
	f.write(nullptr, 0);
	f.close();
}

std::string cxxsp::directory_of(const std::string file_path)
{
	static const char _path_separators[3] = {__WIN32_PATH_SEPARATOR__, __UNIX_PATH_SEPARATOR__, '\0'};
	return file_path.substr(0, file_path.find_last_of(_path_separators));
}

const char* cxxsp::get_filename(const char* path)
{
	char* win32_filename = strrchr(path, __WIN32_PATH_SEPARATOR__) + 1;
	char* unix_filename = strrchr(path, __UNIX_PATH_SEPARATOR__) + 1;
	if(win32_filename || unix_filename)
	{
		if(win32_filename > unix_filename)
			return win32_filename;
		else
			return unix_filename;
	}
	else
		return path;
}
