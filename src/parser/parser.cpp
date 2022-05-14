#include "spdlog/spdlog.h"
#include <stdexcept>
#include <cassert>
#include <algorithm>
#include <parser/parser.hpp>
#include <parser/grammar_file_parser.hpp>
#include <stack>
#include <iostream>

const ProductionElement Parser::kEmptyTerminal =
                        ProductionElement{ProductionElementType::TERMINAL,
                                          kGrammarFileEmptyTerminal};

Parser::Parser(const std::string& grammar_filename) :
  grammar_filename_{grammar_filename} {

  spdlog::debug("Parser({})", grammar_filename_);

  ParseGrammarFile(grammar_filename_, &terminals_,
                   &non_terminals_, &productions_,
                   &start_symbol_);
  // Update terminal -> id, non_terminal -> id, produciton -> id maps
  for (std::size_t id = 0; id < terminals_.size(); ++id) {
    assert (terminal_id_map_.find(terminals_.at(id)) == terminal_id_map_.end());
    terminal_id_map_.insert({terminals_.at(id), id});
  }
  for (std::size_t id = 0; id < non_terminals_.size(); ++id) {
    assert (non_terminal_id_map_.find(non_terminals_.at(id)) == non_terminal_id_map_.end());
    non_terminal_id_map_.insert({non_terminals_.at(id), id});
  }
  for (std::size_t id = 0; id < productions_.size(); ++id) {
    assert (production_id_map_.find(productions_.at(id)) == production_id_map_.end());
    production_id_map_.insert({productions_.at(id), id});
  }

  // print the grammar
  spdlog::debug("Terminals ...");
  for (const auto& t : terminals_) {
    spdlog::debug("{}", t.to_string());
  }

  spdlog::debug("Non Terminals ...");
  for (const auto& nt : non_terminals_) {
    spdlog::debug("{}", nt.to_string());
  }

  spdlog::debug("Productions ...");
  for (const auto&p : productions_) {
    spdlog::debug("{}", p.to_string());
  }

  spdlog::debug("Start Symbol {} ", start_symbol_.to_string());

  ComputeFirst();

  // dump firsts
  spdlog::debug("Firsts ...");
  for (const auto& pe_terminals : first_) {
    spdlog::debug("Firsts of {} is ", pe_terminals.first.to_string());
    for (const auto& t : pe_terminals.second) {
      spdlog::debug(" - {}", t.to_string());
    }
  }

  ComputeFollow();

  // dump follow
  spdlog::debug("Follow ...");
  for (const auto& pe_terminals : follow_) {
    spdlog::debug("Follows of {} is ", pe_terminals.first.to_string());
    for (const auto& t : pe_terminals.second) {
      spdlog::debug(" - {}", t.to_string());
    }
  }
}

// setters
void Parser::SetTerminals(const ProductionElementVector& terminals) {
  terminals_ = terminals;
}

void Parser::SetNonTerminals(const ProductionElementVector& non_terminals) {
  non_terminals_ = non_terminals;
}

void Parser::SetProductions(const ProductionVector& productions) {
  productions_ = productions;
}

// getters
ProductionElementVector Parser::GetTerminals() const {
  return terminals_;
}

ProductionElementVector Parser::GetNonTerminals() const {
  return non_terminals_;
}

ProductionVector Parser::GetProductions() const {
  return productions_;
}

Parser::ProductionElementFirstSet Parser::GetFirsts() const {
  return first_;
}

Parser::ProductionElementFollowSet Parser::GetFollows() const {
  return follow_;
}

bool Parser::ComputeFirst(const ProductionElement& pe) {

  // Intialize with an empty set; So we dont have to check again and again
  if (first_.find(pe) == first_.end()) {
    first_.insert({pe, ProductionElementSet{}});
  }

  auto add_firsts_of_production_element = [&](const ProductionElement& x) {
    // Don't add the empty terminal - Maybe a terminal follows this NT
    for (const auto& y : first_.at(x)) {
      if (y == kEmptyTerminal) {
        continue;
      }
      first_.at(pe).insert(y);
    }
  };

  if (pe.type == ProductionElementType::TERMINAL) {
    first_.at(pe).insert(pe);
    return false;
  }

  bool pe_can_be_empty{false};

  for (const auto& p : productions_) {
    // TODO very inefficient - do something
    if (p.left != pe) { continue; }

    bool pi_empty{true}; // initialized to true to handle the empty production case
    for (std::size_t p_i = 0; p_i < p.right.size(); ++p_i) {

      if (p.right.at(p_i) == kEmptyTerminal) { continue; }

      pi_empty = ComputeFirst(p.right.at(p_i));

      // Add the computed first() to pe's first
      add_firsts_of_production_element(p.right.at(p_i));

      if (!pi_empty) { break; }
    }

    // pi was empty all the way - this means pe can also be empty
    pe_can_be_empty |= pi_empty;
  }

  if (pe_can_be_empty) {
    // if the pe can be empty - add the empty terminal to the set
    first_.at(pe).insert(kEmptyTerminal);
  }

  return pe_can_be_empty;
}

