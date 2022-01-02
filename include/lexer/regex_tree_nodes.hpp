#ifndef __REGEX_TREE_NODES_HPP__
#define __REGEX_TREE_NODES_HPP__

#include <unordered_set>
#include <cassert>
#include <set>

enum NodeType {
  NODE_TYPE_OR = 0,
  NODE_TYPE_CAT = 1,
  NODE_TYPE_STAR = 2,
  NODE_TYPE_LEAF = 3,
  NODE_TYPE_INVALID = 4
};

class Node {
public:
  Node();
  ~Node() = default;

  virtual void ComputeFirstPos() = 0;
  virtual void ComputeLastPos() = 0;
  virtual void ComputeIsNullable() = 0;
  virtual std::string PrintNode() const = 0;

  int GetNumNodes() const { return num_nodes_; }
  Node* GetLeftSubTree() const { return left_; }
  Node* GetRightSubTree() const { return right_; }
  NodeType GetNodeType() const { return node_type_; }
  bool GetIsNullable() const { return is_nullable_; }
  std::unordered_set<int> GetFirstPos() const { return first_pos_; }
  std::unordered_set<int> GetLastPos() const { return last_pos_; }

protected:
  NodeType node_type_{NODE_TYPE_INVALID};

  Node* left_{nullptr};
  Node* right_{nullptr};

  int num_nodes_{0};
  std::unordered_set<int> first_pos_;
  std::unordered_set<int> last_pos_;
  bool is_nullable_{false};
};

class ORNode final : public Node {
public:
  ORNode(Node* const left, Node* const right);
  ~ORNode() = default;

  void ComputeFirstPos();
  void ComputeLastPos();
  void ComputeIsNullable();
  std::string PrintNode() const;
};

class CatNode final : public Node {
public:
  CatNode(Node* const left, Node* const right);
  ~CatNode() = default;

  void ComputeFirstPos();
  void ComputeLastPos();
  void ComputeIsNullable();
  std::string PrintNode() const;
};

class StarNode final : public Node {
public:
  StarNode(Node* const left); // Star is unary; Use only the left child
  ~StarNode() = default;

  void ComputeFirstPos();
  void ComputeLastPos();
  void ComputeIsNullable();
  std::string PrintNode() const;
};

class LeafNode final : public Node {
public:
  // Pass a single character as a string of length 1
  // Pass a "[A-Z]" like definitions as a string
  LeafNode(const std::string& symbol);
  ~LeafNode() = default;

  std::set<char> GetSymbols() const { return symbols_; };

  void ComputeFirstPos();
  void ComputeLastPos();
  void ComputeIsNullable();

  void SetNodePosition(const int node_pos) { node_position_ = node_pos; };
  int GetNodePosition() const  { return node_position_; }

  std::string PrintNode() const;
private:
  std::set<char> symbols_;
  std::string symbols_str_;

  // The leaf nodes in the regex tree are assigned a number that will be used
  // to refer to them in first_pos_, last_pos_ and follow_pos_
  int node_position_{-1};
};


#endif // __REGEX_TREE_NODES_HPP__
