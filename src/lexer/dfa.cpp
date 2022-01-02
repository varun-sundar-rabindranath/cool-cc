// Define a Deterministic Finite Automaton
#include <algorithm>
#include <iostream>
#include "lexer/dfa.hpp"
#include "spdlog/spdlog.h"
#include "lexer/regex_tree_nodes.hpp"
#include "lexer/lex_character_classes.hpp"
#include <iterator>
#include <memory>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <queue>

DFA::DFA(const std::string& regex) :
  regex_{regex},
  augmented_regex_{regex+"#"},
  nodepos_symbols_{},
  nfa_{},
  dfa_{},
  dfa_accepting_states_{},
  regex_tree_{nullptr} {

  // Make regex tree
  spdlog::debug("Making Regex Tree for {} ...", augmented_regex_);
  regex_tree_ = MakeRegexTree(augmented_regex_);


  // Annotate leaf nodes sequentially from left to right
  spdlog::debug("Annotating leaf nodes ...");
  MarkLeafNodesLeftToRight(regex_tree_);

  // Get lead node position and symbols
  spdlog::debug("Constructing leaf-node positions and symbols ...");
  ConstructNodeposSymbols(regex_tree_);

  // Ascertain which nodes are nullable
  spdlog::debug("Computing nullable ...");
  regex_tree_->ComputeIsNullable();

  spdlog::debug("Computing first pos ...");
  regex_tree_->ComputeFirstPos();

  spdlog::debug("Computing last pos ...");
  regex_tree_->ComputeLastPos();

  //spdlog::info("Drawing regex tree...");
  //DrawRegexTree(regex_tree_);
  //spdlog::info("Inorder traversal ...");
  //InorderTraversal(regex_tree_);

  spdlog::debug("Regex Tree -> NFA ...");
  RegexTreeToNFA(regex_tree_);

  //spdlog::debug("Printing NFA transitions ...");
  //PrintNFATransitions();

  spdlog::debug("Subset construction ...");
  SubsetConstruction();

  //spdlog::debug("Printing DFA transitions ...");
  //PrintDFATransitions();
}

DFA::~DFA() {
  // Delete regex tree
  DeleteRegexTree(regex_tree_);
  regex_tree_ = nullptr;

  // clear all maps
  nodepos_symbols_.clear();
  nfa_.clear();
  dfa_.clear();
  dfa_accepting_states_.clear();
}

void DFA::Reset() {
  current_dfa_state_ = dfa_start_state_;
}

int DFA::MoveOnSymbol(const char symbol) {
  // Is there a transition from the current dfa state on the symbol
  if (dfa_.find(current_dfa_state_) == dfa_.end() ||
      dfa_.at(current_dfa_state_).find(symbol) == dfa_.at(current_dfa_state_).end()){
    current_dfa_state_ = -1;
  } else {
    current_dfa_state_ = dfa_.at(current_dfa_state_).at(symbol);
  }
  return current_dfa_state_;
}

bool DFA::InAcceptingState() const {
  return dfa_accepting_states_.find(current_dfa_state_) != dfa_accepting_states_.end();
}

bool DFA::InErrorState() const {
  return current_dfa_state_ == -1;
}

bool DFA::Test(const std::string& test_str) {
  Reset();
  for (const auto x : test_str) {
    spdlog::debug(fmt::format("move on symbol {}", x));
    MoveOnSymbol(x);
  }
  const bool accept{InAcceptingState()};
  Reset();

  return accept;
}

