#ifndef __LEXER_HPP__
#define __LEXER_HPP__
// Declare a Lexer class
#include <memory>
#include <string>
#include <unordered_map>
#include <lexer/dfa.hpp>
#include <error_handler/error_handler.hpp>
#include <utils/file_location.hpp>
#include <unordered_map>
#include <unordered_set>

struct Lexeme {
  std::string lexeme;
  std::string token;
  FileLocationInfo file_location_info;
};

class Lexer {
public:
  Lexer(const std::string& lexer_definition_file);
  ~Lexer();

  void RunLexerOn(const std::string& input_file);

  // Lexer as a file processing machine
  void Reset();
  void SetInputFile(const std::string& input_file);
  // return true if the end of file is not reached
  bool GetNextLexeme(Lexeme* const lexeme);

private:
  std::string lexer_definition_file_;
  std::vector<std::pair<std::string, std::string>> token_regex_precedence_;
  std::unordered_set<std::string> keyword_tokens_;
  std::unordered_set<std::string> symbol_tokens_;
  std::unordered_map<std::string, std::shared_ptr<DFA>> automatons_;
  std::unordered_map<std::string, ErrorHandler> error_handlers_;
  std::unordered_map<std::string, FileLocation> file_locations_;

  // Lexer state
  std::string input_file_;
  std::string input_file_buffer_;
  int lexeme_ptr_;


  void ConstructAutomatons();

  // Lexeme matcher - Returns the next position to process
  int GetLexemeAt(const std::string& buffer, const std::size_t lexeme_ptr,
		  std::string* const lexeme, std::string* const token);

  // lex file readers
  std::vector<std::pair<std::string, std::string>> GetTokenRegex();
  std::unordered_set<std::string> GetKeywords();
  std::unordered_set<std::string> GetSymbols();
};

#endif // __LEXER_HPP__
