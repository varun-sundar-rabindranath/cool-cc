#ifndef __LEX_CHARACTER_CLASSES_HPP__
#define __LEX_CHARACTER_CLASSES_HPP__
/* Defines the various character classes that the lexer supports */

#include <set>
#include <string>

enum CharacterClass {
    CHARACTER_CLASS_UCA_UCZ = 1,
    CHARACTER_CLASS_LCA_LCZ = 2,
    CHARACTER_CLASS_D0_D9 = 3,
    CHARACTER_CLASS_D1_D9 = 4,
    CHARACTER_CLASS_INVALID = 5
};

class LexCharacterClasses {
public:

  // class def is of the form [0-9], [A-Z], [abc] and such definitions
  static std::set<char> GetCharactersInClass(const std::string& class_def);

  static std::set<char> CharactersFromRange(const char start, const char end);

  static std::set<char> GetCharactersForPeriod();

  static std::set<char> GetAllSupportedSymbols();

private:
    // Character class for [A-Z] i.e. Upper Case A to Upper Case Z
    static constexpr int kCC_UCA_UCZ_ASCII_START = 65;
    static constexpr int kCC_UCA_UCZ_ASCII_END = 90;

    // Character class for [a-z] i.e. Lower Case A to Lower Case Z
    static constexpr int kCC_LCA_LCZ_ASCII_START = 97;
    static constexpr int kCC_LCA_LCZ_ASCII_END = 122;

    // Character class for Digits [0-9]
    static constexpr int kCC_0_9_ASCII_START = 48;
    static constexpr int kCC_0_9_ASCII_END = 57;

    // Character class for Digits [1-9]
    static constexpr int kCC_1_9_ASCII_START = 49;
    static constexpr int kCC_1_9_ASCII_END = 57;
};

#endif // __LEX_CHARACTER_CLASSES_HPP__
