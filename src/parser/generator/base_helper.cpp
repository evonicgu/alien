#include "parser/generator/base_helper.h"

namespace alien::parser::generator {

    const std::unordered_set<std::ptrdiff_t>& base_helper::get_first(std::size_t symbol) {
        const auto& set = first[symbol];

        if (!set.empty()) {
            return set;
        }

        first[symbol] = {};

        std::vector<std::size_t> recursive_rules;

        for (std::size_t i = 0; i < rules.ruleset[symbol].size(); ++i) {
            const rules::production& prod = rules.ruleset[symbol][i];

            if (prod.symbols.empty()) {
                first[symbol].insert(-1);
                continue;
            }

            bool nullable = true;

            for (std::size_t j = 0; j < prod.symbols.size(); ++j) {
                const auto& current_symbol = prod.symbols[j];

                if (current_symbol.type == rules::symbol_type::NON_TERMINAL && current_symbol.index == symbol) {
                    nullable = false;
                    recursive_rules.push_back(j);
                    break;
                }

                if (current_symbol.type == rules::symbol_type::TERMINAL) {
                    nullable = false;
                    first[symbol].insert(current_symbol.index);
                    break;
                }

                const auto& next_first = get_first(current_symbol.index);

                for (auto first_symbol : next_first) {
                    if (first_symbol != -1) {
                        first[symbol].insert(first_symbol);
                    }
                }

                if (next_first.find(-1) == next_first.end()) {
                    nullable = false;
                    break;
                }
            }

            if (nullable) {
                first[symbol].insert(-1);
            }
        }

        if (recursive_rules.size() == rules.ruleset[symbol].size()) {
            throw std::runtime_error("Infinitely recursive rule");
        }

        for (std::size_t i = 0; i < recursive_rules.size(); ++i) {
            first[symbol].merge(get_first(rules.ruleset[symbol][i].symbols));
        }

        return first[symbol];
    }

    std::unordered_set<std::ptrdiff_t> base_helper::get_first(const std::vector<rules::grammar_symbol>& str, std::size_t start_index) {
        std::unordered_set<std::ptrdiff_t> first_set;

        if (str.empty()) {
            first_set.insert(-1);
            return first_set;
        }

        bool nullable = true;

        for (std::size_t i = start_index; i < str.size();  i++) {
            const auto& symbol = str[i];

            if (symbol.type == rules::symbol_type::TERMINAL) {
                nullable = false;
                first_set.insert(symbol.index);
                break;
            }

            const std::unordered_set<std::ptrdiff_t>& next_first = get_first(symbol.index);

            for (auto first_symbol : next_first) {
                if (first_symbol != -1) {
                    first_set.insert(first_symbol);
                }
            }

            if (next_first.find(-1) == next_first.end()) {
                nullable = false;
                break;
            }
        }

        if (nullable) {
            first_set.insert(-1);
        }

        return first_set;
    }

}