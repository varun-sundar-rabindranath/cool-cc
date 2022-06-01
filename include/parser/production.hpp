#pragma once

#include <spdlog/spdlog.h>
#include <system_error>
#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_set>

struct ProductionElement;
struct production_element_hash;
struct Production;
struct production_hash;

using ProductionElementSet =
  std::unordered_set<ProductionElement, production_element_hash>;
using ProductionElement_Production_Map =
  std::unordered_map<ProductionElement, std::vector<Production>, production_element_hash>;
using ProductionElementVector = std::vector<ProductionElement>;
using ProductionVector = std::vector<Production>;

enum ProductionElementType {
  TERMINAL = 0,
  NON_TERMINAL = 1,
  INVALID = 2
};

struct ProductionElement {
  ProductionElement() : type{ProductionElementType::INVALID} {}
  ProductionElement(const ProductionElementType& type_, const std::string& e_)
    : type{type_}, element{e_} {
  }
  ProductionElementType type;
  std::string element;

  // comparison operator
  bool operator==(const ProductionElement& other) const {
    return other.type == type && other.element == element;
  }

  bool operator!=(const ProductionElement& other) const {
    return !(*this == other);
  }

  std::ostream& operator<<(std::ostream& stream) const {
    stream << to_string();
    return stream;
  }

  bool IsTerminal() const {
    return type == ProductionElementType::TERMINAL;
  }

  bool IsNonTerminal() const {
    return type == ProductionElementType::NON_TERMINAL;
  }

  std::string to_string() const {
    std::string s;
    if (type == ProductionElementType::TERMINAL) { s = " TERMINAL - "; }
    if (type == ProductionElementType::NON_TERMINAL) { s = " NON TERMINAL - "; }
    if (type == ProductionElementType::INVALID) { s = " INVALID PE - "; }
    s += element + " ";
    return s;
  }
};

// Hash function for ProductionElement
struct production_element_hash {
  std::size_t operator()(const ProductionElement& pe) const {
    return std::hash<std::string>{}(pe.element);
  }
};

// A production is a collection of terminals and nonterminals
struct Production {

  public:

    // Must be a non terminal
    ProductionElement left;
    // Denotes as epsilon production if right is empty
    std::vector<ProductionElement> right;

    Production(const ProductionElement left_,
               std::vector<ProductionElement> right_) : left{left_}, right{right_} {
      if (left.type != NON_TERMINAL) {
        throw std::runtime_error("Non Terminal is not the left side of production");
      }
    }

    bool is_epsilon_production() const {
      return right.empty();
    }

    bool operator==(const Production& other) const {
      if (left != other.left) { return false; }
      if (right.size() != other.right.size()) { return false; }
      for (std::size_t i = 0; i < right.size(); ++i) {
        if (right.at(i) != other.right.at(i)) { return false; }
      }
      return true;
    }

    bool operator!= (const Production& other) const {
      return !(*this == other);
    }

    std::ostream& operator<<(std::ostream& stream) const {
      stream << to_string();
      return stream;
    }

    std::string to_string() const {
      std::string s;
      s += left.element + " -> ";

      if (right.empty()) { s += "empty "; }

      for (const auto& r_element : right) {
        s += r_element.element + std::string(" ");
      }
      return s;
    }

    // Used in the Parser Generator to assign a function name to the semantic ruke
    std::string to_function_name() const {
      std::string s;
      s += left.element + "_FNAME_";
      for (const auto& r_element : right) {
	s += r_element.element + std::string("_FNAME_");
      }
      return s;
    }
};

// Hash function for Production
struct production_hash {
  std::size_t operator()(const Production& p) const {
    return std::hash<std::string>{}(p.to_string());
  }
};
