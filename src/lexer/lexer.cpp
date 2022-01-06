// Define the Lexer class
#include "spdlog/spdlog.h"
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <stack>
#include <cctype>
#include "lexer/lexer.hpp"
#include "utils/string_utils.hpp"
#include "utils/file_utils.hpp"
#include "utils/file_location.hpp"

static const std::string kErrorHeader{"LEXER"};

Lexer::Lexer(const std::string& lexer_definition_file_name) :
   lexer_definition_file_{lexer_definition_file_name},
   token_regex_precedence_{GetTokenRegex()},
   keyword_tokens_{GetKeywords()},
   symbol_tokens_{GetSymbols()}{
  spdlog::info("Constructing a Lexer");

  for (const auto& tok_reg : token_regex_precedence_) {
    spdlog::debug("Token {} Regex {}", tok_reg.first, tok_reg.second);
  }

  for (const auto& kw : keyword_tokens_) {
    spdlog::debug("Keyword {}", kw);
  }

  for (const auto& sym : symbol_tokens_) {
    spdlog::debug("Symbol {}", sym);
  }

  ConstructAutomatons();
}

Lexer::~Lexer() {
  automatons_.clear();
}

void Lexer::Reset() {
  input_file_.clear();
  input_file_buffer_.clear();
  lexeme_ptr_ = -1;
}

void Lexer::SetInputFile(const std::string& input_file) {
  input_file_ = input_file;
  input_file_buffer_ = ReadFile(input_file_);
  lexeme_ptr_ = 0;
}

void Lexer::RunLexerOn(const std::string& input_file) {

  SetInputFile(input_file);

  // Setup error handler
  if (error_handlers_.find(input_file) == error_handlers_.end()) {
    error_handlers_.insert({input_file, ErrorHandler{kErrorHeader, input_file}});
  }
  ErrorHandler& error_handler{error_handlers_.at(input_file)};

  std::stack<int> comment_block_stack;

  std::string lexer_output;
  Lexeme lexeme;
  while (GetNextLexeme(&lexeme)) {
    if (lexeme.lexeme.empty() && comment_block_stack.empty()) {
      // Write error to console
      error_handler.ConsolePrint(lexeme.file_location_info.buf_idx, "Cannot identify token");
      continue;
    }

    // ignore whitespaces and comment line
    if (lexeme.token == "WS" ||
        lexeme.token == "COMMENT_LINE") {
      continue;
    }

    if (lexeme.token == "COMMENT_BLOCK_END" && comment_block_stack.empty()) {
      error_handler.ConsolePrint(lexeme.file_location_info.buf_idx,
                                 "Cannot match comment block parens");
      continue;
    }

    // Is this a comment
    if (lexeme.token == "COMMENT_BLOCK_START") {
      comment_block_stack.push(lexeme.file_location_info.buf_idx);
      continue;
    }

    if (lexeme.token == "COMMENT_BLOCK_END") {
      comment_block_stack.pop();
      continue;
    }

    if (!comment_block_stack.empty()) {
      // we are still processing comment block
      continue;
    }


    std::string token_lower;
    std::transform(lexeme.token.begin(), lexeme.token.end(),
                   std::back_inserter(token_lower),
                   [](const char x) { return std::tolower(x); });

    lexer_output = fmt::format("{}{}\n", lexer_output, lexeme.file_location_info.line_no + 1);
    lexer_output = fmt::format("{}{}\n", lexer_output, token_lower);

    const bool is_keyword{keyword_tokens_.find(lexeme.token) != keyword_tokens_.end()};
    const bool is_symbol{symbol_tokens_.find(lexeme.token) != symbol_tokens_.end()};
    if (!is_keyword && !is_symbol) {
      if (lexeme.token == "STRING") {
        // remove enclosing quotes
        lexer_output = fmt::format("{}{}\n", lexer_output, lexeme.lexeme.substr(1, lexeme.lexeme.length() - 2));
      } else {
        lexer_output = fmt::format("{}{}\n", lexer_output, lexeme.lexeme);
      }
    }
  }

  if (!comment_block_stack.empty()) {
    // we never encountered a comment_block_end
    error_handler.ConsolePrint(
      comment_block_stack.top(), "Cannot idenitfy a matching end token");
  }

  // write string to output file
  WriteToFile(fmt::format("{}.cclex", input_file), lexer_output);

  Reset();

}

bool Lexer::GetNextLexeme(Lexeme* const lexeme) {
  assert (lexeme);

  // setup file location
  if (file_locations_.find(input_file_) == file_locations_.end()) {
    file_locations_.insert({input_file_, FileLocation{input_file_}});
  }
  FileLocation& file_location{file_locations_.at(input_file_)};

  const std::size_t buflen{input_file_buffer_.length()};

  if (lexeme_ptr_ >= static_cast<int>(buflen)) {
    return false;
  }

  // Match lexeme
  std::string lexeme_text;
  std::string token;
  const int lexeme_test_idx{lexeme_ptr_};
  lexeme_ptr_ = GetLexemeAt(input_file_buffer_, lexeme_ptr_, &lexeme_text, &token);
  *lexeme = Lexeme{lexeme_text, token, file_location.GetFileLocationInfo(lexeme_test_idx)};

  return true;
}


