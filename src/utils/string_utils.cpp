// Implementation of string utils

#include "utils/string_utils.hpp"
#include <cctype>

std::string Trim(const std::string& s) {
  std::string trimmed = s;
  while(trimmed.length() && std::isspace(trimmed.front())) {
    trimmed.erase(trimmed.begin());
  }

  while(trimmed.length() && std::isspace(trimmed.back())) {
    trimmed.pop_back();
  }

  return trimmed;
}

std::vector<std::string> Split(const std::string& s, const char delim) {
  std::vector<std::string> parts;
  int start = 0;
  for (std::size_t i = 0; i < s.size(); ++i) {
    if (s.at(i) == delim) {
      parts.emplace_back(s.substr(start, i - start));
      start = i + 1;
    }
  }
  if (s.back() != delim) {
    parts.emplace_back(s.substr(start, s.length() - start));
  }

  return parts;
}
