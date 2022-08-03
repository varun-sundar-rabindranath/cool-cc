// Unit tests for the Recursive Descent Parser Generator

#include <vector>
#include <string>
#include <parser/recursive_descent_parser_generator.hpp>
#include <spdlog/spdlog.h>
#include <CLI/CLI11.hpp>

// Test utilities
static ProductionElement MakePETerminal(const std::string& t) {
  return ProductionElement{ProductionElementType::TERMINAL, t};
}

static ProductionElement MakePENonTerminal(const std::string& nt) {
  return ProductionElement{ProductionElementType::NON_TERMINAL, nt};
}

static Production MakeProduction(const std::string& left_str,
				 const std::vector<std::string>& right_strs,
				 const std::vector<bool>& is_right_terminal)  {
  assert (right_strs.size() == is_right_terminal.size());

  const ProductionElement left{MakePENonTerminal(left_str)};

  const std::size_t n_right{right_strs.size()};
  std::vector<ProductionElement> right;
  for (std::size_t i = 0; i < n_right; ++i) {
    right.push_back(is_right_terminal.at(i) ? MakePETerminal(right_strs.at(i)) : MakePENonTerminal(right_strs.at(i)));
  }

  return Production{left, right};
}

static const std::vector<std::string> kParserGeneratorTestFiles {
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

static const std::vector<RecursiveDescentParserGenerator::ProductionElementFirstSet>
  kParserGeneratorComputeFirstExpected{
    // config_a.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {ParserGenerator::kEmptyTerminal, {ParserGenerator::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}}
    },
    // config_b.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {ParserGenerator::kEmptyTerminal, {ParserGenerator::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("x")}},
     {MakePENonTerminal("T"), {MakePETerminal("x")}}
    },
    // config_c.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {ParserGenerator::kEmptyTerminal, {ParserGenerator::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("a"), MakePETerminal("x")}},
     {MakePENonTerminal("T"), {MakePETerminal("x"), ParserGenerator::kEmptyTerminal}}
    },
    // config_d.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}}
    },
    // config_e.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {ParserGenerator::kEmptyTerminal, {ParserGenerator::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePETerminal("p"), {MakePETerminal("p")}},
     {MakePETerminal("q"), {MakePETerminal("q")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}},
     {MakePENonTerminal("Y"), {MakePETerminal("p"), ParserGenerator::kEmptyTerminal}},
    },
    // config_f.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {ParserGenerator::kEmptyTerminal, {ParserGenerator::kEmptyTerminal}},
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
     {MakePENonTerminal("Y1"), {MakePETerminal("p1"), ParserGenerator::kEmptyTerminal}},
     {MakePENonTerminal("Y2"), {MakePETerminal("p2"), ParserGenerator::kEmptyTerminal}}
    },
    // config_g.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("S"), {MakePETerminal("a")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("T"), {MakePETerminal("b")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}}
    },
    // config_h.grammar
    {{ParserGenerator::kEndOfInputTerminal, {ParserGenerator::kEndOfInputTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("m"), {MakePETerminal("m")}},
     {MakePENonTerminal("S"), {MakePETerminal("a"), MakePETerminal("m")}},
     {MakePENonTerminal("E"), {MakePETerminal("a"), MakePETerminal("m")}},
     {MakePENonTerminal("X"), {MakePETerminal("b")}}
    }
  };

static const std::vector<RecursiveDescentParserGenerator::ProductionElementFirstSet>
  kParserComputeFollowExpected{
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
     {MakePENonTerminal("S"), {}},
     {MakePENonTerminal("E"), {MakePETerminal("c"), ParserGenerator::kEndOfInputTerminal}},
     {MakePENonTerminal("X"), {MakePETerminal("c"), ParserGenerator::kEndOfInputTerminal}}
    },
    // config_h.grammar
    {{MakePENonTerminal("S"), {}},
     {MakePENonTerminal("E"), {MakePETerminal("c"), ParserGenerator::kEndOfInputTerminal}},
     {MakePENonTerminal("X"), {MakePETerminal("c"), ParserGenerator::kEndOfInputTerminal}}
    }
  };

struct ParsingTableEntry {

  ParsingTableEntry(const std::string& nt_str,
		    const std::string& t_str,
		    const Production& production_) :
    non_terminal{MakePENonTerminal(nt_str)},
    terminal{MakePETerminal(t_str)},
    production{production_}
  {
  }

  ProductionElement non_terminal;
  ProductionElement terminal;
  Production production;
};

