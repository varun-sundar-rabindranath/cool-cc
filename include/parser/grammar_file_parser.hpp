#pragma once
/**
 * This file declares all the functions that are used in parsing the grammar file
 */

#include <parser/production.hpp>
#include <string>

static constexpr char const kGrammarFileCommentStart[] = "//";
static constexpr char const kGrammarFileProductionsStart[] = "PRODUCTIONS";
static constexpr char const kGrammarFileTerminalsStart[] = "TERMINALS";
static constexpr char const kGrammarFileNonTerminalsStart[] = "NONTERMINALS";
static constexpr char const kGrammarFileProductionLRSeparator[] = ":";
static constexpr char const kGrammarFileEmptyTerminal[] = "%empty";

/**
 * The function guarantees that the returned vectors of terminals_ and
 * non_terminals_ do not contain any duplicates
 */
void ParseGrammarFile(const std::string& grammar_filename,
		      ProductionElementVector* const terminals,
		      ProductionElementVector* const non_terminals,
		      ProductionVector* const productions,
		      ProductionElement* const start_symbol);
