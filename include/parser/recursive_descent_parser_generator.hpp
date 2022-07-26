#pragma once

/**
 * Subclass of the general ParserGenerator class.
 */

#include <parser/parser_generator.hpp>
#include <string>

class RecursiveDescentParserGenerator : public ParserGenerator {

  public:
    using ProductionElementFirstSet =
        std::unordered_map<ProductionElement, ProductionElementSet, production_element_hash>;
    using ProductionElementFollowSet = ProductionElementFirstSet;

    /**
     * Recursive Descent Parsing Table
     * RD parsing table is one where the rows are for non-terminals,
     * the columns are for terminals and the entries are for the productions
     * Note that if there are multiple entries for a {non-terminal, terminal}
     * pair, then it means that the grammar is ambiguous
     */
    using RDParsingTableEntry = std::vector<std::size_t>;
    using RDParsingTable = std::vector<std::vector<RDParsingTableEntry>>;

  public:
    RecursiveDescentParserGenerator(const std::string& grammar_filename);

    void WriteParsingTable(const std::string& filename) const;

    // getters
    ProductionElementFirstSet GetFirsts() const;
    ProductionElementFollowSet GetFollows() const;
    std::vector<Production> GetParsingTableProductions(
      const ProductionElement& nt, const ProductionElement& t) const;

    void Dump() const;
    void DumpFirst() const;
    void DumpFollow() const;
    void DumpParsingTable() const;

  private:

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
