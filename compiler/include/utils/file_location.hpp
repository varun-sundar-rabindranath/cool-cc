#ifndef __FILE_COL_NO_HPP__
#define __FILE_COL_NO_HPP__

#include <vector>
#include <string>

struct FileLocationInfo {
  std::string file_name;
  std::size_t buf_idx{0};
  std::size_t line_no{0};
  std::size_t col_no{0};
  std::string file_line;
};

class FileLocation {
public:

  FileLocation(const std::string& filename);
  ~FileLocation() = default;

  FileLocationInfo GetFileLocationInfo(const std::size_t buf_idx);

private:
  std::string file_name_;
  std::vector<std::string> file_lines_;
  std::vector<std::size_t> line_start_indices_;
};

#endif // __FILE_COL_NO_HPP__
