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

struct CompileAppSettings {
  std::string filename;

  // lexer definition file
  std::string lexer_definition_file_name;
};

int Run(const CompileAppSettings& settings) {

  spdlog::info("Intializing lexer...");
  Lexer lexer{settings.lexer_definition_file_name};
  lexer.SetInputFile(settings.filename);

  spdlog::info("Initializing parser ...");
  RecursiveDescentParser parser{};

  struct Lexeme lexeme{};
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

int main(int argc, char** argv) {
#if defined(CCDEBUG)
  spdlog::set_level(
        static_cast<spdlog::level::level_enum>(spdlog::level::level_enum::debug));
#endif

  CompileAppSettings settings;

  CLI::App app{"compile app - An app that uses all the passes of the compiler project"};

  app.add_option("-f", settings.filename,
                 "source file - the program to compile")->required(true);

  app.add_option("--lexer-definition-filename",
                 settings.lexer_definition_file_name,
                 "File defining the tokens and the corresponding regex that the given program is based on")
    ->required(true);

  CLI11_PARSE(app, argc, argv);

  return Run(settings);
}
