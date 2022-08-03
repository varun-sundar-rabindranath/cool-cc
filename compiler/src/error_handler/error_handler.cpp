#include "error_handler/error_handler.hpp"
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <utils/file_utils.hpp>

ErrorHandler::ErrorHandler(const std::string& prefix,
			   const std::string& file_name) :
  prefix_{prefix},
  file_name_{file_name},
  file_location_{file_name} {
}

void ErrorHandler::ConsolePrint(const std::size_t buf_idx,
			       	const std::string& msg) {

  const auto file_location_info{file_location_.GetFileLocationInfo(buf_idx)};

  const std::string err_msg_wo_line{
    fmt::format("{} {}:{} -", prefix_, file_name_, file_location_info.line_no + 1)};
  const std::size_t err_msg_wo_line_len{err_msg_wo_line.length()};
  const std::string err_msg{
    fmt::format("{}{}", err_msg_wo_line, file_location_info.file_line)};

  spdlog::error(err_msg);
  spdlog::error(
    fmt::format("{: <{}}{}", "", static_cast<int>(err_msg_wo_line_len + file_location_info.col_no), "^"));
  spdlog::error("Error-Msg {}", msg);
}
