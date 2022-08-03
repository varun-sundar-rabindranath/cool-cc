#ifndef __DFA_HPP__
#define __DFA_HPP__
// Declare a Deterministic Finite Automaton class

#include <string>
#include <vector>
#include <lexer/regex_tree_nodes.hpp>
#include <unordered_map>
#include <memory>

class DFA {
public:
  DFA(const std::string& regex);
  ~DFA();

  // Reset DFA to start state
  void Reset();

  // Update DFA state and return the state of the DFA after transition
  int MoveOnSymbol(const char symbol);

  // Is the DFA in an accepting state ?
  bool InAcceptingState() const;

  // Is the DFA in an error state - Happens where there is no transition on
  // some symbol from some state
  bool InErrorState() const;

  // Test the input string with the DFA.
  // Return true if the string ends in an accepting state
  // Return false otherwise
  bool Test(const std::string& test_str);

private:
  std::string regex_;
  std::string augmented_regex_;
  /* nodepos_symbols_ records the  symbols that belong to a node position
   * Note that node-position is a leaf node in the regex tree
   */
  std::unordered_map<int, std::set<char>> nodepos_symbols_{};

  // NFATransitionMap - Key is a state, Value is a Map
  // ValueMap's key is a symbol, Value is a set of NFA states
  using NFATransitionMap = std::unordered_map<int, std::unordered_map<char, std::set<int>>>;
  NFATransitionMap nfa_{};

  // DFATransitionMap - Key is a state, Value is a Map
  // ValueMap's key is a symbol, Value is a DFA state
  using DFATransitionMap = std::unordered_map<int, std::unordered_map<char, int>>;
  DFATransitionMap dfa_{};
  std::set<int> dfa_accepting_states_{};
  int dfa_start_state_{-1};
  int current_dfa_state_{-1};

  std::shared_ptr<Node> regex_tree_{nullptr};

  std::shared_ptr<Node> MakeRegexTree(const std::string& regex);

#if 0
  void DeleteRegexTree(Node* tree);
#endif

  // Number leaf nodes from left to right
  void MarkLeafNodesLeftToRight(const std::shared_ptr<Node> tree);

  // Get leaf node symbols based on left-to-right leaf annotations
  void ConstructNodeposSymbols(const std::shared_ptr<Node> tree);

  // Construct NFA transitions from regex tree
  void RegexTreeToNFA(const std::shared_ptr<Node> tree);

  void SubsetConstruction();

  /** Utilities **/

  // Draw regex tree - utility
  void DrawRegexTree(const std::shared_ptr<Node> tree);

  // Inorder Traversal - utility
  void InorderTraversal(const std::shared_ptr<Node> tree);

  void PrintNFATransitions();

  void PrintDFATransitions();

};

#endif // __DFA_HPP__
