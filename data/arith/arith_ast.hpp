#include <memory>
#include <parser/parser_node.hpp>

class S_Node : ParseTreeNode {
  S_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r) : ParseTreeNode(r) {
  }
};

class E_Node : ParseTreeNode {
  E_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r) : ParseTreeNode(r) {
  }
};

class E_DASH_Node : ParseTreeNode {
  E_DASH_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r) : ParseTreeNode(r) {
  }
};

class T_Node : ParseTreeNode {
  T_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r) : ParseTreeNode(r) {
  }
};

class T_DASH_Node : ParseTreeNode {
  T_DASH_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r) : ParseTreeNode(r) {
  }
};

class F_Node : ParseTreeNode {
  F_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r) : ParseTreeNode(r) {
  }
};

// Terminal nodes

class Empty_Node : ParseTreeNode {
  Empty_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r = {}) : ParseTreeNode(r) {
  }
};

class Open_Brace_Node : ParseTreeNode {
  Open_Brace_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r = {}) : ParseTreeNode(r) {
  }
};

class Close_Brace_Node : ParseTreeNode {
  Close_Brace_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r = {}) : ParseTreeNode(r) {
  }
};

class ID_Node : ParseTreeNode {
  ID_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r = {}) : ParseTreeNode(r) {
  }
};

class Mul_Node : ParseTreeNode {
  Mul_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r = {}) : ParseTreeNode(r) {
  }
};

class Plus_Node : ParseTreeNode {
  Plus_Node(const std::vector<std::shared_ptr<ParseTreeNode>>& r = {}) : ParseTreeNode(r) {
  }
};
