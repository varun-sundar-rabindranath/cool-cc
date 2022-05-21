#include <parser/grammar_file_parser.hpp>
#include <stdexcept>
#include <utils/string_utils.hpp>
#include <utils/file_utils.hpp>
#include <queue>
#include <sstream>

// Grammar file parsing functions

static bool IsLineStart(const std::string& pattern, const std::string& text) {
  if (text.length() < pattern.length()) { return false; }
  if (text.substr(0, pattern.length()) == pattern) { return true; }
  return false;
}

static ProductionElement ParseTerminal(const std::string& line) {

  // Parse non terminals; Non Terminals are just words - if there aren't any
  // intervening whitespaces we are good
  if (std::any_of(line.begin(), line.end(), [](const char c) { return isspace(c); })) {
    throw std::runtime_error(fmt::format("{} is not a terminal", line));
  }

  return ProductionElement{ProductionElementType::TERMINAL, line};
}

static ProductionElement ParseNonTerminal(const std::string& line) {

  // Parse non terminals; Non Terminals are just words - if there aren't any
  // intervening whitespaces we are good
  if (std::any_of(line.begin(), line.end(), [](const char c) { return isspace(c); })) {
    throw std::runtime_error(fmt::format("{} is not a non-terminal", line));
  }

  return ProductionElement{ProductionElementType::NON_TERMINAL, line};
}

static void ParseProduction(const std::string& line,
                            std::string* const left_token,
                            std::vector<std::string>* const right_tokens) {
  assert (left_token);
  assert (right_tokens);

  // Parse token : regex
  const std::size_t separator_pos{line.find_first_of(kGrammarFileProductionLRSeparator)};
  if (separator_pos == std::string::npos) {
    spdlog::error(fmt::format("Cannot find PARSER : separator in {}", line));
  }

  const std::vector<std::string> parts{line.substr(0, separator_pos),
                                       line.substr(separator_pos + 1, std::string::npos)};

  // left side of the production should just be a word
  *left_token = Trim(parts.at(0));
  // right side is a bunch of tokens and non-terminals separated by spaces
  std::istringstream ss(Trim(parts.at(1)));
  std::string r_token;
  while(ss >> r_token) { right_tokens->push_back(r_token); }
}

static void ParseProductionSemanticRule(
  std::deque<std::string>* const grammar_def_lines,
  std::string* const semantic_rule) {

  assert (grammar_def_lines);
  assert (semantic_rule);
  semantic_rule->clear();

  auto IsGrammarDefProductionSemanticRuleStartLine = [&](const std::string& line) -> bool {
    return IsLineStart(kGrammarFileProductionSemanticRuleStart, line);
  };

  auto IsGrammarDefProductionSemanticRuleEndLine = [&](const std::string& line) -> bool {
    return IsLineStart(kGrammarFileProductionSemanticRuleEnd, line);
  };

  bool is_semantic_rule = false;

  while (!grammar_def_lines->empty()) {
    const std::string line{grammar_def_lines->front()};
    const std::string trimmed{Trim(line)};

    // ignore empty lines
    if (trimmed.empty()) { grammar_def_lines->pop_front(); continue; }

    is_semantic_rule = is_semantic_rule  || IsGrammarDefProductionSemanticRuleStartLine(line);

    if (!is_semantic_rule) {
      // we are infringing on the next production maybe ! abort
      break;
    }

    *semantic_rule += line + "\n";

    grammar_def_lines->pop_front();

    // is the current line the end of the semantic rule ?
    if (IsGrammarDefProductionSemanticRuleEndLine(line)) {
      break;
    }
  }
}