void DFA::SubsetConstruction() {

  dfa_.clear();
  dfa_accepting_states_.clear();
  dfa_start_state_ = -1;

  // Need to check if an nfa set is already populated !! // can use a map for that
  // Need to mark NFA states as computed not computed - can use a map for that
  auto set_to_string = [](const std::set<int>& s) {
    std::string str;
    for (const auto x : s) {
      str += fmt::format("{}-", x);
    }
    return str;
  };

  struct DFATransitions {
    std::set<int> from_nfa_states;
    std::set<int> to_nfa_states;
    std::set<char> transition_symbols;
  };
  std::vector<DFATransitions> dfa_transitions;

  // all the alphabet
  const std::set<char> alphabet{LexCharacterClasses::GetAllSupportedSymbols()};

  // Subset construction
  const auto seed_nfa_states_unordered{regex_tree_->GetFirstPos()};
  std::set<int> seed_nfa_states;
  std::transform(seed_nfa_states_unordered.begin(), seed_nfa_states_unordered.end(),
                 std::inserter(seed_nfa_states, seed_nfa_states.begin()), [](int x){return x;});
  spdlog::debug(fmt::format("seed nfa states {}", set_to_string(seed_nfa_states)));
  std::unordered_set<std::string> state_visited;
  std::queue<std::set<int> > q;
  q.push(seed_nfa_states);

  while (!q.empty()) {

    const auto nfa_states{q.front()};
    q.pop();

    if (state_visited.find(set_to_string(nfa_states)) != state_visited.end()) {
      // already visited
      continue;
    }

    // for each symbol
    for (const auto symbol : alphabet) {
      std::set<int> all_transition_states;
      // where does the current set of nfa states take me ?
      for (const auto nfa_state : nfa_states) {
        if ((nfa_.find(nfa_state) == nfa_.end()) ||
            (nfa_.at(nfa_state).find(symbol) == nfa_.at(nfa_state).end())) {
          // Such an nfa state does not have any transitions or
          // The there is no transition for the current symbol
          continue;
        }
        const auto& transition_states{nfa_.at(nfa_state).at(symbol)};
        for (const auto sdash : transition_states) {
          all_transition_states.insert(sdash);
        }
      }

      if (!all_transition_states.empty()) {
        // update dfa_transitions and populate queue
        q.push(all_transition_states);
        dfa_transitions.push_back({nfa_states, all_transition_states, {symbol}});
      }
    }

    // Mark visited
    state_visited.insert(set_to_string(nfa_states));
  }

  // Print dfa transitions
  for (const auto& dfa_transition : dfa_transitions) {
    std::string transition_symbols;
    for (const auto c : dfa_transition.transition_symbols) {
      transition_symbols += fmt::format(" {}", c);
    }
    spdlog::debug("{} on {} goes to {}",
                 set_to_string(dfa_transition.from_nfa_states),
                 transition_symbols,
                 set_to_string(dfa_transition.to_nfa_states));
  }

  // Post-Processing - Assign DFA state indices
  // What is the start node ? - This is the first from node in dfa transitions
  // What is the accepting node ? - Any node that has the nfa accepting state is a dfa accepting staet
  assert (regex_tree_->GetRightSubTree()->GetNodeType() == NodeType::NODE_TYPE_LEAF);
  const int nfa_accepting_state{
      dynamic_cast<const LeafNode*>(regex_tree_->GetRightSubTree())->GetNodePosition()};
  spdlog::debug(fmt::format("nfa accepting state {}", nfa_accepting_state));
  std::unordered_map<std::string, int> nfa_states_to_dfa_state_map;
  int dfa_state_idx = 0;

  // Add seed_nfa_states as the start state
  nfa_states_to_dfa_state_map.insert(
    {set_to_string(seed_nfa_states), dfa_state_idx++});

  for (const auto& dfa_transition : dfa_transitions) {
    const auto& from_nfa_states{dfa_transition.from_nfa_states};
    const auto& to_nfa_states{dfa_transition.to_nfa_states};
    const auto& transition_symbols{dfa_transition.transition_symbols};

    const auto from_nfa_states_str{set_to_string(from_nfa_states)};
    const auto to_nfa_states_str{set_to_string(to_nfa_states)};

    if (nfa_states_to_dfa_state_map.find(from_nfa_states_str) ==
        nfa_states_to_dfa_state_map.end()) {
      nfa_states_to_dfa_state_map.insert({from_nfa_states_str, dfa_state_idx++});
    }

    if (nfa_states_to_dfa_state_map.find(to_nfa_states_str) ==
        nfa_states_to_dfa_state_map.end()) {
      nfa_states_to_dfa_state_map.insert({to_nfa_states_str, dfa_state_idx++});
    }

    const int dfa_from_state{nfa_states_to_dfa_state_map.at(from_nfa_states_str)};
    const int dfa_to_state{nfa_states_to_dfa_state_map.at(to_nfa_states_str)};
    const bool is_dfa_accepting_state{
      to_nfa_states.find(nfa_accepting_state) != to_nfa_states.end()};

    if (dfa_.find(dfa_from_state) == dfa_.end()) {
      dfa_.insert({dfa_from_state, {}});
    }
    auto& dfa_transition_map{dfa_.at(dfa_from_state)};
    for (const auto& symbol : transition_symbols) {
      // Update DFA transition table
      assert (dfa_transition_map.find(symbol) == dfa_transition_map.end());
      dfa_transition_map.insert({symbol, dfa_to_state});
    }
    // Update dfa accepting state
    if (is_dfa_accepting_state) { dfa_accepting_states_.insert(dfa_to_state);}
  }

  // Update dfa start state
  dfa_start_state_ =
    nfa_states_to_dfa_state_map.at(set_to_string(seed_nfa_states));
}

