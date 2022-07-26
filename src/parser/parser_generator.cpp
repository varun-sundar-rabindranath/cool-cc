#include <parser/parser_generator.hpp>
#include <parser/grammar_file_parser.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

const ProductionElement ParserGenerator::kEndOfInputTerminal =
                        ProductionElement{ProductionElementType::TERMINAL, "$"};
const ProductionElement ParserGenerator::kEmptyTerminal =
                        ProductionElement{ProductionElementType::TERMINAL,
                                          kGrammarFileEmptyTerminal};

ParserGenerator::ParserGenerator(const std::string& grammar_filename) :
  grammar_filename_{grammar_filename},
  terminals_{},
  non_terminals_{},
  productions_{},
  productions_semantic_rules_{},
  productions_semantic_rules_includes_{},
  terminal_id_map_{},
  non_terminal_id_map_{},
  production_id_map_{} {

  spdlog::debug("ParserGenerator({})", grammar_filename_);

  ParseGrammarFile(grammar_filename_, &terminals_, &non_terminals_,
		   &productions_, &productions_semantic_rules_,
		   &productions_semantic_rules_includes_, &start_symbol_);

  // Add End Of Input terminals to the terminals_ and start_symbol production
  terminals_.push_back(kEndOfInputTerminal);
  for (auto& p : productions_) {
    if (p.left == start_symbol_) {
      p.right.push_back(kEndOfInputTerminal);
      // It is okay to break. ParseGrammarFile guarantees that the start_symbol_
      // has only one production
      break;
    }
  }

  // Update terminal -> id, non_terminal -> id, produciton -> id maps
  for (std::size_t id = 0; id < terminals_.size(); ++id) {
    assert (terminal_id_map_.find(terminals_.at(id)) == terminal_id_map_.end());
    terminal_id_map_.insert({terminals_.at(id), id});
  }
  for (std::size_t id = 0; id < non_terminals_.size(); ++id) {
    assert (non_terminal_id_map_.find(non_terminals_.at(id)) == non_terminal_id_map_.end());
    non_terminal_id_map_.insert({non_terminals_.at(id), id});
  }
  for (std::size_t id = 0; id < productions_.size(); ++id) {
    assert (production_id_map_.find(productions_.at(id)) == production_id_map_.end());
    production_id_map_.insert({productions_.at(id), id});
  }

  Dump();
}

void ParserGenerator::WriteSemanticRules(const std::string& filename) const {
  spdlog::debug("Write semantic rules to {}", filename);

  static const std::string kUsingStatements{
    std::string("using ParseTreeNodePTR = std::shared_ptr<ParseTreeNode>;\n") +
    std::string("using ParseTreeNodePTRS = std::vector<ParseTreeNodePTR>;\n") +
    std::string("using ParseTreeNodeFPTR = ParseTreeNodePTR (*)(const ParseTreeNodePTRS&);\n")
  };

  auto make_semantic_rule_function_signature = [](const std::string& fname) -> std::string {
    return fmt::format("ParseTreeNodePTR {}(const ParseTreeNodePTRS& PTN_right)",
		       fname);
  };

  auto define_production_macros = [](const std::size_t n) -> std::string {
    std::string s;
    for (std::size_t i = 0; i < n; ++i) {
      s += fmt::format("#define R{} PTN_right.at({})\n", i, i);
    }
    return s;
  };

  auto undef_production_macros = [](const std::size_t n) -> std::string {
    std::string s;
    for (std::size_t i = 0; i < n; ++i) {
      s += fmt::format("#undef R{}\n", i);
    }
    return s;
  };

  auto make_production_function_name = [](const std::size_t production_idx) -> std::string {
    return fmt::format("P{}", production_idx);
  };

  auto make_production_function_definition =
    [make_semantic_rule_function_signature, define_production_macros, undef_production_macros]
    (const Production& p, const std::string& production_function_name,
     const std::string& semantic_rule) -> std::string {

      std::string s;

      // Add comment
      s += fmt::format("\n\n/*** Production : {} ****/\n", p.to_string());

      // macros defining R[0-9]+
      s += define_production_macros(p.right.size());

      s += make_semantic_rule_function_signature(production_function_name) + semantic_rule;

      // undef the previously defined macros
      s += undef_production_macros(p.right.size());

      return s;
    };

  auto make_production_function_map = [make_production_function_name]
    (const ProductionVector& productions,
     const ProductionIDMap& production_id_map) -> std::string {
      std::string map_defn;
      // Add map include
      map_defn += "#include<unordered_map>\n";

      // Start map definition
      map_defn +=
        "extern const std::unordered_map<std::string,ParseTreeNodeFPTR> PRODUCTION_FUNCTION_MAP {\n";

      for(const auto& production : productions) {
        const auto production_function_name{make_production_function_name(
            production_id_map.at(production))};
	map_defn += fmt::format("\t {{ \"{}\", &{} }},\n",
                                production_function_name,
                                production_function_name);
      }

      // End map definition
      map_defn += "\t };\n";

      return map_defn;
    };

  // Open file stream
  std::fstream f;
  f.open(filename, std::fstream::out | std::fstream::trunc);
  if (!f.is_open()) {
    throw std::runtime_error(fmt::format("{} - Open failed | {}", filename, strerror(errno)));
  }

  // Write headers
  for (const auto& h : productions_semantic_rules_includes_) {
    f << h << std::endl;
  }

  // Write using statements
  f << kUsingStatements << std::endl;

  // Define MPTN - Make ParseTreeNode
  f << fmt::format("#define MPTN(arg) ParseTreeNodePTR(dynamic_cast<ParseTreeNode*>(arg))") << std::endl;

  // Add headers that are required by this code-generation block
  f << "#include <memory>" <<std::endl;

  // Write production semantic rules
  assert (productions_.size() == productions_semantic_rules_.size());
  for (std::size_t i = 0; i < productions_.size(); ++i) {
    const auto& p{productions_.at(i)};
    const auto& semantic_rule{productions_semantic_rules_.at(i)};

    f << make_production_function_definition(
            p, make_production_function_name(i), semantic_rule);
  }

  f << make_production_function_map(productions_, production_id_map_);

  f << fmt::format("#undef MPTN") <<std::endl;

  f.close();
}

// setters
void ParserGenerator::SetTerminals(const ProductionElementVector& terminals) {
  terminals_ = terminals;
}

void ParserGenerator::SetNonTerminals(const ProductionElementVector& non_terminals) {
  non_terminals_ = non_terminals;
}

void ParserGenerator::SetProductions(const ProductionVector& productions) {
  productions_ = productions;
}

// getters
ProductionElementVector ParserGenerator::GetTerminals() const {
  return terminals_;
}

ProductionElementVector ParserGenerator::GetNonTerminals() const {
  return non_terminals_;
}

ProductionVector ParserGenerator::GetProductions() const {
  return productions_;
}

void ParserGenerator::Dump() const {

  spdlog::debug("Grammar file - {}", grammar_filename_);

  spdlog::debug("Terminals ...");
  for (const auto& t : terminals_) {
    spdlog::debug("{}", t.to_string());
  }

  spdlog::debug("Non Terminals ...");
  for (const auto& nt : non_terminals_) {
    spdlog::debug("{}", nt.to_string());
  }

  spdlog::debug("Productions & Semantic Rules ...");
  for (std::size_t i = 0; i < productions_.size(); ++i) {
    spdlog::debug("{}", productions_.at(i).to_string());
    spdlog::debug("{}", productions_semantic_rules_.at(i));
  }

  spdlog::debug("Start Symbol {} ", start_symbol_.to_string());
}
