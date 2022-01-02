#ifndef __FILE_UTILS_HPP__
#define __FILE_UTILS_HPP__
// Utilities to process files
#include <string>
#include <vector>

std::string ReadFile(const std::string& file_path);

std::vector<std::string> ReadFileLines(const std::string& file_path);

void WriteToFile(const std::string& file_name, const std::string& content);

#endif // __FILE_UTILS_HPP__
