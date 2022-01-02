// Main function for the cool compiler
#include <CLI/CLI11.hpp>
#include "spdlog/spdlog.h"

#include "lexer/lexer.hpp"

struct CoolCCAppSettings {
  std::string filename;
  std::string lexer_definition_file_name;
  bool lexer{false};
};

int Run(const CoolCCAppSettings& settings) {
  spdlog::info("Cool source file {}", settings.filename);
  spdlog::info("Lexer definition filename ? {}", settings.lexer_definition_file_name);
  spdlog::info("Lexer on ? {}", settings.lexer);

  Lexer lexer{settings.lexer_definition_file_name};
  if (settings.lexer) {
    lexer.RunLexerOn(settings.filename);
  }



  return 0;
}

int main(int argc, char** argv) {
#if defined(CCDEBUG)
  spdlog::set_level(
        static_cast<spdlog::level::level_enum>(spdlog::level::level_enum::debug));
#endif

  CoolCCAppSettings settings;

  CLI::App app{"cool-cc - A COOL compiler impl."};
  app.add_option("-f", settings.filename, "COOL source file");
  app.add_option("--lexer-definition-filename",
                 settings.lexer_definition_file_name,
                 "File defining the tokens and the corresponding regex");
  app.add_flag("--lexer", settings.lexer, "Run the lexer");
  CLI11_PARSE(app, argc, argv);

  return Run(settings);
}