static const std::vector<std::string> kParserParsingTableTestFiles {
  /**
   * arithmetic operations with + and * with precedence enforced
   */
  "./test-src/parser-test-grammar-files/arith.grammar"
};

using kParsingTableEntries = std::vector<ParsingTableEntry>;

static const std::vector<kParsingTableEntries> kParsingTableExpected {
  {
    {"S", "id", MakeProduction("S", {"E", "$"}, {0, 1})},
    {"S", "(", MakeProduction("S", {"E", "$"}, {0, 1})},
    {"E", "id", MakeProduction("E", {"T", "E_DASH"}, {0, 0})},
    {"E", "(", MakeProduction("E", {"T", "E_DASH"}, {0, 0})},
    {"E_DASH", "+", MakeProduction("E_DASH", {"+", "T", "E_DASH"}, {1, 0, 0})},
    {"E_DASH", ")", MakeProduction("E_DASH", {ParserGenerator::kEmptyTerminal.element}, {1})},
    {"E_DASH", "$", MakeProduction("E_DASH", {ParserGenerator::kEmptyTerminal.element}, {1})},
    {"T", "id", MakeProduction("T", {"F", "T_DASH"}, {0, 0})},
    {"T", "(", MakeProduction("T", {"F", "T_DASH"}, {0, 0})},
    {"T_DASH", "+", MakeProduction("T_DASH", {ParserGenerator::kEmptyTerminal.element}, {1})},
    {"T_DASH", ")", MakeProduction("T_DASH", {ParserGenerator::kEmptyTerminal.element}, {1})},
    {"T_DASH", "$", MakeProduction("T_DASH", {ParserGenerator::kEmptyTerminal.element}, {1})},
    {"T_DASH", "*", MakeProduction("T_DASH", {"*", "F", "T_DASH"}, {1, 0, 0})},
    {"F", "id", MakeProduction("F", {"id"}, {1})},
    {"F", "(", MakeProduction("F", {"(", "E", ")"}, {1, 0, 1})}
  }
};

struct ParserGeneratorTestSettings {
  bool test_compute_first{false};
  bool test_compute_follow{false};
  bool test_compute_parsing_table{false};
};

class ParserGeneratorTest {
  public:
    ParserGeneratorTest(const ParserGeneratorTestSettings& settings) :
      settings_{settings} {
    }

    bool RunTests() {
      if (settings_.test_compute_first) {
	TestComputeFirst();
      }

      if (settings_.test_compute_follow) {
	TestComputeFollow();
      }

      if (settings_.test_compute_parsing_table) {
	TestComputeParsingTable();
      }

      return true;
    }

  private:
    void TestComputeFirst() const;
    void TestComputeFollow() const;
    void TestComputeParsingTable() const;

  private:
    // utilities
    bool CompareProductionElementSets(const ProductionElementSet& a,
				      const ProductionElementSet& b) const;

    // TODO: Please use templates instead
    bool CompareProductionElementFirstSets(
      const RecursiveDescentParserGenerator::ProductionElementFirstSet& a,
      const RecursiveDescentParserGenerator::ProductionElementFirstSet& b) const;
    bool CompareProductionElementFollowSets(
      const RecursiveDescentParserGenerator::ProductionElementFollowSet& a,
      const RecursiveDescentParserGenerator::ProductionElementFollowSet& b) const;
    bool CompareProductionVectors(
      const ProductionVector& a,
      const ProductionVector& b) const;
    bool IsParsingTableMatch(
      const RecursiveDescentParserGenerator& p,
      const kParsingTableEntries& expected) const;

  private:
    ParserGeneratorTestSettings settings_;
};

bool ParserGeneratorTest::CompareProductionElementSets(
  const ProductionElementSet& a, const ProductionElementSet& b) const {
  // Test sizes
  if (a.size() != b.size()) { return false; }
  for (const auto& a_item : a) {
    if (b.find(a_item) == b.end()) { return false; }
  }
  return true;
};

