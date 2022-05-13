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

void ParseGrammarFile(const std::string& grammar_filename,
		      ProductionElementSet* const terminals,
		      ProductionElementSet* const non_terminals,
		      ProductionElement_Production_Map* const productions,
		      ProductionElement* const start_symbol);
