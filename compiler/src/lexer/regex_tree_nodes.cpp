// Define regex tree nodes
#include "lexer/regex_tree_nodes.hpp"
#include "lexer/lex_character_classes.hpp"
#include <algorithm>
#include <cassert>
#include <fmt/format.h>

Node::Node() = default;

ORNode::ORNode(const std::shared_ptr<Node> left,
	       const std::shared_ptr<Node> right) {
  assert (left);
  assert (right);

  node_type_ = NodeType::NODE_TYPE_OR;
  left_ = left;
  right_ = right;

  num_nodes_ = left_ ? left_->GetNumNodes() : 0;
  num_nodes_ += right_ ? right_->GetNumNodes() : 0;
  num_nodes_ += 1;
}

void ORNode::ComputeFirstPos() {
  left_->ComputeFirstPos();
  right_->ComputeFirstPos();
  const auto left_fp{left_->GetFirstPos()};
  const auto right_fp{right_->GetFirstPos()};

  std::set_union(left_fp.begin(), left_fp.end(), right_fp.begin(), right_fp.end(),
		 std::inserter(first_pos_, first_pos_.end()));
}

void ORNode::ComputeLastPos() {
  left_->ComputeLastPos();
  right_->ComputeLastPos();
  const auto left_lp{left_->GetLastPos()};
  const auto right_lp{right_->GetLastPos()};

  std::set_union(left_lp.begin(), left_lp.end(), right_lp.begin(), right_lp.end(),
		 std::inserter(last_pos_, last_pos_.end()));
}

void ORNode::ComputeIsNullable() {
  left_->ComputeIsNullable();
  right_->ComputeIsNullable();
  is_nullable_ = left_->GetIsNullable() || right_->GetIsNullable();
}

std::string ORNode::PrintNode()  const {

  std::string fp_string;
  std::string lp_string;
  for (auto idx : first_pos_) { fp_string += fmt::format(" {}", idx); }
  for (auto idx : last_pos_) { lp_string += fmt::format(" {}", idx); }
  return fmt::format("OR-NODE({}) F ({}) L ({})",
		     is_nullable_ ? "N" : "!N",
		     fp_string, lp_string);
}

CatNode::CatNode(const std::shared_ptr<Node> left,
		 const std::shared_ptr<Node> right) {
  assert (left);
  assert (right);

  node_type_ = NodeType::NODE_TYPE_CAT;
  left_ = left;
  right_ = right;

  num_nodes_ = left_ ? left_->GetNumNodes() : 0;
  num_nodes_ += right_ ? right_->GetNumNodes() : 0;
  num_nodes_ += 1;
}

void CatNode::ComputeFirstPos() {

  left_->ComputeFirstPos();
  right_->ComputeFirstPos();

  const auto left_fp{left_->GetFirstPos()};
  const auto right_fp{right_->GetFirstPos()};

  // if left is nullable - then go with left and right
  // otherwise just go with left ??
  if (left_->GetIsNullable()) {
    std::set_union(left_fp.begin(), left_fp.end(), right_fp.begin(), right_fp.end(),
  		   std::inserter(first_pos_, first_pos_.end()));
  } else {
    first_pos_ = left_fp;
  }
}

void CatNode::ComputeLastPos() {

  left_->ComputeLastPos();
  right_->ComputeLastPos();

  const auto left_lp{left_->GetLastPos()};
  const auto right_lp{right_->GetLastPos()};

  // if left is nullable - then go with left and right
  // otherwise just go with left ??
  if (right_->GetIsNullable()) {
    std::set_union(left_lp.begin(), left_lp.end(), right_lp.begin(), right_lp.end(),
  		   std::inserter(last_pos_, last_pos_.end()));
  } else {
    last_pos_ = right_lp;
  }
}

void CatNode::ComputeIsNullable() {
  left_->ComputeIsNullable();
  right_->ComputeIsNullable();
  is_nullable_ = left_->GetIsNullable() && right_->GetIsNullable();
}

