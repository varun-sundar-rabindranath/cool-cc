#pragma once
/**
 * This is the base-class for all the grammar ast nodes
 *  - All the grammar AST nodes should necessarily inherit from this ParserNode
 *  - All the semantic rules in the grammar file definitions must return this
 *    ParserNode
 */
#include <vector>
#include <memory>

class ParseTreeNode {
  public:
    ParseTreeNode(const std::vector<std::shared_ptr<ParseTreeNode>> right) 
    : right_{right} {
    }
  private:
    std::vector<std::shared_ptr<ParseTreeNode>> right_;
};
