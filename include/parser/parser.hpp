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

  public:
    Parser(const std::string& grammar_filename);

    // setters
    void SetTerminals(const ProductionElementSet& terminals);
    void SetNonTerminals(const ProductionElementSet& terminals);
    void SetProductions(const ProductionElement_Production_Map& terminals);

    // getters
    ProductionElementSet GetTerminals() const;
    ProductionElementSet GetNonTerminals() const;
    ProductionElement_Production_Map GetProductions() const;
    ProductionElementFirstSet GetFirsts() const;
    ProductionElementFollowSet GetFollows() const;

  private:
    // Actual State
    ProductionElementSet terminals_;
    ProductionElementSet non_terminals_;
    ProductionElement_Production_Map productions_;
    ProductionElement start_symbol_;
    std::string grammar_filename_;

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
    void ComputeFirst(); // Fills the derived state
    void ComputeFollow(); // Fills the derived state

    // return true if this production element can be empty ; false otherwise
    bool ComputeFirst(const ProductionElement& pe); // Fills the derived state
    void ComputeFollowPass();
};