Node* DFA::MakeRegexTree(const std::string& regex) {

  /* A star node always corresponds to the symbol before the star.
   * Some examples,
   * ab* - star is for the symbol b
   * a[a-z]* - star is for the character class [a-z]
   * a(b|c)* - star is for the previous block (b|c)
   * (a|b)*abc - star is for the previous block (a|b)
   * So we have to treat
   * ab* -> a(b*)
   * a[a-z]* -> a([a-z]*)
   * a(b|c)* as a((b|c)*)
   * (a|b)*abc -> ((a|b)*)abc
   * so it is important to parenthesize the input string ? - Yes, but lets
   * do it just for the * stuff!! i.e. if you encounter a "*" and not "\*"
   * put parens around the guy !!
   * TODO : Fix input regex to reflect this for now.. (for now in comments always
   * means forever)
   */

  auto is_open_paren = [](char c) -> bool {
    if (c == '(') { return true; }
    return false;
  };

  auto is_close_paren = [](char c) -> bool {
    if (c == ')') { return true; }
    return false;
  };

  auto is_open_box_paren = [](char c) -> bool {
    if (c == '[') { return true; }
    return false;
  };

  auto is_close_box_paren = [](char c) -> bool {
    if (c == ']') { return true; }
    return false;
  };

  auto is_symbol = [](char c) -> bool {
    if (c == '(') { return false; }
    if (c == ')') { return false; }
    if (c == '*') { return false; }
    if (c == '|') { return false; }
    if (c == ']') { return false; }
    if (c == '[') { return false; }
    return true;
  };

  auto is_bslash = [] (char c) -> bool {
    if (c == '\\') { return true; }
    return false;
  };

  auto is_star = [](char c) -> bool {
    if (c == '*') { return true; }
    return false;
  };

  auto is_or = [](char c) -> bool {
    if (c == '|') { return true; }
    return false;
  };

  auto get_matching_close_paren = [&](const std::string& regex, int pos,
                                      bool (*is_open_char)(char),
                                      bool (*is_close_char)(char)) {
    assert (is_open_char(regex.at(pos)));
    int match = 1;
    pos++;

    while (pos < int(regex.size()) && match) {
      if (is_open_char(regex.at(pos))) {
	match++;
      }
      if (is_close_char(regex.at(pos))) {
	match--;
      }
      pos++;
    }

    if (match == 0) {
      return pos - 1;
    }
    // Should never be here if we have matching parens
    throw std::invalid_argument("Cannot find matching parens in the regex");
  };

  // In the given regex! if you encounter a '(', find the matching ')' - Create
  // a substr and make a regex tree before you proceed

  // In the given regex if you encounter a '/' use the next character

  auto process_symbol = [&](const std::string& s, Node* const tree) -> Node* {
    // Make a leaf node
    if (tree) {
      return new CatNode(tree, new LeafNode(s));
    } else {
      return new LeafNode(s);
    }
  };

  Node* tree = nullptr;
  std::size_t sptr = 0;
  while (sptr < regex.size()) {

    // Check if this is an escape sequence
    if (is_bslash(regex.at(sptr))) {

      // This is definitely a symbol
      const std::string char_str{std::string{regex.at(sptr)} + std::string{regex.at(sptr + 1)}};
      tree = process_symbol(char_str, tree);

      // processed 2 characters !
      sptr++;
      sptr++;
      continue;
    }

    const char c{regex.at(sptr)};

    if (is_open_paren(c)) {
      const int matching_close_paren =
        get_matching_close_paren(regex, sptr, is_open_paren, is_close_paren);
      const int paren_str_len = matching_close_paren - sptr + 1;
      if (tree) {
	// Make a cat node
	tree = new CatNode(tree, MakeRegexTree(regex.substr(sptr + 1, paren_str_len - 2)));
      } else {
	tree = MakeRegexTree(regex.substr(sptr + 1, paren_str_len - 2));
      }
      // Update sptr
      sptr = matching_close_paren;
    }

    if (is_open_box_paren(c)) {
      const int matching_close_paren =
        get_matching_close_paren(regex, sptr, is_open_box_paren, is_close_box_paren);
      const int paren_str_len = matching_close_paren - sptr + 1;
      tree = process_symbol(regex.substr(sptr, paren_str_len), tree);
      sptr = matching_close_paren;
    }

    if (is_symbol(c)) {
      // Make a leaf node
      if (tree) {
	tree = new CatNode(tree, new LeafNode(std::string{c}));
      } else {
	tree = new LeafNode(std::string{c});
      }
    }

    if (is_star(c)) {
      // Make a * node; There must be an existing tree
      assert (tree);
      tree = new StarNode(tree);
    }

    if (is_or(c)) {
      // Make an OR node; there must be an existing tree
      assert (tree);
      tree = new ORNode(
	tree, MakeRegexTree(regex.substr(sptr + 1, regex.size() - sptr + 1)));
      // The rest of the regex is taken care of
      break;
    }

    sptr++;
  }

  return tree;
}

