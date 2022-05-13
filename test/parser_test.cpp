// Unit tests for the Parser

#include <vector>
#include <string>
#include <parser/parser.hpp>
#include <spdlog/spdlog.h>
#include <CLI/CLI11.hpp>

// Test utilities
static ProductionElement MakePETerminal(const std::string& t) {
  return ProductionElement{ProductionElementType::TERMINAL, t};
}

static ProductionElement MakePENonTerminal(const std::string& nt) {
  return ProductionElement{ProductionElementType::NON_TERMINAL, nt};
}

static const std::vector<std::string> kParserTestFiles {
  /**
   * E -> abc
   */
  "./test-src/parser-test-grammar-files/config_a.grammar",
  /**
   * E -> Tabc
   * T -> xy
   */
  "./test-src/parser-test-grammar-files/config_b.grammar",
  /**
   * E -> Tabc
   * T -> xy
   * T -> %empty
   */
  "./test-src/parser-test-grammar-files/config_c.grammar",
  /**
   * E -> a X b c
   * X -> x y;
   */
  "./test-src/parser-test-grammar-files/config_d.grammar",
  /**
   * E -> a X Y b c
   * X -> x y
   * Y -> p q
   * Y -> %empty
   */
  "./test-src/parser-test-grammar-files/config_e.grammar",
  /**
   * E -> a X Y1 Y2 b c
   * X -> x y
   * Y1 -> p1 q1
   * Y1 -> epsilon
   * Y2 -> p2 q2
   * Y2 -> epsilon
   */
  "./test-src/parser-test-grammar-files/config_f.grammar",
  /**
   * E -> a X
   * T -> b E c
   * X -> x y
   */
  "./test-src/parser-test-grammar-files/config_g.grammar",
  /**
   * E -> a X
   * X -> b E c
   * E -> m
   */
  "./test-src/parser-test-grammar-files/config_h.grammar"
};

static const std::vector<Parser::ProductionElementFirstSet> kParserComputeFirstExpected{
    // config_a.grammar
    {{MakePETerminal("%empty"), {MakePETerminal("%empty")}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}}
    },
    // config_b.grammar
    {{Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("x")}},
     {MakePENonTerminal("T"), {MakePETerminal("x")}}
    },
    // config_c.grammar
    {{Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("a"), MakePETerminal("x")}},
     {MakePENonTerminal("T"), {MakePETerminal("x"), Parser::kEmptyTerminal}}
    },
    // config_d.grammar
    {{MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}}
    },
    // config_e.grammar
    {{Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePETerminal("p"), {MakePETerminal("p")}},
     {MakePETerminal("q"), {MakePETerminal("q")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}},
     {MakePENonTerminal("Y"), {MakePETerminal("p"), Parser::kEmptyTerminal}},
    },
    // config_f.grammar
    {{Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePETerminal("p1"), {MakePETerminal("p1")}},
     {MakePETerminal("q1"), {MakePETerminal("q1")}},
     {MakePETerminal("p2"), {MakePETerminal("p2")}},
     {MakePETerminal("q2"), {MakePETerminal("q2")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}},
     {MakePENonTerminal("Y1"), {MakePETerminal("p1"), Parser::kEmptyTerminal}},
     {MakePENonTerminal("Y2"), {MakePETerminal("p2"), Parser::kEmptyTerminal}}
    },
    // config_g.grammar
    {{MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("T"), {MakePETerminal("b")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}}
    },
    // config_h.grammar
    {{MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("m"), {MakePETerminal("m")}},
     {MakePENonTerminal("E"), {MakePETerminal("a"), MakePETerminal("m")}},
     {MakePENonTerminal("X"), {MakePETerminal("b")}}
    }
  };

static const std::vector<Parser::ProductionElementFirstSet> kParserComputeFollowExpected{
    // config_a.grammar
    {{MakePENonTerminal("E"), {}}
    },
    // config_b.grammar
    {{MakePENonTerminal("E"), {}},
     {MakePENonTerminal("T"), {MakePETerminal("a")}}
    },
    // config_c.grammar
    {{MakePENonTerminal("E"), {}},
     {MakePENonTerminal("T"), {MakePETerminal("a")}}
    },
    // config_d.grammar
    {{MakePENonTerminal("E"), {}},
     {MakePENonTerminal("X"), {MakePETerminal("b")}}
    },
    // config_e.grammar
    {{MakePENonTerminal("E"), {}},
     {MakePENonTerminal("X"), {MakePETerminal("p"), MakePETerminal("b")}},
     {MakePENonTerminal("Y"), {MakePETerminal("b")}}
    },
    // config_f.grammar
    {{MakePENonTerminal("E"), {}},
     {MakePENonTerminal("X"), {MakePETerminal("p1"), MakePETerminal("p2"), MakePETerminal("b")}},
     {MakePENonTerminal("Y1"), {MakePETerminal("p2"), MakePETerminal("b")}},
     {MakePENonTerminal("Y2"), {MakePETerminal("b")}}
    },
    // config_g.grammar
    {{MakePENonTerminal("T"), {}},
     {MakePENonTerminal("E"), {MakePETerminal("c")}},
     {MakePENonTerminal("X"), {MakePETerminal("c")}}
    },
    // config_h.grammar
    {{MakePENonTerminal("E"), {MakePETerminal("c")}},
     {MakePENonTerminal("X"), {MakePETerminal("c")}}
    }
  };

