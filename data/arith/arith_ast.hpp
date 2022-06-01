#include <memory>
#include <parser/parser_node.hpp>

class S_Node : public ParseTreeNode {
  public:
  S_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r) : ParseTreeNode(r) {
  }
};

class E_Node : public ParseTreeNode {
  public:
  E_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r) : ParseTreeNode(r) {
  }
};

class E_DASH_Node : public ParseTreeNode {
  public:
  E_DASH_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r) : ParseTreeNode(r) {
  }
};

class T_Node : public ParseTreeNode {
  public:
  T_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r) : ParseTreeNode(r) {
  }
};

class T_DASH_Node : public ParseTreeNode {
  public:
  T_DASH_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r) : ParseTreeNode(r) {
  }
};

class F_Node : public ParseTreeNode {
  public:
  F_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r) : ParseTreeNode(r) {
  }
};

// Terminal nodes

class Empty_Node : public ParseTreeNode {
  public:
  Empty_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r = {}) : ParseTreeNode(r) {
  }
};

class Open_Brace_Node : public ParseTreeNode {
  public:
  Open_Brace_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r = {}) : ParseTreeNode(r) {
  }
};

class Close_Brace_Node : public ParseTreeNode {
  public:
  Close_Brace_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r = {}) : ParseTreeNode(r) {
  }
};

class ID_Node : public ParseTreeNode {
  public:
  ID_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r = {}) : ParseTreeNode(r) {
  }
};

class Mul_Node : public ParseTreeNode {
  public:
  Mul_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r = {}) : ParseTreeNode(r) {
  }
};

class Plus_Node : public ParseTreeNode {
  public:
  Plus_Node(const std::vector<std::shared_ptr<ParseTreeNode>> r = {}) : ParseTreeNode(r) {
  }
};
