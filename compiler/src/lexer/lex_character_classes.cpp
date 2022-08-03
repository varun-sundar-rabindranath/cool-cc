#include "lexer/lex_character_classes.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <spdlog/spdlog.h>

std::set<char> LexCharacterClasses::GetCharactersInClass(const std::string& class_def) {

  // A class def is of the form [A-Z], [0-9], [A-Z0-9abc] etc.
  // 1. Need to identify stuff of the form x-x
  // 2. Need to identify individual stuff for [abc] etc
  // 3. Need to idenitfy characters that are escaped
  // 4. Process special characters like . and backslash
  std::set<char> characters;
  assert (!class_def.empty());

  bool not_characters = class_def.at(0) == '^';

  std::size_t cd_idx = class_def.at(0) == '^' ? 1 : 0;
  while (cd_idx < class_def.size()) {
    const char c = class_def.at(cd_idx);
    if (c == '\\') {
      // Move on to the next character and take it literally
      assert (cd_idx + 1 < class_def.size());
      characters.insert(class_def.at(cd_idx + 1));
      cd_idx++;
      cd_idx++;
      continue;
    }

    if (c == '.') {
      throw std::invalid_argument("Encountered Period in a character class. Unsupported");
    }

    // check if this is character class definitions
    if ((cd_idx + 2 < class_def.size()) && (class_def.at(cd_idx + 1) == '-')) {
      auto class_characters{CharactersFromRange(class_def.at(cd_idx),
                                                class_def.at(cd_idx + 2))};
      for (const auto cc : class_characters) {
        characters.insert(cc);
      }
      cd_idx++; // the hyphen character
      cd_idx++; // last character of the character class definition
      cd_idx++; // after character class definition
      continue;
    }

    // What is left isn't a special character. Simply add it
    characters.insert(class_def.at(cd_idx));
    cd_idx++;
  }

  std::set<char> characters_in_class;
  if (not_characters) {
    auto all_characters{GetCharactersForPeriod()};
    std::set_difference(all_characters.begin(), all_characters.end(),
                        characters.begin(), characters.end(),
                        std::inserter(characters_in_class, characters_in_class.end()));
  } else {
    characters_in_class = characters;
  }

  return characters_in_class;
}

std::set<char> LexCharacterClasses::CharactersFromRange(
  const char start, const char end) {

  if (!((start == 'a' && end == 'z') ||
       (start == 'A' && end == 'Z') ||
       (start == '0' && end == '9') ||
       (start == '1' && end == '9') ||
       (start == 'E' && end == 'F') ||
       (start == 'W' && end == 'S') ||
       (start == 'E' && end == 'L'))) {
    throw std::invalid_argument("No character range found");
  }

  std::set<char> characters;

  // Treat E-L as a special character
  if (start == 'E' && end == 'L') {
    characters.insert('\n');
  } else if (start == 'E' && end == 'F') {
    characters.insert(26);
  } else if (start == 'W' && end == 'S') {
    characters.insert(' ');
    characters.insert('\t');
    characters.insert('\n');
    characters.insert('\v');
    characters.insert('\f');
    characters.insert('\r');
  } else {
    char c = start;
    while (c <= end) {
      characters.insert(c);
      c++;
    }
  }

  return characters;
}

std::set<char> LexCharacterClasses::GetCharactersForPeriod() {
  return GetAllSupportedSymbols();
}

std::set<char> LexCharacterClasses::GetAllSupportedSymbols() {
  // - Add all ascii characters from 33 to 126
  // - Add all whitespaces

  std::set<char> characters;

  // Adding characters from 33 to 126
  for (int ascii_idx = 33; ascii_idx <= 126; ++ascii_idx) {
    const char ascii_char = static_cast<char>(ascii_idx);
    characters.insert(ascii_char);
  }

  // Adding whitespaces
  characters.insert('\t');
  characters.insert('\n');
  characters.insert(' ');

  return characters;
}

