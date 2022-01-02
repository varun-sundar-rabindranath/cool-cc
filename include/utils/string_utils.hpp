#ifndef __STRING_UTILS_HPP__
#define __STRING_UTILS_HPP__
// Utilities to process string

#include <string>
#include <vector>

std::string Trim(const std::string& s);

std::vector<std::string> Split(const std::string& s, const char delim);

#endif // __STRING_UTILS_HPP__