void Lexer::ConstructAutomatons() {
  spdlog::debug("#Tokens and Regex {}", token_regex_precedence_.size());

  for (const auto& tr : token_regex_precedence_) {
    spdlog::debug("{} - {}", tr.first, tr.second);
    auto dfa{std::make_shared<DFA>(tr.second)};
    automatons_.insert({tr.first, dfa});
    // TODO For some reason the following gives an error !!
    //automatons_.insert({tr.first, std::make_shared<DFA>(tr.second)});
  }
}

int Lexer::GetLexemeAt(const std::string& buffer,
		       const std::size_t lexeme_ptr,
                       std::string* const lexeme,
                       std::string* const token) {
  assert (lexeme);
  assert (token);

  // Reset lexeme
  *lexeme = std::string{};
  *token = std::string{};

  std::unordered_map<std::string, std::shared_ptr<DFA>> tokens_automatons{automatons_};

  auto reset_dfas = [&]() {
    for (auto& token_automaton : automatons_) {
      token_automaton.second->Reset();
    }
  };

  auto move_dfas_on_symbol = [&](const char symbol) {
    for (auto& token_automaton : tokens_automatons) {
      token_automaton.second->MoveOnSymbol(symbol);
    }
  };

  auto remove_error_dfas = [&]() {
    // find error dfas
    std::vector<std::string> error_tokens;
    for (const auto& token_automaton : tokens_automatons) {
      if (token_automaton.second->InErrorState()) {
	error_tokens.emplace_back(token_automaton.first);
      }
    }

    for (const auto& error_token : error_tokens) {
      tokens_automatons.erase(tokens_automatons.find(error_token));
    }
  };

  auto tokens_in_accepting_state = [&]() -> std::set<std::string> {
    std::set<std::string> accepting_tokens;
    for (const auto& token_automaton : tokens_automatons) {
      if (token_automaton.second->InAcceptingState()) {
	accepting_tokens.insert(token_automaton.first);
      }
    }
    return accepting_tokens;
  };

  auto get_top_token_in_accepting_state = [&](const std::set<std::string>& accepting_state_tokens) -> std::string {
    for (const auto& token_regex : token_regex_precedence_) {
      if (accepting_state_tokens.find(token_regex.first) != accepting_state_tokens.end()) {
        return token_regex.first;
      }
    }
    return {};
  };

  struct LastMatch {
    int last_match_ptr{-1};
    std::set<std::string> match_tokens;
  };

  // Initialize last match to invalid state
  LastMatch last_match;
  const std::size_t buflen{buffer.length()};
  assert (lexeme_ptr < buflen);
  std::size_t forward_ptr{lexeme_ptr};

  // Reset dfa before start
  reset_dfas();

  while (forward_ptr < buflen) {
    const char symbol{buffer.at(forward_ptr)};

    // Move dfas
    move_dfas_on_symbol(symbol);

    // Remove all dfas in error state
    remove_error_dfas();
    // If there are no dfas left - breakout
    if (tokens_automatons.empty()) { break; }

    // Get all tokens that are in accepting state
    const auto accepting_tokens{tokens_in_accepting_state()};

    if (!accepting_tokens.empty()) {
      // Update last match
      last_match = LastMatch{static_cast<int>(forward_ptr), accepting_tokens};
    }

    forward_ptr++;
  }

  // Reset dfa after processing
  reset_dfas();

  // If there has been no match - throw error
  if (last_match.last_match_ptr == -1) {
    spdlog::debug(fmt::format("No match for lexeme @ {} -{})",
                              lexeme_ptr,
                              buffer.substr(lexeme_ptr,  30)));
    return lexeme_ptr + 1;
  } else {
    // Update output lexeme
    assert (last_match.last_match_ptr >= static_cast<int>(lexeme_ptr));
    assert (!last_match.match_tokens.empty());
    *lexeme = buffer.substr(lexeme_ptr,
                            last_match.last_match_ptr - lexeme_ptr + 1);
    *token = get_top_token_in_accepting_state(last_match.match_tokens);
    spdlog::debug(fmt::format("lexeme @ {} - ({}, {})",
                              lexeme_ptr, *lexeme, *token));
    return last_match.last_match_ptr + 1;
  }
}

// Lexer Definition File read utilities
#define COMMENT_START "//"
#define DEFINITION_START "DEFINITION"
#define KEYWORD_START "KEYWORDS"
#define SYMBOL_START "SYMBOLS"
#define TOKEN_REGEX_SEP ':'

static bool IsLexDefComment(const std::string& s) {
  // A comment starts with "//"
  const std::string comment_start(COMMENT_START);
  if (s.length() < comment_start.length()) { return false; }
  if (s.substr(0, comment_start.length()) == comment_start) { return true; }
  return false;
}