void DFA::DeleteRegexTree(Node* const tree) {

  if (tree) {
    DeleteRegexTree(tree->GetLeftSubTree());
    DeleteRegexTree(tree->GetRightSubTree());
    const auto node_type{tree->GetNodeType()};
    if (node_type == NODE_TYPE_CAT) {
      delete dynamic_cast<CatNode*>(tree);
    }
    if (node_type == NODE_TYPE_OR) {
      delete dynamic_cast<ORNode*>(tree);
    }
    if (node_type == NODE_TYPE_STAR) {
      delete dynamic_cast<StarNode*>(tree);
    }
    if (node_type == NODE_TYPE_LEAF) {
      delete dynamic_cast<LeafNode*>(tree);
    }
  }
}

void DFA::MarkLeafNodesLeftToRight(Node* const root) {
  assert (root);

  int left_to_right_idx = 1;

  // Do a DFS - This by default would access leaf nodes from left to right
  std::stack<Node*> nodes;
  nodes.push(root);

  while(!nodes.empty()) {
    auto node = nodes.top();
    nodes.pop();

    if (node->GetNodeType() == NodeType::NODE_TYPE_LEAF) {
      dynamic_cast<LeafNode*>(node)->SetNodePosition(left_to_right_idx++);
    }

    // Put children on to the stack
    auto right_sub_tree = node->GetRightSubTree();
    auto left_sub_tree = node->GetLeftSubTree();
    if (right_sub_tree) { nodes.push(right_sub_tree); }
    if (left_sub_tree) { nodes.push(left_sub_tree); }
  }
}

