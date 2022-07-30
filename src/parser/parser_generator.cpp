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

void ParserGenerator::WriteGrammerObjects(std::fstream& file_stream) const {

  auto production_element_string = [](const ProductionElement& pe) -> std::string {

    std::string pe_string;

    // Determine production element type
    const std::string pe_type_string =
        pe.type == ProductionElementType::TERMINAL ?
        "ProductionElementType::TERMINAL" : "ProductionElementType::NON_TERMINAL";

    pe_string += fmt::format("{{ {}, \"{}\" }}", pe_type_string, pe.element);

    return pe_string;
  };

  auto define_production_element =
    [production_element_string](
      const ProductionElement& pe, const std::string& var_name) -> std::string {

      std::string definition;

      // Add required headers
      definition += "#include <parser/production.hpp> // ProductionElement\n";

      // Define production element
      definition += fmt::format("extern const ProductionElement {} {};",
                         var_name, production_element_string(pe));
      return definition;
    };

  auto define_production_element_vector =
    [production_element_string]
    (const ProductionElementVector& pes, const std::string& var_name)
    -> std::string {

    std::string definition;

    // Add required headers
    definition += "#include <parser/production.hpp> // ProductionElementVector\n";

    // Start ProductionElementVector definition
    definition += fmt::format("extern const ProductionElementVector {}{{\n",
                              var_name);

    // Define production elements
    for (std::size_t i = 0; i < pes.size(); ++i) {
      definition += fmt::format(" ProductionElement {}",
                                production_element_string(pes.at(i)));
      definition += i != pes.size() - 1 ? ",\n" : "\n";
    }

    // Close ProductionElementVector definition
    definition += " };";
    return definition;
  };

  auto define_production_element_id_map =
    [production_element_string]
    (const ProductionElementIDMap& pe_id_map, const std::string& var_name)
    -> std::string {

      std::string definition;

      // Add required headers
      definition += "#include <parser/parser_generator.hpp> // ProductionElementIDMap\n";
      definition += "#include <parser/production.hpp> // ProductionElementVector\n";

      // Start ProductionElementIDMap definition
      definition += fmt::format(
        "extern const ProductionElementIDMap {}{{\n", var_name);

      // Define production element map entries
      std::size_t element_idx{0};
      for (const auto& pe_id : pe_id_map) {
        definition += fmt::format("  {{ ProductionElement{}, {} }}",
                        production_element_string(pe_id.first), pe_id.second);
        definition += element_idx == pe_id_map.size() - 1 ? "\n" : ",\n";
        element_idx++;
      }

      // End ProductionElementIDMap definition
      definition += " };";

      return definition;
    };

  auto production_string = [production_element_string] (const Production& p) -> std::string {

    std::string definition;

    definition += " Production {\n";
    // Write the left production element
    definition += fmt::format("   ProductionElement{},\n",
                              production_element_string(p.left));

    // Write all the right side productions as a vector
    definition += "   ProductionElementVector{\n";
    for (std::size_t rpe_i = 0; rpe_i < p.right.size(); ++rpe_i) {
      const bool is_last_rpe{rpe_i == p.right.size() - 1};
      const auto& rpe{p.right.at(rpe_i)};
      definition += fmt::format("     ProductionElement{}",
                                production_element_string(rpe));
      definition += is_last_rpe ? "\n" : ",\n";
    }

    // Close out ProductionElementVector definition
    definition += "   }";
    // Close out production definition
    definition += "}";

    return definition;
  };

  auto define_production_vector =
    [production_string]
    (const ProductionVector& productions,
     const std::string& var_name) -> std::string {
      std::string definition;

      // Add required headers
      definition += "#include <parser/production.hpp> // ProductionVector\n";

      // Start production vector definition
      definition += fmt::format("extern const ProductionVector {} {{\n", var_name);

      for (std::size_t pi = 0; pi < productions.size(); ++pi) {

        const bool is_last_pi{pi == productions.size() - 1};
        const auto& p{productions.at(pi)};
        definition += production_string(p);
        definition += is_last_pi ? "\n" : ",\n";
      }

      // End production vector definition
      definition += "};";

      return definition;
  };

  auto define_production_id_map = [production_string](
    const ProductionIDMap& production_id_map,
    const std::string& var_name) -> std::string {

    std::string definition;

    // Add required headers
    definition += "#include <parser/production.hpp> // Production\n";

    // Instantiate the definition
    definition += fmt::format("extern const ProductionIDMap {} {{\n", var_name);

    // Iterate through the map and add items
    std::size_t p_counter = 0;
    for (const auto& production_id : production_id_map) {
      p_counter++;
      const auto& p = production_id.first;
      const auto& id = production_id.second;
      definition += fmt::format("    {{ {}, {} }}", production_string(p), id);
      definition += p_counter == production_id_map.size() ? "\n" : ",\n";
    }

    // Close definition of production map
    definition += "};";

    return definition;
  };

  // Write Start Symbol
  {
    const std::string definition{
      define_production_element(start_symbol_, "START_SYMBOL")};
    file_stream << "// Start Symbol"<< std::endl << definition << std::endl << std::endl;
  }

  // Write Terminals definition
  {
    const std::string definition{
      define_production_element_vector(terminals_, "TERMINALS_DEFINITION")};
    file_stream << "// Terminals"<< std::endl << definition << std::endl << std::endl;
  }

  // Write Non Terminals definition
  {
    const std::string definition{
      define_production_element_vector(non_terminals_, "NON_TERMINALS_DEFINITION")};
    file_stream << "// Non Terminals"<< std::endl << definition << std::endl << std::endl;
  }

  // Write Terminals - ID map
  {
    const std::string definition {
      define_production_element_id_map(terminal_id_map_,
                                       "TERMINALS_ID_MAP_DEFINITION")};
    file_stream << "// Terminals ID Map"<< std::endl << definition << std::endl << std::endl;
  }

  // Write Non Terminals - ID map
  {
    const std::string definition {
      define_production_element_id_map(non_terminal_id_map_,
                                       "NON_TERMINALS_ID_MAP_DEFINITION")};
    file_stream << "// Non Terminals ID Map"<< std::endl << definition << std::endl << std::endl;
  }

  // Write Productions
  {
    const std::string definition {
      define_production_vector(productions_, "PRODUCTION_VECTOR_DEFINITION")};
    file_stream << "// Productions"<< std::endl << definition << std::endl << std::endl;
  }

  // Write ProductionIDMap
  {
    const std::string definition {
      define_production_id_map(production_id_map_, "PRODUCTION_ID_MAP_DEFINITION")};
    file_stream << "// Production - ID Map"<< std::endl << definition << std::endl << std::endl; 
  }

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
