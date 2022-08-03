// Implementation of file utils

#include "utils/file_utils.hpp"
#include <fstream>
#include <sstream>

std::string ReadFile(const std::string& file_path) {

  std::ifstream f{file_path};
  std::stringstream buffer;
  buffer << f.rdbuf();

  return buffer.str();
}

std::vector<std::string> ReadFileLines(const std::string& file_path) {

  std::vector<std::string> file_lines;

  std::ifstream f(file_path);

  std::string line;
  while (std::getline(f, line)) {
    file_lines.emplace_back(line);
  }

  f.close();

  return file_lines;
}

void WriteToFile(const std::string& file_name, const std::string& content) {

  std::ofstream outfile;
  outfile.open(file_name.c_str(), std::ios::out | std::ios::trunc);
  outfile << content;
  outfile.close();
}