void DFA::ConstructNodeposSymbols(const Node* const tree) {
  assert(tree);

  nodepos_symbols_.clear();
  // Need to access all leaf nodes - Do a DFS
  std::stack<const Node*> nodes;
  nodes.push(tree);

  while (!nodes.empty()) {
    auto node = nodes.top();
    nodes.pop();

    if (node->GetNodeType() == NodeType::NODE_TYPE_LEAF) {
      nodepos_symbols_.insert({dynamic_cast<const LeafNode*>(node)->GetNodePosition(),
                               dynamic_cast<const LeafNode*>(node)->GetSymbols()});
    }

    // Put children on to the stack
    auto right_sub_tree = node->GetRightSubTree();
    auto left_sub_tree = node->GetLeftSubTree();
    if (right_sub_tree) { nodes.push(right_sub_tree); }
    if (left_sub_tree) { nodes.push(left_sub_tree); }
  }
}

void DFA::RegexTreeToNFA(const Node* const tree) {
  assert (tree);
  // Clear existing NFA if any
  nfa_.clear();

  // Theory:
  // Constructing an NFA from a regex-tree is based on the idea of computing
  // follow_pos(i) - follow_pos(i) calculates all the node-positions that follow
  // the node-position i
  // For CAT-NODE:
  //  The follow_pos(i) = {j, k, l ...} - where,
  //    i is some position in the last_pos_ of left-child of the CAT node
  //    "j, k, l ..." is some position in the first_pos_ of the right-child of the CAT node
  // For STAR-NODE n:
  //  The follow_pos(i) = first_pos(n), where i is some position in the
  //  last_position of n

  // key is the node-position and value is the follow-positions of the node positons
  std::unordered_map<int, std::set<int> > position_followpos;

  // TODO - Really need a dfs utility
  // Need to access all leaf nodes - Do a DFS
  std::stack<const Node*> nodes;
  nodes.push(tree);

  while (!nodes.empty()) {
    auto node = nodes.top();
    nodes.pop();

    // CAT node case
    if (node->GetNodeType() == NodeType::NODE_TYPE_CAT) {
      const auto left = node->GetLeftSubTree();
      const auto right = node->GetRightSubTree();
      // y follows x
      for (const auto x : left->GetLastPos()) {
        for (const auto y : right->GetFirstPos()) {
          if (position_followpos.find(x) == position_followpos.end()) {
            position_followpos.insert({x, {y}});
          } else {
            position_followpos.at(x).insert(y);
          }
        }
      }
    }

    // STAR node case
    if (node->GetNodeType() == NodeType::NODE_TYPE_STAR) {
      const auto left = node->GetLeftSubTree();
      const auto first_pos = left->GetFirstPos();
      const auto last_pos = left->GetLastPos();
      // y follows x
      for (const auto y : first_pos) {
        for (const auto x : last_pos) {
          if (position_followpos.find(x) == position_followpos.end()) {
            position_followpos.insert({x, {y}});
          } else {
            position_followpos.at(x).insert(y);
          }
        }
      }
    }

    // Put children on to the stack
    auto right_sub_tree = node->GetRightSubTree();
    auto left_sub_tree = node->GetLeftSubTree();
    if (right_sub_tree) { nodes.push(right_sub_tree); }
    if (left_sub_tree) { nodes.push(left_sub_tree); }
  }

  for (const auto& pos_followpos : position_followpos) {
    auto p = pos_followpos.first;
    auto& fp = pos_followpos.second;
    std::string fp_str = fmt::format("{} - follow ", p);
    for (const auto fpp : fp) {
      fp_str += fmt::format(" {}", fpp);
    }
    spdlog::debug(fp_str);
  }

  // Update nfa_ based on position_followpos
  for (const auto& position_follow : position_followpos) {
    const auto position = position_follow.first;
    const auto& follow = position_follow.second;

    if (nfa_.find(position) == nfa_.end()) {
      nfa_.insert({position, {}});
    }
    auto& transition_map{nfa_.at(position)};

    // Get symbols at position
    const auto position_symbols{nodepos_symbols_.at(position)};

    // position transitions into follow for every symbol in follow
    for (auto followpos : follow) {
      for (const auto symbol : position_symbols) {
        if (transition_map.find(symbol) == transition_map.end()) {
          transition_map.insert({symbol, {followpos}});
        } else {
          transition_map.at(symbol).insert(followpos);
        }
      } // symbols
    } // follow positions
  } // followpos of all positions
}

