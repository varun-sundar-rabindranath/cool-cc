#include <utils/file_location.hpp>
#include <utils/file_utils.hpp>
#include <spdlog/spdlog.h>

FileLocation::FileLocation(const std::string& filename)
  : file_name_{filename},
    file_lines_{ReadFileLines(filename)}{

  int buf_idx{0};
  for (const auto& file_line : file_lines_) {
   line_start_indices_.emplace_back(buf_idx);
   buf_idx += file_line.length() + 1;
  }

#if 0
  for (int i = 0; i < static_cast<int>(file_lines_.size()); ++i) {
    spdlog::info("{} {} {}", line_start_indices_.at(i),
		 file_lines_.at(i), file_lines_.at(i).length());
  }
#endif
}

FileLocationInfo FileLocation::GetFileLocationInfo(const std::size_t buf_idx) {

  // line number
  const auto line_start_ub{std::upper_bound(line_start_indices_.begin(),
					    line_start_indices_.end(),
					    buf_idx)};
  const int line_no = buf_idx == 0 ? 0 :  line_start_ub - line_start_indices_.begin() - 1;
  const int col_no = buf_idx - line_start_indices_.at(line_no);
  //spdlog::info("linestart found {} - {} {} {}", *line_start_ub, buf_idx, line_no, col_no);
  assert (col_no >= 0 && col_no <= static_cast<int>(file_lines_.at(line_no).length()));
  const std::string line = file_lines_.at(line_no);
  return {file_name_, buf_idx, static_cast<std::size_t>(line_no),
	  static_cast<std::size_t>(col_no), file_lines_.at(line_no)};
}
