#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <parser/grammar_file_parser.hpp>

#include <parser/production.hpp>

// Parser base class
class Parser {

  public:
    static const ProductionElement kEmptyTerminal;
    using ProductionElementFirstSet =
        std::unordered_map<ProductionElement, ProductionElementSet, production_element_hash>;
    using ProductionElementFollowSet = ProductionElementFirstSet;

    using ProductionElementIDMap =
        std::unordered_map<ProductionElement, std::size_t, production_element_hash>;
    using ProductionIDMap =
        std::unordered_map<Production, std::size_t, production_hash>;

    using IDProductionElementMap =
        std::unordered_map<std::size_t, ProductionElement>;
    using IDProductionMap = std::unordered_map<std::size_t, Production>;

    using ProductionElementVector = std::vector<ProductionElement>;
    using TerminalVector = ProductionElementVector;
    using NonTerminalVector = ProductionElementVector;
    using ProductionVector = std::vector<Production>;

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

  private:
    // Actual State
    ProductionElementVector terminals_;
    ProductionElementVector non_terminals_;
    ProductionVector productions_;
    ProductionElement start_symbol_;

    std::string grammar_filename_;

    // Actual State
    /* ID mapping is only required for easy manipulation in algorithms
     * and datastructures.
     * The ID always starts from zero.
     */
    ProductionElementIDMap terminal_id_map_;
    ProductionElementIDMap non_terminal_id_map_;
    ProductionIDMap production_id_map_;
    /* ID to ProductionElement / Production reverse mapping */
    IDProductionElementMap id_terminal_map_;
    IDProductionElementMap id_non_terminal_map_;
    IDProductionMap id_production_map_;

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

  private:

    // ID mapping utils
    std::size_t ProductionElementID(const ProductionElement& pe) const;
    std::size_t TerminalID(const ProductionElement& pe) const;
    std::size_t NonTerminalID(const ProductionElement& pe) const;
    std::size_t ProductionID(const Production& pe) const;
    ProductionElement TerminalID(const std::size_t id) const;
    ProductionElement NonTerminalID(const std::size_t id) const;
    Production ProductionID(const std::size_t id) const;

    void ComputeFirst(); // Fills the derived state
    void ComputeFollow(); // Fills the derived state

    // return true if this production element can be empty ; false otherwise
    bool ComputeFirst(const ProductionElement& pe); // Fills the derived state
    void ComputeFollowPass();
};
