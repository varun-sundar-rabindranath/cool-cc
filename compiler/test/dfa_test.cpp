#include <iostream>
#include <spdlog/common.h>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>
#include <lexer/dfa.hpp>

using namespace std;

using VECTOR_STRING = std::vector<std::string>;

const std::string MISC_REGEX{"((a|b)*)abb"};
const VECTOR_STRING MISC_PASS {"abb", "aabb", "babb", "ababb"};
const VECTOR_STRING MISC_FAIL {"abba", "bbba", ""};

const std::string CLASS_REGEX{"(class|Class)"};
const VECTOR_STRING CLASS_PASS {"class", "Class"};
const VECTOR_STRING CLASS_FAIL {"class1", "CLASS", "clASs"};

const std::string INTEGERS_REGEX{"(0|[1-9]([0-9]*))"};
const VECTOR_STRING INTEGERS_PASS {"0", "10", "900200"};
const VECTOR_STRING INTEGERS_FAIL {"000", "01", "", "00123"};

const std::string IDENTIFIER_REGEX{"[a-z]([A-Za-z_]*)"};
const VECTOR_STRING IDENTIFIER_PASS {"a", "aA", "aAF", "aA_", "a_AF_"};
const VECTOR_STRING IDENTIFIER_FAIL {"A", "Aa", "ABC", "ABC__", ""};

const std::string TYPE_REGEX{"[A-Z]([A-Za-z_]*)"};
const VECTOR_STRING TYPE_PASS {"A", "Aa", "ABC", "A_B_C_"};
const VECTOR_STRING TYPE_FAIL {"a", "aA", "aAF", "aA_", "a_AF_", ""};

const std::string SELF_TYPE_REGEX{"SELF_TYPE"};
const VECTOR_STRING SELF_TYPE_PASS {"SELF_TYPE"};
const VECTOR_STRING SELF_TYPE_FAIL {"ELF_TYPE", "SELF_TYP", "ELF_TYP", ""};

const std::string SELF_IDENTIFIER_REGEX{"self"};
const VECTOR_STRING SELF_IDENTIFIER_PASS {"self"};
const VECTOR_STRING SELF_IDENTIFIER_FAIL {"SELF", ""};

const std::string STRINGS_REGEX{"\"(((\\\\.)|([^\\\\\"])|([W-S]))*)\""};
const VECTOR_STRING STRINGS_PASS {"\"\"", "\"a\"", "\"abc. abc\"",
                                  "\"abc\\nabc\"", "\"abc.ab\\v\"",
				  "\"Hello\\\", World.\\n\"",
				  "\" inherits Closure {\n\"",
				  "\"  apply(y : EvalObject) : EvalObject {\n\"",
				  "\"    { out_string(\\\"Applying closure \\\"\"",
				  "\"\\n\\\");\n\"",
				  "\"      x <- y;\n\"",
				  "\";}};\n\"",
				  "\"};\n\""};
const VECTOR_STRING STRINGS_FAIL {"", "\"hello\"hello", "\"hello\"hello\"",
                                  "\"abc\babc\""};

const std::string COMMENT_LINE_REGEX{"--(([^E-LE-F])*)([E-L]|[E-F])"};
const VECTOR_STRING COMMENT_LINE_PASS {"-- hello \n", "--hello\n",
"-- conforms to the return type List, because Cons is a subclass of\n",
"-- List.\n"};
const VECTOR_STRING COMMEN_LINE_FAIL {"-- hello", "hello\n", "-hello\n"};

const std::string COMMENT_BLOCK_START_REGEX{"\\(\\*"};
const VECTOR_STRING COMMENT_BLOCK_START_PASS {"(*"};
const VECTOR_STRING COMMENT_BLOCK_START_FAIL {" (*", "( *", "(* "};

const std::string COMMENT_BLOCK_END_REGEX{"\\*\\)"};
const VECTOR_STRING COMMENT_BLOCK_END_PASS {"*)"};
const VECTOR_STRING COMMENT_BLOCK_END_FAIL {" *)", "* )", "*) "};

void dfa_test() {

#define TEST(regex, passes, fails)					  \
  {									  \
    spdlog::info(fmt::format("Testing regex {}", regex));		  \
    auto dfa{DFA(regex)};						  \
    for (const auto& tc : passes) {				          \
      spdlog::debug(fmt::format("Testing {}", tc));		          \
      const auto test{dfa.Test(tc)};					  \
      if (!test) {							  \
	spdlog::error(fmt::format("{} dfa.Test({}) should pass but failed !", regex, tc)); \
      }									  \
    }									  \
    for (const auto& tc : fails) {					  \
      const auto test{dfa.Test(tc)};					  \
      if (test) {							  \
	spdlog::error(fmt::format("{} dfa.Test({}) should fail but passed ", regex, tc));  \
      }									  \
    }									  \
  }

  TEST(MISC_REGEX, MISC_PASS, MISC_FAIL)
  TEST(CLASS_REGEX, CLASS_PASS, CLASS_FAIL)
  TEST(INTEGERS_REGEX, INTEGERS_PASS, INTEGERS_FAIL)
  TEST(IDENTIFIER_REGEX, IDENTIFIER_PASS, IDENTIFIER_FAIL)
  TEST(TYPE_REGEX, TYPE_PASS, TYPE_FAIL)
  TEST(SELF_TYPE_REGEX, SELF_TYPE_PASS, SELF_TYPE_FAIL)
  TEST(SELF_IDENTIFIER_REGEX, SELF_IDENTIFIER_PASS, SELF_IDENTIFIER_FAIL)
  TEST(STRINGS_REGEX, STRINGS_PASS, STRINGS_FAIL)
  TEST(COMMENT_LINE_REGEX, COMMENT_LINE_PASS, COMMEN_LINE_FAIL)
  TEST(COMMENT_BLOCK_START_REGEX, COMMENT_BLOCK_START_PASS, COMMENT_BLOCK_START_FAIL)
  TEST(COMMENT_BLOCK_END_REGEX, COMMENT_BLOCK_END_PASS, COMMENT_BLOCK_END_FAIL)

#undef TEST
}

int main() {

#if defined(CCDEBUG)
  spdlog::set_level(
        static_cast<spdlog::level::level_enum>(spdlog::level::level_enum::debug));
#endif

  spdlog::debug("debug print ");

  dfa_test();

  return 0;
}
