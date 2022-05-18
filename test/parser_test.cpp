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
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
     {Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}}
    },
    // config_b.grammar
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
     {Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("x")}},
     {MakePENonTerminal("T"), {MakePETerminal("x")}}
    },
    // config_c.grammar
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
     {Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("a"), MakePETerminal("x")}},
     {MakePENonTerminal("T"), {MakePETerminal("x"), Parser::kEmptyTerminal}}
    },
    // config_d.grammar
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("x"), {MakePETerminal("x")}},
     {MakePETerminal("y"), {MakePETerminal("y")}},
     {MakePENonTerminal("E"), {MakePETerminal("a")}},
     {MakePENonTerminal("X"), {MakePETerminal("x")}}
    },
    // config_e.grammar
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
     {Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
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
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
     {Parser::kEmptyTerminal, {Parser::kEmptyTerminal}},
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
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
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
    {{Parser::kEndOfInputTerminal, {Parser::kEndOfInputTerminal}},
     {MakePETerminal("a"), {MakePETerminal("a")}},
     {MakePETerminal("b"), {MakePETerminal("b")}},
     {MakePETerminal("c"), {MakePETerminal("c")}},
     {MakePETerminal("m"), {MakePETerminal("m")}},
     {MakePENonTerminal("S"), {MakePETerminal("a"), MakePETerminal("m")}},
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
     {MakePENonTerminal("S"), {}},
     {MakePENonTerminal("E"), {MakePETerminal("c"), Parser::kEndOfInputTerminal}},
     {MakePENonTerminal("X"), {MakePETerminal("c"), Parser::kEndOfInputTerminal}}
    },
    // config_h.grammar
    {{MakePENonTerminal("S"), {}},
     {MakePENonTerminal("E"), {MakePETerminal("c"), Parser::kEndOfInputTerminal}},
     {MakePENonTerminal("X"), {MakePETerminal("c"), Parser::kEndOfInputTerminal}}
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
    {"E_DASH", ")", MakeProduction("E_DASH", {Parser::kEmptyTerminal.element}, {1})},
    {"E_DASH", "$", MakeProduction("E_DASH", {Parser::kEmptyTerminal.element}, {1})},
    {"T", "id", MakeProduction("T", {"F", "T_DASH"}, {0, 0})},
    {"T", "(", MakeProduction("T", {"F", "T_DASH"}, {0, 0})},
    {"T_DASH", "+", MakeProduction("T_DASH", {Parser::kEmptyTerminal.element}, {1})},
    {"T_DASH", ")", MakeProduction("T_DASH", {Parser::kEmptyTerminal.element}, {1})},
    {"T_DASH", "$", MakeProduction("T_DASH", {Parser::kEmptyTerminal.element}, {1})},
    {"T_DASH", "*", MakeProduction("T_DASH", {"*", "F", "T_DASH"}, {1, 0, 0})},
    {"F", "id", MakeProduction("F", {"id"}, {1})},
    {"F", "(", MakeProduction("F", {"(", "E", ")"}, {1, 0, 1})}
  }
};

struct ParserTestSettings {
  bool test_compute_first{false};
  bool test_compute_follow{false};
  bool test_compute_parsing_table{false};
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
      const Parser::ProductionElementFirstSet& a,
      const Parser::ProductionElementFirstSet& b) const;
    bool CompareProductionElementFollowSets(
      const Parser::ProductionElementFollowSet& a,
      const Parser::ProductionElementFollowSet& b) const;
    bool CompareProductionVectors(
      const ProductionVector& a,
      const ProductionVector& b) const;
    bool IsParsingTableMatch(
      const Parser& p,
      const kParsingTableEntries& expected) const;

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

bool ParserTest::CompareProductionVectors(
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

bool ParserTest::IsParsingTableMatch(const Parser& p,
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

void ParserTest::TestComputeParsingTable() const {

  int pass_count = 0;
  for (std::size_t i = 0; i < kParserParsingTableTestFiles.size(); ++i) {
    const auto& grammar_filename{kParserParsingTableTestFiles.at(i)};
    Parser p{grammar_filename};

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

  ParserTestSettings settings;

  CLI::App app{"parser_test - Parser Test File"};
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

  ParserTest t{settings};
  t.RunTests();

  return 0;
}
