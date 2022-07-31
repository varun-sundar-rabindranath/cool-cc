/**
 * Executable to generate parsing files from the grammar definition files
 */

#include <CLI/CLI11.hpp>
#include <fmt/format.h>
#include "spdlog/spdlog.h"
#include <parser/recursive_descent_parser_generator.hpp>
#include <string>

struct ParsingFileGeneratorSetting {
  std::string grammar_definition_file_name;
  std::string output_directory;
};

void Run(const ParsingFileGeneratorSetting& settings) {
  spdlog::info("Generating parsing files for Grammar {}",
               settings.grammar_definition_file_name);
  spdlog::info("Writing parsing files to {}",
               settings.output_directory);
  RecursiveDescentParserGenerator rdpg{settings.grammar_definition_file_name};

  rdpg.WriteSemanticRules(
    fmt::format("{}/semantic_rules.cpp", settings.output_directory));
  rdpg.WriteParsingTable(
    fmt::format("{}/parsing_table.cpp", settings.output_directory));
}

int main(int argc, char** argv) {

  ParsingFileGeneratorSetting settings;

  CLI::App app{"Parsing Files Generator - This is similar to the YACC program"};

  // Parser command group
  app.add_option("--grammar-definition-filename",
                 settings.grammar_definition_file_name,
                 "File defining the grammar - Terminals, Non Terminals & Productions")
    ->required(true);
  app.add_option("--output-directory",
                 settings.output_directory,
                 "Output directory location to put the parsing files")
    ->required(true);

  CLI11_PARSE(app, argc, argv);

  Run(settings);

  return 0;
}