struct ParserTestSettings {
  bool test_compute_first{false};
  bool test_compute_follow{false};
};

class ParserTest {
  public:
    ParserTest(const ParserTestSettings& settings) : settings_{settings} {
    }

    bool RunTests() {
      if (settings_.test_compute_first) {
	TestComputeFirst();
      }

      if (settings_.test_compute_follow) {
	TestComputeFollow();
      }

      return true;
    }

  private:
    void TestComputeFirst() const;
    void TestComputeFollow() const;

  private:
    // utilities
    bool CompareProductionElementSets(const ProductionElementSet& a,
				      const ProductionElementSet& b) const;

    bool CompareProductionElementFirstSets(
      const Parser::ProductionElementFirstSet& a,
      const Parser::ProductionElementFirstSet& b) const;
    bool CompareProductionElementFollowSets(
      const Parser::ProductionElementFollowSet& a,
      const Parser::ProductionElementFollowSet& b) const;

  private:
    ParserTestSettings settings_;
};

bool ParserTest::CompareProductionElementSets(const ProductionElementSet& a,
					      const ProductionElementSet& b) const {
  // Test sizes
  if (a.size() != b.size()) { return false; }
  for (const auto& a_item : a) {
    if (b.find(a_item) == b.end()) { return false; }
  }
  return true;
};

bool ParserTest::CompareProductionElementFirstSets(
      const Parser::ProductionElementFirstSet& a,
      const Parser::ProductionElementFirstSet& b) const {

  // compare sizes
  if (a.size() != b.size()) { return false; }

  for (const auto& a_item : a) {
    const auto& pe_a{a_item.first};
    const auto& fs_a{a_item.second};

    if (b.find(pe_a) == b.end()) {
      spdlog::error("Cannot find production element {} in b ", pe_a.to_string());
      return false;
    }
    if (!CompareProductionElementSets(b.at(pe_a), fs_a)) {
      spdlog::error("Comparing first sets of {} failed ", pe_a.to_string());
      return false;
    }
  }

  return true;
}

bool ParserTest::CompareProductionElementFollowSets(
      const Parser::ProductionElementFollowSet& a,
      const Parser::ProductionElementFollowSet& b) const {

  // compare sizes
  if (a.size() != b.size()) { return false; }

  for (const auto& a_item : a) {
    const auto& pe_a{a_item.first};
    const auto& fs_a{a_item.second};

    if (b.find(pe_a) == b.end()) {
      spdlog::error("Cannot find production element {} in b ", pe_a.to_string());
      return false;
    }
    if (!CompareProductionElementSets(b.at(pe_a), fs_a)) {
      spdlog::error("Comparing follow sets of {} failed ", pe_a.to_string());
      return false;
    }
  }

  return true;
}

// Actual tests

void ParserTest::TestComputeFirst() const {

  int pass_count = 0;
  for (std::size_t i = 0; i < kParserTestFiles.size(); ++i) {
    const auto& grammar_filename{kParserTestFiles.at(i)};
    Parser p{grammar_filename};

    const auto& actual{p.GetFirsts()};
    const auto& expected{kParserComputeFirstExpected.at(i)};

    if (!CompareProductionElementFirstSets(actual, expected)) {
      spdlog::error("ComputeFirst() Test failed for grammar file {}",
		    grammar_filename);
    } else {
      pass_count++;
    }
  }

  spdlog::info("ComputeFirst() Tests - {} / {} passed",
	       pass_count, kParserTestFiles.size());
}

void ParserTest::TestComputeFollow() const {

  int pass_count = 0;
  for (std::size_t i = 0; i < kParserTestFiles.size(); ++i) {
    const auto& grammar_filename{kParserTestFiles.at(i)};
    Parser p{grammar_filename};

    const auto& actual{p.GetFollows()};
    const auto& expected{kParserComputeFollowExpected.at(i)};

    if (!CompareProductionElementFollowSets(actual, expected)) {
      spdlog::error("ComputeFollow() Test failed for grammar file {}",
		    grammar_filename);
    } else {
      pass_count++;
    }
  }

  spdlog::info("ComputeFollow() Tests - {} / {} passed",
	       pass_count, kParserTestFiles.size());
}

int main(int argc, char *argv[]) {

#if defined(CCDEBUG)
  spdlog::set_level(
        static_cast<spdlog::level::level_enum>(spdlog::level::level_enum::debug));
#endif

  ParserTestSettings settings;

  CLI::App app{"parser_test - Parser Test File"};
  app.add_flag("--test-compute-first",
               settings.test_compute_first,
               "Test the ComputFirst() function of the Parser");
  app.add_flag("--test-compute-follow",
               settings.test_compute_follow,
               "Test the ComputFollow() function of the Parser");
  CLI11_PARSE(app, argc, argv);

  ParserTest t{settings};
  t.RunTests();

  return 0;
}
