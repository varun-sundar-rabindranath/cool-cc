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
  for (const auto& pe_ps : productions_) {
    for (const auto& p : pe_ps.second) {
      spdlog::debug("{}", p.to_string());
    }
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
void Parser::SetTerminals(const ProductionElementSet& terminals) {
  terminals_ = terminals;
}

void Parser::SetNonTerminals(const ProductionElementSet& non_terminals) {
  non_terminals_ = non_terminals;
}

void Parser::SetProductions(const ProductionElement_Production_Map& productions) {
  productions_ = productions;
}

// getters
ProductionElementSet Parser::GetTerminals() const {
  return terminals_;
}

ProductionElementSet Parser::GetNonTerminals() const {
  return non_terminals_;
}

ProductionElement_Production_Map Parser::GetProductions() const {
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

  for (const auto& p : productions_[pe]) {
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
    ComputeFollowPassDFS(); // basically a DFS that updates the follow-set of NT

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

void Parser::ComputeFollowPassDFS() {
  // A DFS pass to update the follow set

  ProductionElementSet visited;

  for (const auto& nt : non_terminals_) {
    if (visited.find(nt) == visited.end()) {
      ComputeFollowPassDFS(nt, &visited);
    }
  }
}

void Parser::ComputeFollowPassDFS(const ProductionElement& nt,
                                  ProductionElementSet* const visited) {

  std::stack<ProductionElement> s;

  auto add_to_stack = [&](const ProductionElement& nt_pe) {
    // if nt_pe is a terminal - return
    if (nt_pe.type == ProductionElementType::TERMINAL) { return; }
    // if already visited - return
    if (visited->find(nt_pe) != visited->end()) { return; }

    // All checks passed - Add to stack and mark visited
    s.push(nt_pe);
    visited->insert(nt_pe);
  };

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

  // Intialize with start_symbol_
  add_to_stack(nt);

  while (!s.empty()) {
    auto pe{s.top()}; s.pop();

    // visit all the productions that the pe is a part of to add children
    for (const auto& nt_productions : productions_) {
      const auto& productions{nt_productions.second};
      for (const auto& p : productions) {
        for (std::size_t p_i = 0; p_i < p.right.size(); ++p_i) {
          if (p.right.at(p_i) != pe) { continue; }
          /* If the current pe matches our node,
           *  Add the existing follow set & first set and,
           *  Add to dfs if not already visited
           */
          const bool is_last_element{p_i == p.right.size() - 1};
          if (is_last_element) {
            // Whatever follows the left side of the production also follows this PE
            spdlog::debug("Adding follows of {} to {}", p.left.to_string(), pe.to_string());
            add_follow_to_follow(p.left, pe);
            add_to_stack(p.left);
          } else {
            const auto& next_pe{p.right.at(p_i + 1)};
            // The firsts of the p_i + 1 element follows p_i element
            spdlog::debug("Adding firsts of {} to {}", next_pe.to_string(), pe.to_string());
            add_firsts_to_follow(next_pe, pe);
            // If the next element can derive epsilon then whatever follows that follows this PE
            if (first_.at(next_pe).find(kEmptyTerminal) != first_.at(next_pe).end()) {
              add_follow_to_follow(next_pe, pe);
              add_to_stack(next_pe);
            }
          }
        } // production right side
      } // productions of a specific NT
    } // productions
  } // dfs
}
