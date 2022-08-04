/**
 * Compile : Program links against the libraries both in the compiler and
 * the pl projects.
 * Given a lex file and an input program - parses the input program
 */

#include <CLI/CLI11.hpp>
#include <spdlog/spdlog.h>
#include <lexer/lexer.hpp>
#include <parser/recursive_descent_parser.hpp>
#include <stdexcept>
#include <string>
#include <fstream>

struct CompileAppSettings {
  std::string program_filename;
  // When set to true, program_filename is ignored
  bool interpreter_mode{false};

  // lexer definition file
  std::string lexer_definition_file_name;
};

bool ProcessFile(const CompileAppSettings& settings) {

  spdlog::info("Intializing lexer...");
  Lexer lexer{settings.lexer_definition_file_name};
  lexer.SetInputFile(settings.program_filename);

  spdlog::info("Initializing parser ...");
  RecursiveDescentParser parser{};

  struct Lexeme lexeme{};
  // check if the lexer works
  while(lexer.GetNextLexeme(&lexeme)) {
    std::cout<<lexeme.lexeme<< " - "<<lexeme.token<<std::endl;
  }

  return 0;

  while (lexer.GetNextLexeme(&lexeme)) {
    if (parser.GetParserState() != ParserState::PARSER_STATE_PROCESSING) {
      throw std::runtime_error("Parser not in processing state ...");
    }
    const auto parser_state{parser.ProcessLexeme(lexeme)};
    if (parser_state == ParserState::PARSER_STATE_ERROR) {
      throw std::runtime_error("Parser is in error state ... Aborting");
    }
  }

  assert(!lexer.GetNextLexeme(&lexeme) && "Lexer still has tokens ...");
  assert((parser.GetParserState() != ParserState::PARSER_STATE_FINISHED) &&
         "Parser not in finished state");

  return 0;

}

int Run(const CompileAppSettings& settings) {

  auto write_to_file = [](const std::string& file_name,
                          const std::string& content) {
    std::ofstream outfile;
    outfile.open(file_name.c_str(), std::ios::out | std::ios::trunc);
    outfile << content;
    outfile.close();
  };

  if (settings.interpreter_mode) {
    while (true) {
      std::cout<<">> ";
      std::string input;
      std::getline(std::cin, input);

      if (input == "exit") {
        return 0;
      }

      const std::string input_string_filename{"/tmp/input_string.txt"};

      // Write the input string to a file and process the file
      write_to_file(input_string_filename, input);

      // Make a new settings struct with the filename updated
      CompileAppSettings s{settings};
      s.program_filename = input_string_filename;
      s.interpreter_mode = false;

      // Process input string file
      ProcessFile(s);
    }
  } else {
    // normal mode
    return ProcessFile(settings);
  }

  return 0;
}

int main(int argc, char** argv) {
#if defined(CCDEBUG)
  spdlog::set_level(
        static_cast<spdlog::level::level_enum>(spdlog::level::level_enum::debug));
#endif

  CompileAppSettings settings;

  CLI::App app{"compile app - An app that uses all the passes of the compiler project"};

  app.add_option("-f", settings.program_filename,
                 "source file - the program to compile");
  app.add_flag("-i", settings.interpreter_mode,
               "Run the compiler in interpreter mode");

  app.add_option("--lexer-definition-filename",
                 settings.lexer_definition_file_name,
                 "File defining the tokens and the corresponding regex that the given program is based on")
    ->required(true);

  CLI11_PARSE(app, argc, argv);

  auto settings_check = [](const CompileAppSettings& settings) -> bool {
    // The arguments program_filename and interpreter_mode are mutually
    // exclusive and atleast one must be specified
    const bool have_program_filename = !settings.program_filename.empty();
    const bool have_interpreter_mode = settings.interpreter_mode;
    const bool good_settings{(have_program_filename && !have_interpreter_mode) ||
                             (!have_program_filename && have_interpreter_mode)};
    return good_settings;
  };

  if (!settings_check(settings)) {
    spdlog::info("CompilerAppSettings:");
    spdlog::info("settings.program_filename {}", settings.program_filename);
    spdlog::info("settings.interpreter_mode {}", settings.interpreter_mode);
    spdlog::info("settings.lexer_definition_file_name {}", settings.lexer_definition_file_name);
    throw std::invalid_argument("Bad Settings");
  }

  return Run(settings);
}
