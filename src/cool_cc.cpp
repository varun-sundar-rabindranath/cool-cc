// Main function for the cool compiler
#include <CLI/CLI11.hpp>
#include "spdlog/spdlog.h"

#include "lexer/lexer.hpp"
//#include "parser/parser.hpp"
#include "parser/recursive_descent_parser_generator.hpp"

struct CoolCCAppSettings {
  std::string filename;

  // lexer command options
  std::string lexer_definition_file_name;
  bool lexer{false};

  // parser command options
  std::string grammar_definition_file_name;
  bool parser{false};
};

int RunLexer(const std::string& filename,
	     const std::string& lexer_definition_file_name) {
  spdlog::info("Running Lexer...");
  spdlog::info(" - COOL source file {}", filename);
  spdlog::info(" - Lex definition file {}", lexer_definition_file_name);

  Lexer lexer{lexer_definition_file_name};
  lexer.RunLexerOn(filename);
  return 0;
}

int RunParser(const std::string& grammar_definition_file_name) {
  spdlog::info("Running Parser ...");
  spdlog::info(" - Grammar definition file {}", grammar_definition_file_name);

  RecursiveDescentParserGenerator rd_parser_generator{grammar_definition_file_name};

  rd_parser_generator.WriteSemanticRules(
    "/home/varun/study/compilers/cool-cc/data/arith/arith_semantic_rules.cpp");
  rd_parser_generator.WriteParsingTable(
    "/home/varun/study/compilers/cool-cc/data/arith/arith_parsing_table.cpp");
  return 0;
}

int Run(const CoolCCAppSettings& settings) {
  if (settings.lexer) {
    RunLexer(settings.filename, settings.lexer_definition_file_name);
  }

  if (settings.parser) {
    RunParser(settings.grammar_definition_file_name);
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

  // Make these into command groups
  // Lexer command group
  app.add_option("--lexer-definition-filename",
                 settings.lexer_definition_file_name,
                 "File defining the tokens and the corresponding regex");
  app.add_flag("--lexer", settings.lexer, "Run the lexer");

  // Parser command group
  app.add_option("--grammar-definition-filename",
                 settings.grammar_definition_file_name,
                 "File defining the grammar - Terminals, Non Terminals & Productions");
  app.add_flag("--parser", settings.parser, "Run the parser");

  CLI11_PARSE(app, argc, argv);

  return Run(settings);
}
