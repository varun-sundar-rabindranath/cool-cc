#include <parser/recursive_descent_parser.hpp>
#include <parser/production.hpp>
#include <parser/parser_node.hpp>
#include <algorithm>
#include <limits>

extern const ProductionElement START_SYMBOL;
extern const ProductionElementVector TERMINALS_DEFINITION;
extern const ProductionElementIDMap TERMINALS_ID_MAP_DEFINITION;
extern const ProductionElementIDMap NON_TERMINALS_ID_MAP_DEFINITION;
extern const ProductionIDMap PRODUCTION_ID_MAP_DEFINITION;
extern const ProductionVector PRODUCTION_VECTOR_DEFINITION;
extern const std::vector<std::vector<std::vector<std::size_t>>> PARSING_TABLE_DEFINITION;
extern const std::unordered_map<std::string, ParseTreeNodeFPTR> PRODUCTION_FUNCTION_MAP;

const RecursiveDescentParser::ProductionID RecursiveDescentParser::InvalidProductionID{-1};
const RecursiveDescentParser::StateStackEntry RecursiveDescentParser::StackReductionMarker{
  ProductionElement{ProductionElementType::INVALID, ""}, InvalidProductionID};

RecursiveDescentParser::RecursiveDescentParser() :
  parser_state_{PARSER_STATE_PROCESSING},
  state_stack_{},
  reduction_store_{} {

  // ProductionElement::element is treated as lexer tokens - Make a map
  // of the ProductionElement::element string and the ProductionElement
  // itself
  for (const auto& pe : TERMINALS_DEFINITION) {
    token_production_element_map_.insert({pe.element, pe});
  }

  // Inverse of ProductionIDMap
  //std::unordered_map<std::size_t, Production> id_production_map_;
  for (const auto& p_id : PRODUCTION_ID_MAP_DEFINITION) {
    id_production_map_.insert({p_id.second, p_id.first});
  }

  // Intialize parser state with the start symbol
  state_stack_.push(StateStackEntry{START_SYMBOL, InvalidProductionID});
}

ParserState RecursiveDescentParser::ProcessLexeme(const Lexeme& lexeme) {

  auto reduce_stack = [&]() -> void {
    assert (state_stack_.top() == StackReductionMarker);
    const auto sse{state_stack_.top()};
    assert (sse.second != InvalidProductionID);

    // pop the StackReductionMarker
    state_stack_.pop();

    // Get the production that has been expanded
    const std::size_t production_id{static_cast<std::size_t>(sse.second)};

    // Empty the reduction store into a vector and call the production's
    // semantic rule
    std::vector<ParseTreeNodePTR> args;
    while (!reduction_store_.empty()) { args.push_back(reduction_store_.top()); reduction_store_.pop(); };

    // TODO : Find a better way to index the production function
    auto& production_fn{
      PRODUCTION_FUNCTION_MAP.at(fmt::format("P{}", production_id))};

    auto node{production_fn(args)};
    // Push node back in reduction_store_ to serve as arguments for further
    // reduction, if any
    reduction_store_.push(node);
    // The stack is reduced - Remove the top
    state_stack_.pop();
  };

  auto expand_stack = [&](const std::size_t production_id) -> void {

    // We are expanding the stack with the given production_id. replace
    // the top of the stack, that has InvalidProductionID, with a proper
    // ProductionID
    {
      auto sse{state_stack_.top()};
      state_stack_.pop();
      sse.second = production_id;
      state_stack_.push(sse);
    }

    // Place a stack reduction marker before expanding
    state_stack_.push(StackReductionMarker);

    const auto& production{id_production_map_.at(production_id)};

    // Place the right side of the production in the reverse order
    for (int i = production.right.size() - 1; i >= 0; i--) {
      state_stack_.push(StateStackEntry{production.right.at(i), InvalidProductionID});
    }
  };

  // Refuse to process if the parser is not in PARSER_STATE_PROCESSING state
  if (parser_state_ != PARSER_STATE_PROCESSING) {
    return parser_state_;
  }

  bool lexeme_consumed{false};
  while ((not lexeme_consumed) && (parser_state_ != PARSER_STATE_ERROR)) {
    const auto sse{state_stack_.top()};

    if (sse == StackReductionMarker) {
      reduce_stack();
      continue;
    }

    const auto& pe{sse.first}; // The element at the top of the stack

    if (pe.type == ProductionElementType::TERMINAL) {
      // The lexeme must match the terminal - Otherwise it is a parser error
      if (lexeme.lexeme != pe.element) {
        parser_state_ = ParserState::PARSER_STATE_ERROR;
        continue;
      }

      // If there is a match - create an empty ParseTreeNode - At the moment
      // the actual value string is not being considered - This is a TODO
      state_stack_.pop();
      reduction_store_.push(ParseTreeNodePTR(nullptr));

      lexeme_consumed = true;
      continue;
    }

    if (pe.type == ProductionElementType::NON_TERMINAL) {
      const ProductionElement& t{token_production_element_map_.at(lexeme.lexeme)};
      // There must be a valid production id in the parsing table to expand.
      // Otherwise it is a parser error
      const std::size_t nt_id{NON_TERMINALS_ID_MAP_DEFINITION.at(pe)};
      const std::size_t t_id{TERMINALS_ID_MAP_DEFINITION.at(t)};
      const auto& production_ids{PARSING_TABLE_DEFINITION.at(nt_id).at(t_id)};
      if (production_ids.size() == 1) {
        // Has too few or too many productions ! - Error
        parser_state_ = ParserState::PARSER_STATE_ERROR;
        continue;
      }
      const std::size_t production_id{production_ids.front()};

      // Expand stack
      expand_stack(production_id);
      continue;
    }
  } // while

  return parser_state_;
}
