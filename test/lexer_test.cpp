// Run lexer of each of the input files
// Have the ground truth lexer outputs in a directory
// Compare the generated lexer output with the ground truth lexer output
#include <vector>
#include <string>
#include <lexer/lexer.hpp>
#include <utils/file_utils.hpp>
#include <spdlog/spdlog.h>
#include <CLI/CLI11.hpp>

struct LexerTestSettings {
  std::string lexer_definition_file_name;
};

struct TestFiles {
  std::string cool_program_file;
  std::string cool_lex_file;
  std::string coolcc_lex_file;
};

// The cool files are taken from the Stanford compiler course
// The ground truths (cool_lex_file) are generated from by "cool --lex cool-file"
static const std::vector<TestFiles> kTestFiles{
  {"./test-src/arith.cl",
   "./test-src/arith.cl-lex",
   "./test-src/arith.cl.cclex"},
  {"./test-src/atoi.cl",
   "./test-src/atoi.cl-lex",
   "./test-src/atoi.cl.cclex"},
  {"./test-src/atoi_test.cl",
   "./test-src/atoi_test.cl-lex",
   "./test-src/atoi_test.cl.cclex"},
  {"./test-src/book_list.cl",
   "./test-src/book_list.cl-lex",
   "./test-src/book_list.cl.cclex"},
  {"./test-src/cells.cl",
   "./test-src/cells.cl-lex",
   "./test-src/cells.cl.cclex"},
  {"./test-src/complex.cl",
   "./test-src/complex.cl-lex",
   "./test-src/complex.cl.cclex"},
  {"./test-src/cool.cl",
   "./test-src/cool.cl-lex",
   "./test-src/cool.cl.cclex"},
  {"./test-src/graph.cl",
   "./test-src/graph.cl-lex",
   "./test-src/graph.cl.cclex"},
  {"./test-src/hairyscary.cl",
   "./test-src/hairyscary.cl-lex",
   "./test-src/hairyscary.cl.cclex"},
  {"./test-src/hello_world.cl",
   "./test-src/hello_world.cl-lex",
   "./test-src/hello_world.cl.cclex"},
  {"./test-src/io.cl",
   "./test-src/io.cl-lex",
   "./test-src/io.cl.cclex"},
  {"./test-src/lam.cl",
   "./test-src/lam.cl-lex",
   "./test-src/lam.cl.cclex"},
  {"./test-src/life.cl",
   "./test-src/life.cl-lex",
   "./test-src/life.cl.cclex"},
  {"./test-src/list.cl",
   "./test-src/list.cl-lex",
   "./test-src/list.cl.cclex"},
  {"./test-src/new_complex.cl",
   "./test-src/new_complex.cl-lex",
   "./test-src/new_complex.cl.cclex"},
  {"./test-src/palindrome.cl",
   "./test-src/palindrome.cl-lex",
   "./test-src/palindrome.cl.cclex"},
  {"./test-src/primes.cl",
   "./test-src/primes.cl-lex",
   "./test-src/primes.cl.cclex"},
  {"./test-src/sort_list.cl",
   "./test-src/sort_list.cl-lex",
   "./test-src/sort_list.cl.cclex"},
  {"./test-src/nested_comments.cl",
   "./test-src/nested_comments.cl-lex",
   "./test-src/nested_comments.cl.cclex"}
};

void RunTests(const LexerTestSettings& settings) {

  for (const auto& test : kTestFiles) {
    spdlog::info("Testing {} ...", test.cool_program_file);
    // Run the lexer on the cool_program_file
    Lexer lexer{settings.lexer_definition_file_name};
    lexer.RunLexerOn(test.cool_program_file);

    // Read lex output from coolcc (my implementation)
    const std::vector<std::string> test_lex{ReadFileLines(test.coolcc_lex_file)};
    // Read lex output from cool
    const std::vector<std::string> groundtruth_lex{ReadFileLines(test.cool_lex_file)};

    if (test_lex.size() != groundtruth_lex.size()) {
      spdlog::error("Lex #lines mismatch {} vs {}",
		    test_lex.size(), groundtruth_lex.size());
      continue;
    }

    const std::size_t n_test_lines{test_lex.size()};
    for (std::size_t i = 0; i < n_test_lines; ++i) {
      if (test_lex.at(i) != groundtruth_lex.at(i)) {
	spdlog::error("{} : Test lex {} vs Groundtruth lex {} mismatch",
		       i, test_lex.at(i), groundtruth_lex.at(i));
      }
    }
    spdlog::info("Test {} Passed ...", test.cool_program_file);
  }
}

int main(int argc, char *argv[]) {

#if defined(CCDEBUG)
  spdlog::set_level(
        static_cast<spdlog::level::level_enum>(spdlog::level::level_enum::debug));
#endif

  LexerTestSettings settings;

  CLI::App app{"lexer_test - Lexer Test File"};
  app.add_option("--lexer-definition-filename",
                 settings.lexer_definition_file_name,
                 "File defining tokens and regexes");
  CLI11_PARSE(app, argc, argv);

  RunTests(settings);

  return 0;
}