void Parser::ComputeFirst() {
  first_.clear();
  /*
   * For every non-terminal all the terminals that can kick off that non-terminal
   * is in that non-terminal's first set
   * Cases:
   *  1. E -> abc ; a is in first(E)
   *  2. E -> Tabc, T -> xy ; x is in first(E)
   *  3. E -> Tabc, T -> xy, T-> epsilon ; {a, x} is in first(E)
   */

  // Compute firsts of terminals
  for (const auto& t : terminals_) {
    ComputeFirst(t);
  }

  // Compute firsts of non-terminals
  for (const auto& nt : non_terminals_) {
    ComputeFirst(nt);
  }
}

void Parser::ComputeFollow() {
  follow_.clear();

  /*
   * For every non-terminal, the Follow set contains all the terminals that can
   * immediately follow that non-terminal
   * Cases:
   *  1. If Y follows X in some production and Y cannot be empty
   *      Follow(X) = First(Y)
   *  2. If Y1 Y2 ... Y(n-1) Yn follows X and Y1, Y2, ... Y(n - 1) can be empty
   *      Follow(X) = First(Y1) U First(Y2) U .... First(Yn)
   *      also can be expressed as Follow(X) = First(Y1) U Follow(Y1)
   *  3. If Y1 Y2 ... Yn follows X and Y1 Y2 ... Yn can be empty and L is the
   *     left side of the production and L != X
   *      Follow(X) = First(Y1) U First(Y2) U ... First(Yn) U Follow(L)
   *      also can be expressed as Follow(X) = First(Y1) U Follow(Y1)
   *  4. If nothing follows X and L is the left side of the production and
   *     L != X
   *      Follow(X) = Follow(L)
   *
   * Examples for Cases
   *  1. E -> a X b c; X -> x y;
   *     {b} is in Follow(X)
   *  2. E -> a X Y b c; X -> x y; Y -> p q; Y -> epsilon
   *     {p, b} is in Follow(X)
   *     {b} is in Follow(Y)
   *  3. E -> a X Y1 Y2 b c; X -> x y; Y1 -> p1 q1; Y2 -> p2 q2; Y1 -> epsilon;
   *     Y2 -> epsilon
   *     {p1, p2, b} is in Follow(X)
   *     {p2, b} is in Follow(Y1)
   *     {b} is in Follow(Y2)
   *  4. E -> a X; T ->  b E c; X -> x y
   *     {c} is in Follow(E)
   *     {c} is in Follow(X)
   */

  /*
   * What about recursive definitions ?
   *
   * E -> a X; X -> b E c; E -> m
   * {c} is in Follow(E)
   * {c} is in Follow(X)
   *
   * Think:
   *     E
   *    / \
   *   a   X
   *     / | \
   *    b  E  c
   *      / \
   *     a   X
   *  c should be in Follow(X)
   */

  // Intialize all Non-Terminals with empty FollowSet
  //
  for (const auto& nt : non_terminals_) { follow_.insert({nt, {}}); }

  int follow_items_prev = -1;
  int follow_items = 0;

  while (follow_items != follow_items_prev) {
    // Iterate follow set computations
    ComputeFollowPass();

    // Sum sizes of all the follow sets
    follow_items_prev = follow_items;

    follow_items = 0;
    for (const auto& pe_followset : follow_) {
      follow_items += pe_followset.second.size();
    }

    spdlog::debug("{}() : Follow Sets grew from {} -> {}",
                  __func__, follow_items_prev, follow_items);
  }
}