static bool IsLexDefDefinitionStartLine(const std::string& s) {
  const std::string def_start(DEFINITION_START);
  // The definitions start with "DEFINITIONS"
  if (s.length() < def_start.length()) { return false; }
  if (s.substr(0, def_start.length()) == def_start) { return true; }
  return false;
}

static bool IsLexDefKeywordStartLine(const std::string& s) {
  const std::string kw_start(KEYWORD_START);
  // The definitions start with "KEYWORDS"
  if (s.length() < kw_start.length()) { return false; }
  if (s.substr(0, kw_start.length()) == kw_start) { return true; }
  return false;
}

static bool IsLexDefSymbolStartLine(const std::string& s) {
  const std::string symbol_start(SYMBOL_START);
  // The definitions start with "SYMBOLS"
  if (s.length() < symbol_start.length()) { return false; }
  if (s.substr(0, symbol_start.length()) == symbol_start) { return true; }
  return false;
}

std::vector<std::pair<std::string, std::string>> Lexer::GetTokenRegex() {

  std::vector<std::pair<std::string, std::string>> token_regex;
  const std::vector<std::string> lex_def_lines{ReadFileLines(lexer_definition_file_)};

  bool expect_token_regex{false};
  for (const std::string& line : lex_def_lines) {
    const std::string trimmed{Trim(line)};

    // ignore if it is a comment
    if (IsLexDefComment(trimmed)) { continue; }

    if (expect_token_regex) {
      // Make sure that the section hasn't ended
      expect_token_regex = !IsLexDefKeywordStartLine(trimmed) && !IsLexDefSymbolStartLine(trimmed);
    }

    if (!expect_token_regex) {
      auto defs_start_line = IsLexDefDefinitionStartLine(trimmed);
      if (defs_start_line) {
        spdlog::debug("Definitions start encountered...");
      }
      expect_token_regex = defs_start_line;
      continue;
    }

    // Parse token : regex
    const std::size_t separator_pos{trimmed.find_first_of(TOKEN_REGEX_SEP)};
    if (separator_pos == std::string::npos) {
      spdlog::error(fmt::format("Cannot find TOKEN_REGEX_SEPARATOR {}", trimmed));
      continue;
    }

    const std::vector<std::string> parts{trimmed.substr(0, separator_pos),
                                         trimmed.substr(separator_pos + 1, std::string::npos)};
    const std::string token{Trim(parts.at(0))};
    const std::string regex_wp{Trim(parts.at(1))};
    // Regex is surrounded by {} .. Remove the parens
    const std::string regex{regex_wp.substr(1, regex_wp.length() - 2)};

    //assert (token_regex.find(token) == token_regex.end());
    token_regex.push_back({token, regex});
  }

  return token_regex;
}

std::unordered_set<std::string> Lexer::GetKeywords() {
  std::unordered_set<std::string> keywords;
  const std::vector<std::string> lex_def_lines{ReadFileLines(lexer_definition_file_)};

  bool expect_keyword{false};
  for (const std::string& line : lex_def_lines) {
    const std::string trimmed{Trim(line)};

    // ignore if it is a comment
    if (IsLexDefComment(trimmed)) { continue; }

    if (expect_keyword) {
      // Make sure that the section hasn't ended
      expect_keyword = !IsLexDefDefinitionStartLine(trimmed) && !IsLexDefSymbolStartLine(trimmed);
    }

    if (!expect_keyword) {
      auto kw_start_line = IsLexDefKeywordStartLine(trimmed);
      if (kw_start_line) {
        spdlog::debug("Keywords start encountered...");
      }
      expect_keyword = kw_start_line;
      continue;
    }

    // the entire line is a keyword
    keywords.insert(trimmed);
  }

  return keywords;
}

std::unordered_set<std::string> Lexer::GetSymbols() {

  std::unordered_set<std::string> symbols;
  const std::vector<std::string> lex_def_lines{ReadFileLines(lexer_definition_file_)};

  bool expect_symbol{false};
  for (const std::string& line : lex_def_lines) {
    const std::string trimmed{Trim(line)};

    // ignore if it is a comment
    if (IsLexDefComment(trimmed)) { continue; }

    if (expect_symbol) {
      // Make sure that the section hasn't ended
      expect_symbol = !IsLexDefDefinitionStartLine(trimmed) && !IsLexDefKeywordStartLine(trimmed);
    }

    if (!expect_symbol) {
      auto symbol_start_line = IsLexDefSymbolStartLine(trimmed);
      if (symbol_start_line) {
        spdlog::debug("Symbol start encountered...");
      }
      expect_symbol = symbol_start_line;
      continue;
    }

    // the entire line is a symbol
    symbols.insert(trimmed);
  }

  return symbols;
}

#undef COMMENT_START
#undef DEFINITION_START
#undef KEYWORD_START
#undef SYMBOL_START
#undef TOKEN_REGEX_SEP
