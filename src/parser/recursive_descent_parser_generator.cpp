#include <parser/recursive_descent_parser_generator.hpp>
#include <fstream>

RecursiveDescentParserGenerator::RecursiveDescentParserGenerator(
  const std::string& grammar_filename) : ParserGenerator(grammar_filename),
    first_{}, follow_{}, rd_parsing_table_{}
{

  ComputeFirst();
  ComputeFollow();

  DumpFirst();

  DumpFollow();

  ComputeParsingTable();

  DumpParsingTable();

}

// getters
RecursiveDescentParserGenerator::ProductionElementFirstSet
RecursiveDescentParserGenerator::GetFirsts() const {
  return first_;
}

RecursiveDescentParserGenerator::ProductionElementFollowSet
RecursiveDescentParserGenerator::GetFollows() const {
  return follow_;
}

std::vector<Production> RecursiveDescentParserGenerator::GetParsingTableProductions(
      const ProductionElement& nt, const ProductionElement& t) const {
  const std::size_t nt_idx{non_terminal_id_map_.at(nt)};
  const std::size_t t_idx{terminal_id_map_.at(t)};
  std::vector<Production> productions;
  for (const auto& p_idx : rd_parsing_table_[nt_idx][t_idx]) {
    productions.push_back(productions_.at(p_idx));
  }
  return productions;
}

// Compute functions
void RecursiveDescentParserGenerator::ComputeFirst() {
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

bool RecursiveDescentParserGenerator::ComputeFirst(const ProductionElement& pe) {

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

void RecursiveDescentParserGenerator::ComputeFollow() {
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

void RecursiveDescentParserGenerator::ComputeFollowPass() {

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

      if (p_j == p.right.size() && p.left != pi_e) {
        // p_j fell of the end of the production - This means whatever follows
        // the left of the production follows p_i also
        add_follow_to_follow(p.left, pi_e);
      }
    } // production right side iterator
  } // productions iterator
}

void RecursiveDescentParserGenerator::ComputeParsingTable() {

  auto add_firsts_of_production_element = [&](const ProductionElement& x,
                                              ProductionElementSet* const pe_set) {
    assert (pe_set);
    const auto& first_set{first_.at(x)};
    for (const auto& y : first_set) {
      if (y == kEmptyTerminal) {
        // Don't add the empty terminal - Maybe a terminal follows this NT
        continue;
      }
      assert (y.IsTerminal());
      pe_set->insert(y);
    }
  };

  const std::size_t n_terminals{terminals_.size()};
  const std::size_t n_non_terminals{non_terminals_.size()};

  // Initialize parsing table
  rd_parsing_table_.clear();
  rd_parsing_table_.resize(n_non_terminals);
  for (auto& pt_row : rd_parsing_table_) {
    pt_row.resize(n_terminals, {});
  }

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
    const bool p_can_be_empty{r_i == p.right.size()};

    const auto p_id{production_id_map_[p]};
    const auto nt_id{non_terminal_id_map_[p.left]};

    // Add the production_id to all rd_parsing_table[p.left][first_set_element]
    for (const auto& p_first : p_firsts) {
      assert (p_first.IsTerminal());
      const auto t_id{terminal_id_map_[p_first]};
      rd_parsing_table_[nt_id][t_id].push_back(p_id);
    }

    // Add the production_id to all Follow[p.left] if p_can_be_empty
    if (p_can_be_empty) {
      for (const auto& nt_follow : follow_[p.left]) {
        assert (nt_follow.IsTerminal());
        const auto t_id{terminal_id_map_[nt_follow]};
        rd_parsing_table_[nt_id][t_id].push_back(p_id);
      }
    }
  } // productions_
}

