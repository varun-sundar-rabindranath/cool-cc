#ifndef __ERROR_HANDLER_HPP__
#define __ERROR_HANDLER_HPP__

#include <string>
#include <utils/file_location.hpp>

class ErrorHandler {
public:
  ErrorHandler(const std::string& prefix, const std::string& file_name);
  ~ErrorHandler() = default;

  void ConsolePrint(const std::size_t buf_idx, const std::string& msg);
private:
  std::string prefix_;
  std::string file_name_;
  FileLocation file_location_;
};

#endif // __ERROR_HANDLER_HPP__