void ParseGrammarFile(const std::string& grammar_filename,
		      ProductionElementVector* const terminals,
		      ProductionElementVector* const non_terminals,
		      ProductionVector* const productions,
		      std::vector<std::string>* const semantic_rules,
		      ProductionElement* const start_symbol) {
  assert (terminals);
  assert (non_terminals);
  assert (productions);
  assert (start_symbol);

  // clear
  terminals->clear();
  non_terminals->clear();
  productions->clear();

  // create a set for terminals and non-terminals; The find() function is used
  // very frequently in this function.
  ProductionElementSet terminals_set;
  ProductionElementSet non_terminals_set;

  // helpers
  auto IsGrammarDefComment = [&](const std::string& line) -> bool {
    return IsLineStart(kGrammarFileCommentStart, line);
  };

  auto IsGrammarDefProductionsStartLine = [&](const std::string& line) -> bool {
    return IsLineStart(kGrammarFileProductionsStart, line);
  };

  auto IsGrammarDefTerminalsStartLine = [&](const std::string& line) -> bool {
    return IsLineStart(kGrammarFileTerminalsStart, line);
  };

  auto IsEmptyToken = [&](const std::string& token) -> bool {
    return token == kGrammarFileEmptyTerminal;
  };

  auto IsGrammarDefNonTerminalsStartLine = [&](const std::string& line) -> bool {
    return IsLineStart(kGrammarFileNonTerminalsStart, line);
  };


  const std::vector<std::string> grammar_def_lines_vec{ReadFileLines(grammar_filename)};
  std::deque<std::string> grammar_def_lines{grammar_def_lines_vec.cbegin(),
					    grammar_def_lines_vec.cend()};

  /* There are 3 distinct sections in a grammar definition
   * 1. Terminals section
   * 2. Non Terminals section
   * 3. Production section
   * Parse each of these sections and make them a part of the Parser's state
   */

  bool is_terminals_section{false};
  bool is_non_terminals_section{false};
  bool is_production_section{false};
  bool is_start_symbol{false};

  while (!grammar_def_lines.empty()) {
    const std::string line{grammar_def_lines.front()};
    const std::string trimmed{Trim(line)};

    // ignore empty lines
    if (trimmed.empty()) { grammar_def_lines.pop_front(); continue; }

    // ignore if it is a comment
    if (IsGrammarDefComment(trimmed)) { grammar_def_lines.pop_front(); continue; }

    // check if this is a section start
    if (IsGrammarDefTerminalsStartLine(line)) {
      is_terminals_section = true;
      is_non_terminals_section = false;
      is_production_section = false;
      is_start_symbol = false;
      grammar_def_lines.pop_front();
      continue;
    }
    if (IsGrammarDefNonTerminalsStartLine(line)) {
      is_terminals_section = false;
      is_non_terminals_section = true;
      is_production_section = false;
      is_start_symbol = true; // The first symbol in the NT section is the start symbol
      grammar_def_lines.pop_front();
      continue;
    }
    if (IsGrammarDefProductionsStartLine(line)) {
      is_terminals_section = false;
      is_non_terminals_section = false;
      is_production_section = true;
      is_start_symbol = false;
      grammar_def_lines.pop_front();
      continue;
    }

    if (is_terminals_section) {
      assert (!is_non_terminals_section && !is_production_section);
      auto t{ParseTerminal(trimmed)};
      if (non_terminals_set.find(t) != non_terminals_set.end()) {
        throw std::runtime_error(
                fmt::format("{} is already registered as a Non Terminal", trimmed));
      }
      terminals_set.insert(t);
      terminals->push_back(t);
      grammar_def_lines.pop_front();
      continue;
    }

    if (is_non_terminals_section) {
      assert (!is_terminals_section && !is_production_section);
      auto nt{ParseNonTerminal(trimmed)};
      if (terminals_set.find(nt) != terminals_set.end()) {
        throw std::runtime_error(
                fmt::format("{} is already registered as a Terminal", trimmed));
      }
      non_terminals_set.insert(nt);
      non_terminals->push_back(nt);
      if (is_start_symbol) { *start_symbol = nt; is_start_symbol = false; }
      grammar_def_lines.pop_front();
      continue;
    }

    if (is_production_section) {
      assert (!is_terminals_section && !is_non_terminals_section);

      std::string left_token;
      std::vector<std::string> right_tokens;
      ParseProduction(trimmed, &left_token, &right_tokens);

      // Make left side of the production
      const ProductionElement left_side{ProductionElementType::NON_TERMINAL, left_token};
      // left token must be a non-terminal
      if (non_terminals_set.find(left_side) == non_terminals_set.end()) {
        throw std::runtime_error(
                    fmt::format("{} : Left side of the production is not a Non Terminal", left_token));
      }

      // Make right side of the production
      std::vector<ProductionElement> right_side;
      for (const auto& r_token : right_tokens) {

        // Sanity check
        if (IsEmptyToken(r_token)) {
          if (right_tokens.size() != 1) {
            throw std::runtime_error(
                      fmt::format("Production {} is ill-formed. Can't have empty in the mix", trimmed));
          }
        }

        // right side tokens can be either terminals or non-terminals
        const ProductionElement r_token_t{ProductionElementType::TERMINAL, r_token};
        const ProductionElement r_token_nt{ProductionElementType::NON_TERMINAL, r_token};
        if (terminals_set.find(r_token_t) != terminals_set.end()) {
          right_side.push_back(r_token_t);
          continue;
        }
        if (non_terminals_set.find(r_token_nt) != non_terminals_set.end()) {
          right_side.push_back(r_token_nt);
          continue;
        }
        throw std::runtime_error(
                fmt::format("In {} | {} is neither a terminal / non-terminal",
                            trimmed, r_token));
      }

      productions->push_back(Production{left_side, right_side});
      grammar_def_lines.pop_front();

      // Expect a semantic rule definition in this line
      std::string production_semantic_rule;
      ParseProductionSemanticRule(&grammar_def_lines, &production_semantic_rule);
      semantic_rules->push_back(production_semantic_rule);
      continue;
    }

  }

  // Check that the #semantic rules match the #productions
  if (productions->size() != semantic_rules->size()) {
    throw std::invalid_argument(
	    fmt::format("Found {} productions but {} semantic rules",
			productions->size(), semantic_rules->size()));
  }

  // Check that the start_symbol has one and only one production
  std::size_t n_start_symbol_productions = 0;
  for (const auto& p : *productions) {
    if (p.left == *start_symbol) { n_start_symbol_productions++; }
  }
  if (n_start_symbol_productions != 1) {
    throw std::invalid_argument(
	    fmt::format("Start Symbol can have only one production. But found {} productions",
			n_start_symbol_productions));
  }
}