// Parsing table writer
void RecursiveDescentParserGenerator::WriteParsingTable(
  const std::string& filename) const {

  spdlog::debug("Write Parsing Table to {}", filename);

  // Open file stream
  std::fstream f;
  f.open(filename, std::fstream::out | std::fstream::trunc);
  if (!f.is_open()) {
    throw std::runtime_error(fmt::format("{} - Open failed | {}", filename, strerror(errno)));
  }

  auto define_production_element_vector =
    [](const ProductionElementVector& pes, const std::string& var_name)
    -> std::string {

    std::string definition;

    // Add required headers
    definition += "#include <parser/production.hpp> // ProductionElementVector\n";

    // Start ProductionElementVector definition
    definition += fmt::format("extern const ProductionElementVector {}{{\n",
                              var_name);

    // Define production elements
    for (std::size_t i = 0; i < pes.size(); ++i) {
      const std::string pe_type_string =
        pes.at(i).type == ProductionElementType::TERMINAL ?
        "ProductionElementType::TERMINAL" : "ProductionElementType::NON_TERMINAL";

      definition += fmt::format(
        " ProductionElement {{ {}, \"{}\" }}", pe_type_string, pes.at(i).element);
      definition += i != pes.size() - 1 ? ",\n" : "\n";
    }

    // Close ProductionElementVector definition
    definition += " };";
    return definition;
  };

  auto define_production_element_id_map =
    [](const ProductionElementIDMap& pe_id_map, const std::string& var_name)
    -> std::string {

      std::string definition;

      // Add required headers
      definition += "#include <parser/parser_generator.hpp> // ProductionElementIDMap\n";
      definition += "#include <parser/production.hpp> // ProductionElementVector\n";

      // Start ProductionElementIDMap definition
      definition += fmt::format(
        "extern const ParserGenerator::ProductionElementIDMap {}{{\n", var_name);

      // Define production element map entries
      std::size_t element_idx{0};
      for (const auto& pe_id : pe_id_map) {
        const std::string pe_type_string =
          pe_id.first.type == ProductionElementType::TERMINAL ?
          "ProductionElementType::TERMINAL" : "ProductionElementType::NON_TERMINAL";

        definition += fmt::format("  {{ ProductionElement{{ {}, \"{}\" }}, {} }}",
                                  pe_type_string, pe_id.first.element, pe_id.second);
        definition += element_idx == pe_id_map.size() - 1 ? "\n" : ",\n";
        element_idx++;
      }

      // End ProductionElementIDMap definition
      definition += " };";

      return definition;
    };

  auto define_parsing_table = [](
    const ProductionElementIDMap& terminal_id_map,
    const ProductionElementIDMap& non_terminal_id_map,
    const ProductionIDMap& production_id_map,
    const RDParsingTable& parsing_table,
    const std::string& var_name) -> std::string {

      std::vector<ProductionElement> terminals(terminal_id_map.size(),
                                               ProductionElement{});
      std::vector<ProductionElement> non_terminals(non_terminal_id_map.size(),
                                                   ProductionElement{});
      std::vector<Production> productions;

      // Position terminals and non-terminals based on their ids
      for (const auto& terminal_id : terminal_id_map) {
        terminals.at(terminal_id.second) = terminal_id.first;
      }
      for (const auto& non_terminal_id : non_terminal_id_map) {
        non_terminals.at(non_terminal_id.second) = non_terminal_id.first;
      }
      // Position productions based on their ids
      {
        for (const auto& p_id : production_id_map) {
          productions.push_back(p_id.first);
        }
        std::sort(productions.begin(), productions.end(),
                  [&](const Production& a, const Production& b){
                    return production_id_map.at(a) < production_id_map.at(b);
                  });
      }

      std::string definition;

      // Add required headers
      definition += "#include <vector>\n";

      // Add productions and production ids as comments
      definition += "// Productions and IDs\n";
      for (const auto& p : productions) {
        definition += fmt::format("// {} - {}\n",
                                  production_id_map.at(p), p.to_string());
      }

      // Start table definition
      definition += fmt::format(
        "extern const std::vector<std::vector<std::vector<std::size_t>>> {}{{\n", var_name);

      // Add table definitions
      for (std::size_t nt_idx = 0; nt_idx < non_terminals.size(); nt_idx++) {
        definition += "  {";
        for (std::size_t t_idx = 0; t_idx < terminals.size(); t_idx++) {

          // Start production ids definition block
          definition += "{";
          const auto& production_ids{parsing_table[nt_idx][t_idx]};
          for (std::size_t pidx = 0; pidx < production_ids.size(); pidx++) {
            definition += fmt::format("{}", production_ids.at(pidx));
            definition += pidx == production_ids.size() - 1 ? "" : ",";
          }
          definition += "}";
          definition += t_idx == terminals.size() - 1 ? "" : ", ";
        }
        definition += "}";
        definition += nt_idx == non_terminals.size() - 1 ? "\n" : ",\n";
      }


      // Close table definition
      definition += "};\n";

      return definition;
    };


  // Write Terminals definition
  {
    const std::string definition{
      define_production_element_vector(terminals_, "TERMINALS_DEFINITION")};
    f << "// Terminals\n"<< std::endl << definition << std::endl;
  }

  // Write Non Terminals definition
  {
    const std::string definition{
      define_production_element_vector(non_terminals_, "NON_TERMINALS_DEFINITION")};
    f << "// Non Terminals\n"<< std::endl << definition << std::endl;
  }

  // Write Terminals - ID map
  {
    const std::string definition {
      define_production_element_id_map(terminal_id_map_,
                                       "TERMINALS_ID_MAP_DEFINITION")};
    f << " // Terminals ID Map\n"<< std::endl << definition << std::endl;
  }

  // Write Non Terminals - ID map
  {
    const std::string definition {
      define_production_element_id_map(non_terminal_id_map_,
                                       "NON_TERMINALS_ID_MAP_DEFINITION")};
    f << " // Non Terminals ID Map\n"<< std::endl << definition << std::endl;
  }

  // Write ParsingTable
  {
    const std::string definition {
        define_parsing_table(terminal_id_map_, non_terminal_id_map_,
                             production_id_map_, rd_parsing_table_,
                             "PARSING_TABLE_DEFINITION")};
    f << " // Parsing Table\n"<< std::endl << definition << std::endl;
  }

  f.close();
}