void Parser::ComputeFollowPass() {
  // for every non terminal in the production - keep adding the firsts
  // of the following elements.
  // if end is reached - add the follow of the left side of the terminal
  auto add_firsts_to_follow = [&](const ProductionElement& from_firsts_pe,
                                  const ProductionElement& to_follow_pe) {
    for (const auto& first_item: first_.at(from_firsts_pe)) {
      if (first_item == kEmptyTerminal) { continue; }
      follow_.at(to_follow_pe).insert(first_item);
    }
  };

  auto add_follow_to_follow = [&](const ProductionElement& from_follow_pe,
                                  const ProductionElement& to_follow_pe) {
    assert (from_follow_pe != to_follow_pe);
    if (from_follow_pe == to_follow_pe) {
      // Nothing to do
      return;
    }
    for (const auto& follow_item : follow_.at(from_follow_pe)) {
      follow_.at(to_follow_pe).insert(follow_item);
    }
  };

  for (const auto& p : productions_) {
    for (std::size_t p_i = 0; p_i < p.right.size(); ++p_i) {
      const auto& pi_e{p.right.at(p_i)};

      if (!pi_e.IsNonTerminal()) {
        continue;
      }

      std::size_t p_j = p_i + 1;
      while (p_j < p.right.size()) {
        // Add first of p_j to p_i
        const auto& pj_e{p.right.at(p_j)};
        add_firsts_to_follow(pj_e, pi_e);
        if (!first_.at(pj_e).count(kEmptyTerminal)) {
          // pj_e cannot be empty
          break;
        }
        p_j++;
      }

      if (p_j == p.right.size()) {
        // p_j fell of the end of the production - This means whatever follows
        // the left of the production follows p_i also
        add_follow_to_follow(p.left, pi_e);
      }
    } // production right side iterator
  } // productions iterator
}

void Parser::ComputeParsingTable() {

  auto add_firsts_of_production_element = [&](const ProductionElement& x,
                                              ProductionElementSet* const pe_set) {
    assert (pe_set);
    // Don't add the empty terminal - Maybe a terminal follows this NT
    for (const auto& y : first_.at(x)) {
      if (y == kEmptyTerminal) {
        continue;
      }
      pe_set->insert(y);
    }
  };

  // Initialize parsing tables with all error states
  rd_parsing_table_.clear();
  const std::size_t n_terminals{terminals_.size()};
  const std::size_t n_non_terminals{non_terminals_.size()};
  rd_parsing_table_ = std::vector<std::vector<std::size_t>>{
                        n_non_terminals, std::vector<std::size_t>{
                          n_terminals, kParserErrorEntry}};

  /**
   * How to fill the parsing table ?
   *  - Note that the entry is actually the production id
   *  - For each production,
   *   - Add the production to all the First(production)
   *     First(production) is calculated by the union of the right side of the
   *     production. keep doing the union until you find a right side element
   *     that cannot derive epsilon
   *   - If the production can derive epsilon add the production to all the
   *     Follow (production->left)
   */

  for (const auto& p : productions_) {
    /**
     * Need 2 pieces of information
     *  - The First set of the production
     *  - Can the production drive epsilon
     */

    // The First set of the production
    ProductionElementSet p_firsts;
    std::size_t r_i = 0;
    while (r_i < p.right.size()) {
      const auto& pe{p.right.at(r_i)};
      if (pe == kEmptyTerminal) { ++r_i; continue; }

      // Add firsts of pe to p_first
      add_firsts_of_production_element(pe, &p_firsts);

      if (first_[pe].find(kEmptyTerminal) == first_[pe].end()) {
        // pe cannot derive epsilon
        break;
      }

      ++r_i;
    }
    // Can the production derive epsilon
    const bool p_can_be_empty{r_i == p.right.size() + 1};

    const auto p_id{production_id_map_[p]};
    const auto nt_id{non_terminal_id_map_[p.left]};

    // Add the production_id to all rd_parsing_table[p.left][first_set_element]
    for (const auto& p_first : p_firsts) {
      assert (p_first.IsTerminal());
      const auto t_id{terminal_id_map_[p_first]};
      rd_parsing_table_[nt_id][t_id] = p_id;
    }

    // Add the production_id to all Follow[p.left] if p_can_be_empty
    if (p_can_be_empty) {
      for (const auto& nt_follow : follow_[p.left]) {
        assert (nt_follow.IsTerminal());
        const auto t_id{terminal_id_map_[nt_follow]};
        rd_parsing_table_[nt_id][t_id] = p_id;
      }
    }
  } // productions_
}