void DFA::DrawRegexTree(const Node* const tree) {
#define TOP_INDENT 40
#define CHILD_INDENT 0
  std::queue<std::pair<const Node*, int> > q;
  q.push({tree, TOP_INDENT});
  // Whenever there is a nullptr it means that it is the next level
  q.push({nullptr, -1});

  std::string level_str;
  while (!q.empty()) {
    auto node_indent = q.front();
    auto node = node_indent.first;
    auto min_indent = node_indent.second;
    q.pop();

    if (!node) {
      // Print level string and continue
      std::cout<<level_str<<std::endl;
      level_str.clear();

      // Mark the next level
      if (!q.empty()) {
	q.push({nullptr, -1});
      }
      continue;
    }

    const auto left = node->GetLeftSubTree();
    const auto right = node->GetRightSubTree();

    // Add spaces to account for min-indent if necessary
    const int extra_spaces = std::max(static_cast<int>(min_indent - level_str.length()), 0);
    level_str = level_str + std::string(extra_spaces, ' ');
    const std::string node_str = node->PrintNode();
    const int parent_str_mid = level_str.length() + node_str.length() / 2;
    level_str += node_str;

    // Add other nodes at this level
    if (left) { q.push({left, parent_str_mid - CHILD_INDENT}); }
    if (right) { q.push({right, parent_str_mid + CHILD_INDENT / 2}); }
  }

  assert (level_str.empty());
#undef TOP_INDENT
#undef CHILD_INDENT
}

void DFA::InorderTraversal(const Node* const tree) {
  if(tree) {
    InorderTraversal(tree->GetLeftSubTree());
    spdlog::info(fmt::format("Inorder {}", tree->PrintNode()));
    InorderTraversal(tree->GetRightSubTree());
  }
}

void DFA::PrintNFATransitions() {

  spdlog::info("== NFA Transitions ==");

  for (const auto& s_transitions : nfa_) {
    const auto state = s_transitions.first;
    const auto& transitions = s_transitions.second;
    const std::string state_str = fmt::format("{} ", state);
    for (const auto& symbol_sdash_set : transitions) {
      const auto symbol = symbol_sdash_set.first;
      const auto& sdashes = symbol_sdash_set.second;
      std::string transition_str{fmt::format("{} on {} ", state_str, symbol)};
      for (const auto sdash : sdashes) {
        transition_str += fmt::format("{} ", sdash);
      }
      spdlog::info(transition_str);
    }
  }
}

void DFA::PrintDFATransitions() {
  spdlog::info("== DFA Transitions ==");

  spdlog::info(fmt::format("Start State {}", dfa_start_state_));

  std::string accepting_states_str{"Accepting States "};
  for (const auto& s : dfa_accepting_states_) {
    accepting_states_str += fmt::format(" {}", s);
  }
  spdlog::info(accepting_states_str);

  for (const auto& s_transitions : dfa_) {
    const auto state = s_transitions.first;
    const auto& transitions = s_transitions.second;
    for (const auto& symbol_sdash : transitions) {
      const auto symbol = symbol_sdash.first;
      const auto sdash = symbol_sdash.second;
      const std::string transition_str{fmt::format("{} on {}  - {}", state, symbol, sdash)};
      spdlog::info(transition_str);
    }
  }
}