std::string CatNode::PrintNode()  const {

  std::string fp_string;
  std::string lp_string;
  for (auto idx : first_pos_) { fp_string += fmt::format(" {}", idx); }
  for (auto idx : last_pos_) { lp_string += fmt::format(" {}", idx); }
  return fmt::format("CAT-NODE({}) F ({}) L ({})",
		     is_nullable_ ? "N" : "!N",
		     fp_string, lp_string);
}

StarNode::StarNode(const std::shared_ptr<Node> left) {
  assert(left);

  node_type_ = NodeType::NODE_TYPE_STAR;
  left_ = left;
  right_ = nullptr;

  num_nodes_ = left_ ? left_->GetNumNodes() : 0;
  num_nodes_ += 1;
}

void StarNode::ComputeFirstPos() {
  // Need this to propagate so we can compute first pose for nodes below
  left_->ComputeFirstPos();
  first_pos_ = left_->GetFirstPos();
}

void StarNode::ComputeLastPos() {
  left_->ComputeLastPos();
  last_pos_ = left_->GetLastPos();
}

void StarNode::ComputeIsNullable() {
  // Need this to propagate so we can compute IsNullable for nodes below
  left_->ComputeIsNullable();
  is_nullable_ = true;
}

std::string StarNode::PrintNode()  const {

  std::string fp_string;
  std::string lp_string;
  for (auto idx : first_pos_) { fp_string += fmt::format(" {}", idx); }
  for (auto idx : last_pos_) { lp_string += fmt::format(" {}", idx); }
  return fmt::format("STAR-NODE({}) F ({}) L ({})",
		     is_nullable_ ? "N" : "!N",
		     fp_string, lp_string);
}

LeafNode::LeafNode(const std::string& symbol) {
  assert (!symbol.empty());
  node_type_ = NodeType::NODE_TYPE_LEAF;
  left_ = nullptr;
  right_ = nullptr;

  symbols_str_ = symbol;
  symbols_ = symbol.length() == 1 ? std::set<char>{symbol.at(0)} :
    LexCharacterClasses::GetCharactersInClass(symbol);

  if (symbol.length() == 1) {
    // single symbol like 'a', 'b' etc
    symbols_ = symbol.at(0) == '.' ?
      LexCharacterClasses::GetCharactersForPeriod() :
      std::set<char>{symbol.at(0)};
  } else {
    // This can be an escape sequence like \[, \], \\ or,
    // This can be a character class like [a-z], [A-Z], [\[abc]
    // If it is character class definition remove the enclosing box parens
    if (symbol.length() == 2) {
      // Process escape sequence
      symbols_ = LexCharacterClasses::GetCharactersInClass(symbol);
    } else {
      // This must be a character class definition; Sanity check
      assert (symbol.front() == '[' && symbol.back() == ']');
      symbols_ = LexCharacterClasses::GetCharactersInClass(symbol.substr(1, symbol.size() - 2));
    }
  }

  num_nodes_ = 1;
}

void LeafNode::ComputeFirstPos() {
  // This is a symbol(s) node ! The first pos is that symbol(s) itself
  first_pos_.insert(node_position_);
}

void LeafNode::ComputeLastPos() {
  // This is a symbol(s) node ! The last pos is that symbol(s) itself
  last_pos_.insert(node_position_);
}

void LeafNode::ComputeIsNullable() {
  is_nullable_ = false;
}

std::string LeafNode::PrintNode()  const {
  std::string fp_string;
  std::string lp_string;
  for (auto idx : first_pos_) { fp_string += fmt::format(" {}", idx); }
  for (auto idx : last_pos_) { lp_string += fmt::format(" {}", idx); }
  std::string symbols_list;
  for (const auto x : symbols_) {
    symbols_list = fmt::format("{} {}", symbols_list, x);
  }
  return fmt::format("LEAF-NODE({}) - {} - F ({}) L ({}) {}",
		     is_nullable_ ? "N" : "!N",
                     symbols_str_,
		     fp_string, lp_string, symbols_list);
}