// Dump functions
void RecursiveDescentParserGenerator::Dump() const {

  ParserGenerator::Dump();

  DumpFirst();

  DumpFollow();

  DumpParsingTable();
}

void RecursiveDescentParserGenerator::DumpFirst() const {
  // dump firsts
  spdlog::debug("Firsts ...");
  for (const auto& pe_terminals : first_) {
    spdlog::debug("Firsts of {} is ", pe_terminals.first.to_string());
    for (const auto& t : pe_terminals.second) {
      spdlog::debug(" - {}", t.to_string());
    }
  }
}

void RecursiveDescentParserGenerator::DumpFollow() const {
  // dump follow
  spdlog::debug("Follow ...");
  for (const auto& pe_terminals : follow_) {
    spdlog::debug("Follows of {} is ", pe_terminals.first.to_string());
    for (const auto& t : pe_terminals.second) {
      spdlog::debug(" - {}", t.to_string());
    }
  }
}

void RecursiveDescentParserGenerator::DumpParsingTable() const {

  // dump parsing table
  if (rd_parsing_table_.empty()) {
    spdlog::debug("Parsing table is empty");
    return;
  }

  const std::size_t n_rows{rd_parsing_table_.size()};
  const std::size_t n_cols{rd_parsing_table_.at(0).size()};

  const std::size_t cell_pre_padding{2};
  const std::size_t cell_post_padding{2};

  std::size_t cell_width{0};
  std::size_t cell_height{0};
  for (std::size_t r = 0; r < n_rows; ++r) {
    for (std::size_t c = 0; c < n_cols; ++c) {
      cell_height = std::max(cell_height, rd_parsing_table_[r][c].size());
      for (const auto p_id : rd_parsing_table_[r][c]) {
        const std::size_t p_size{productions_[p_id].to_string().length()};
        cell_width = std::max(cell_width, p_size);
      }
    }
  }

  auto add_cell_text = [&](std::string* const dump_str,
                           const std::string& cell_text) {
    *dump_str += fmt::format("{:>{}}", cell_text, cell_pre_padding) +
                 fmt::format("{:>{}}", "", (cell_width - cell_text.length()) + cell_post_padding);
  };

  auto add_cell_entry = [&](std::string* const dump_str, std::size_t entry_idx,
                            int m, int n) {
    if (rd_parsing_table_[m][n].size() <= entry_idx) {
      // just cell_width + cell_padding spaces
      add_cell_text(dump_str, entry_idx == 0 ? "Err" : "");
      return;
    }

    const auto p_id = rd_parsing_table_[m][n][entry_idx];
    const auto& entry = productions_[p_id];
    add_cell_text(dump_str, entry.to_string());
  };

  // dump header row (terminals)
  std::string header;
  add_cell_text(&header, "");
  for (std::size_t c = 0; c < n_cols; ++c) {
    add_cell_text(&header, terminals_.at(c).element);
  }
  spdlog::debug("{}", header);
  for (std::size_t r = 0; r < n_rows; ++r) {
    std::string row_str;
    for (std::size_t ch = 0; ch < cell_height; ++ch) {
      if (ch == 0) {
        add_cell_text(&row_str, non_terminals_.at(r).element);
      } else {
        add_cell_text(&row_str, "");
      }

      for (std::size_t c = 0; c < n_cols; ++c) {
        add_cell_entry(&row_str, ch, r, c);
      }
    }

    spdlog::debug("{}", row_str);
  }
}
