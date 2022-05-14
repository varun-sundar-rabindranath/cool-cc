#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <parser/grammar_file_parser.hpp>

#include <parser/production.hpp>

// Parser base class
class Parser {

  private:
    static const std::size_t kParserErrorEntry = -1;

  public:
    static const ProductionElement kEmptyTerminal;

    using ProductionElementFirstSet =
        std::unordered_map<ProductionElement, ProductionElementSet, production_element_hash>;
    using ProductionElementFollowSet = ProductionElementFirstSet;

    using ProductionElementIDMap =
        std::unordered_map<ProductionElement, std::size_t, production_element_hash>;
    using ProductionIDMap =
        std::unordered_map<Production, std::size_t, production_hash>;

    // Recursive Descent Parsing Table
    // RD parsing table is one where the rows are for non-terminals,
    // the columns are for terminals and the entries are for the productions
    using RDParsingTable = std::vector<std::vector<std::size_t>>;

  public:
    Parser(const std::string& grammar_filename);

    // setters
    void SetTerminals(const ProductionElementVector& terminals);
    void SetNonTerminals(const ProductionElementVector& terminals);
    void SetProductions(const ProductionVector& productions);

    // getters
    ProductionElementVector GetTerminals() const;
    ProductionElementVector GetNonTerminals() const;
    ProductionVector GetProductions() const;
    ProductionElementFirstSet GetFirsts() const;
    ProductionElementFollowSet GetFollows() const;

    // debug functions
    void Dump() const;

  private:
    // Actual State
    ProductionElementVector terminals_;
    ProductionElementVector non_terminals_;
    ProductionVector productions_;
    ProductionElement start_symbol_;

    std::string grammar_filename_;

    // Derived State
    /* ID mapping is only required for easy manipulation in algorithms
     * and datastructures.
     * The ID always starts from zero.
     */
    ProductionElementIDMap terminal_id_map_;
    ProductionElementIDMap non_terminal_id_map_;
    ProductionIDMap production_id_map_;

    // Derived State
    /* Hashmap with a Production Element as Key (can be both terminal and non-
     * -terminal) and a set of Terminals as value
     * first_ : For every non-terminal, all the terminals that the Non-Terminal
     * can start off with is in first_
     * Example:
     *  in E -> abc
     *  first_[E] is {a}
     */
    ProductionElementFirstSet first_;
    /* Hashmap with a Production Element as Key (can be both terminal and non-
     * -terminal) and a set of Terminals as value
     * follow_ : For every non-terminal, all the terminals that can follow the
     * Non-Terminal is in follow_
     * in E -> Tab
     * follow_[T] is {a}
     */
    ProductionElementFollowSet follow_;

    /* Recursive Descent Parsing Table */
    RDParsingTable rd_parsing_table_;

  private:
    void ComputeFirst(); // Fills the derived state
    void ComputeFollow(); // Fills the derived state
    void ComputeParsingTable(); // Fills the derived state

    // return true if this production element can be empty ; false otherwise
    bool ComputeFirst(const ProductionElement& pe); // Fills the derived state
    void ComputeFollowPass();

};
