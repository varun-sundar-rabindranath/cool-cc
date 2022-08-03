#pragma once
/**
 * This file declares all the functions that are used in parsing the grammar file
 */

#include <parser/production.hpp>
#include <string>

static constexpr char const kGrammarFileCommentStart[] = "//";
static constexpr char const kGrammarFileProductionSemanticRuleIncludesStart[] = "INCLUDES";
static constexpr char const kGrammarFileProductionsStart[] = "PRODUCTIONS";
static constexpr char const kGrammarFileTerminalsStart[] = "TERMINALS";
static constexpr char const kGrammarFileNonTerminalsStart[] = "NONTERMINALS";
static constexpr char const kGrammarFileProductionLRSeparator[] = ":";
static constexpr char const kGrammarFileEmptyTerminal[] = "%empty";
static constexpr char const kGrammarFileProductionSemanticRuleStart[] = "{";
static constexpr char const kGrammarFileProductionSemanticRuleEnd[] = "}";

/**
 * The function guarantees that,
 * 1. The returned vectors of terminals_ and non_terminals_ do not contain any
 *    duplicates
 * 2. The start_symbol has one and only one production
 */
void ParseGrammarFile(const std::string& grammar_filename,
		      ProductionElementVector* const terminals,
		      ProductionElementVector* const non_terminals,
		      ProductionVector* const productions,
		      std::vector<std::string>* const semantic_rules,
		      std::vector<std::string>* const semantic_rules_includes,
		      ProductionElement* const start_symbol);
