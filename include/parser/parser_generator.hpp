#pragma once

/**
 * Base class for all Parser Generators
 * The main job of the ParserGenerator is to setup all the information, such as,
 * - The list of terminals
 * - The list of non-terminals
 * - The list of productions
 * - The parsing table etc.
 * The parser can then use this information to parse input strings
 */

#include <unordered_map>
#include <parser/production.hpp>
#include <fstream>

class ParserGenerator {

  public:
    static const ProductionElement kEndOfInputTerminal; // this is the $ in dragon boo
    static const ProductionElement kEmptyTerminal;

  public:

    ParserGenerator(const std::string& grammar_filename);

    // setters
    void SetTerminals(const ProductionElementVector& terminals);
    void SetNonTerminals(const ProductionElementVector& terminals);
    void SetProductions(const ProductionVector& productions);

    // getters
    ProductionElementVector GetTerminals() const;
    ProductionElementVector GetNonTerminals() const;
    ProductionVector GetProductions() const;

    void Dump() const;

    void WriteSemanticRules(const std::string& filename) const;
    void WriteGrammerObjects(std::fstream& file_stream) const;
    virtual void WriteParsingTable(const std::string& filename) const = 0;

  protected:
    // Actual State
    std::string grammar_filename_;
    ProductionElementVector terminals_;
    ProductionElementVector non_terminals_;
    ProductionVector productions_;
    std::vector<std::string> productions_semantic_rules_;
    ProductionElement start_symbol_;

    std::vector<std::string> productions_semantic_rules_includes_;

    // Derived State
    /* ID mapping is only required for easy manipulation in algorithms
     * and datastructures.
     * The ID always starts from zero.
     */
    ProductionElementIDMap terminal_id_map_;
    ProductionElementIDMap non_terminal_id_map_;
    ProductionIDMap production_id_map_;

    // Writer utilities

    // Braced-Initialized writer utilities
    std::string BraceInitializedProductionElementString(
      const ProductionElement& pe) const;
    std::string BraceInitializedProductionString(const Production& p) const;

    // Definition writer utilities
    std::string ProductionElementDefinitionString(
      const ProductionElement& pe, const std::string& var_name) const;
    std::string ProductionElementVectorDefinitionString(
      const ProductionElementVector& pes, const std::string& var_name) const;
    std::string ProductionElementIDMapDefinitionString(
     const ProductionElementIDMap& pe_id_map, const std::string& var_name) const;
    std::string ProductionVectorDefinitionString(
      const ProductionVector& productions, const std::string& var_name) const;
    std::string ProductionIDMapDefinitionString(
      const ProductionIDMap& production_id_map,
      const std::string& var_name) const;
};