bool ParserGeneratorTest::CompareProductionElementFirstSets(
      const RecursiveDescentParserGenerator::ProductionElementFirstSet& a,
      const RecursiveDescentParserGenerator::ProductionElementFirstSet& b) const {

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

bool ParserGeneratorTest::CompareProductionElementFollowSets(
      const RecursiveDescentParserGenerator::ProductionElementFollowSet& a,
      const RecursiveDescentParserGenerator::ProductionElementFollowSet& b) const {

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

bool ParserGeneratorTest::CompareProductionVectors(
  const ProductionVector& a,
  const ProductionVector& b) const {

  // Convert vector into sets for easy comparison
  const std::unordered_set<Production, production_hash> a_set(a.begin(), a.end());
  const std::unordered_set<Production, production_hash> b_set{b.begin(), b.end()};

  if (a_set.size() != b_set.size()) {
    return false;
  }

  for (const auto& a_item : a_set) {
    if (b_set.find(a_item) == b_set.end()) {
      return false;
    }
  }

  return true;
}

bool ParserGeneratorTest::IsParsingTableMatch(
  const RecursiveDescentParserGenerator& p,
  const kParsingTableEntries& expected) const {

  const auto& terminals{p.GetTerminals()};
  const auto& non_terminals{p.GetNonTerminals()};

  auto get_expected_productions =
    [&](const ProductionElement& nt, const ProductionElement& t) -> ProductionVector {
      ProductionVector expected_ps;
      for (const auto& pt_entry : expected) {
	if (pt_entry.non_terminal == nt && pt_entry.terminal == t) {
	  expected_ps.push_back(pt_entry.production);
	}
      }
      return expected_ps;
  };

  for (const auto& nt : non_terminals) {
    for (const auto& t : terminals) {
      const auto& actual_productions{p.GetParsingTableProductions(nt, t)};
      const auto& expected_productions{get_expected_productions(nt, t)};
      if (!CompareProductionVectors(actual_productions, expected_productions)) {
        spdlog::error("Comparing parser entry at {}, {}", nt.to_string(), t.to_string());
        spdlog::error("actual productions ");
        for (const auto& p : actual_productions) {
          spdlog::error("{}", p.to_string());
        }
        spdlog::error("expected productions ");
        for (const auto& p : expected_productions) {
          spdlog::error("{}", p.to_string());
        }
	return false;
      }
    }
  }
  return true;
}

// Actual tests

void ParserGeneratorTest::TestComputeFirst() const {

  int pass_count = 0;
  for (std::size_t i = 0; i < kParserGeneratorTestFiles.size(); ++i) {
    const auto& grammar_filename{kParserGeneratorTestFiles.at(i)};
    RecursiveDescentParserGenerator p{grammar_filename};

    const auto& actual{p.GetFirsts()};
    const auto& expected{kParserGeneratorComputeFirstExpected.at(i)};

    if (!CompareProductionElementFirstSets(actual, expected)) {
      spdlog::error("ComputeFirst() Test failed for grammar file {}",
		    grammar_filename);
    } else {
      pass_count++;
    }
  }

  spdlog::info("ComputeFirst() Tests - {} / {} passed",
	       pass_count, kParserGeneratorTestFiles.size());
}

void ParserGeneratorTest::TestComputeFollow() const {

  int pass_count = 0;
  for (std::size_t i = 0; i < kParserGeneratorTestFiles.size(); ++i) {
    const auto& grammar_filename{kParserGeneratorTestFiles.at(i)};
    RecursiveDescentParserGenerator p{grammar_filename};

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
	       pass_count, kParserGeneratorTestFiles.size());
}

void ParserGeneratorTest::TestComputeParsingTable() const {

  int pass_count = 0;
  for (std::size_t i = 0; i < kParserParsingTableTestFiles.size(); ++i) {
    const auto& grammar_filename{kParserParsingTableTestFiles.at(i)};
    RecursiveDescentParserGenerator p{grammar_filename};

    const auto& expected{kParsingTableExpected.at(i)};
    if (!IsParsingTableMatch(p, expected)) {
      spdlog::error("ComputeParsingTable() Test failed for grammar file {}",
		    grammar_filename);
    } else {
      pass_count++;
    }
  }

  spdlog::info("ComputeParsingTable() Tests - {} / {} passed",
	       pass_count, kParserParsingTableTestFiles.size());
}

int main(int argc, char *argv[]) {

#if defined(CCDEBUG)
  spdlog::set_level(
        static_cast<spdlog::level::level_enum>(spdlog::level::level_enum::debug));
#endif

  ParserGeneratorTestSettings settings;

  CLI::App app{"parser_generator_test - Parser Test File"};
  app.add_flag("--test-compute-first",
               settings.test_compute_first,
               "Test the ComputFirst() function of the Parser");
  app.add_flag("--test-compute-follow",
               settings.test_compute_follow,
               "Test the ComputFollow() function of the Parser");
  app.add_flag("--test-compute-parsing-table",
               settings.test_compute_parsing_table,
               "Test the ComputParsingTable() function of the Parser");
  CLI11_PARSE(app, argc, argv);

  ParserGeneratorTest t{settings};
  t.RunTests();

  return 0;
}
