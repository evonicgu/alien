#ifndef ALIEN_PARSER_SLR_ALGORITHM_H
#define ALIEN_PARSER_SLR_ALGORITHM_H

#include <set>
#include <utility>
#include "parser/algorithm/algorithm.h"
#include "parser/config/rules/rules.h"
#include "util/vecset.h"

namespace alien::parser::algorithm::slr {

    using item = std::tuple<unsigned int, unsigned int, unsigned int>;

    using namespace config::rules;

    bool is_base(const item& it) {
        return std::get<0>(it) == 0 || std::get<2>(it) != 0;
    }

    std::vector<item> closure(const std::vector<item>& items, const rules& grammar, const alphabet& symbols) {
        util::vecset<item> closure{items};

        for (unsigned int i = 0; i < closure.size(); ++i) {
            const production& prod = grammar.ruleset.at(std::get<0>(closure[i]))[std::get<1>(closure[i])];

            unsigned int pos = std::get<2>(closure[i]), symbol;

            if (pos >= prod.symbols.size()) {
                continue;
            }

            symbol = prod.symbols[pos];

            if (symbols[symbol].type == grammar_symbol::symbol_type::TERMINAL) {
                continue;
            }

            for (unsigned int j = 0; j < grammar.ruleset.at(symbol).size(); ++j) {
                closure.push_back({symbol, j, 0});
            }
        }

        return (std::vector<item>) closure;
    }

    std::vector<item> move(const std::vector<item>& items, unsigned int symbol, const rules& grammar) {
        std::vector<item> moved;

        for (const item& it : items) {
            const production& prod = grammar.ruleset.at(std::get<0>(it))[std::get<1>(it)];

            unsigned int pos = std::get<2>(it);

            if (pos < prod.symbols.size() && prod.symbols[pos] == symbol) {
                moved.push_back({std::get<0>(it), std::get<1>(it), pos + 1});
            }
        }

        return moved;
    }

    parsing_table build_table(const rules& grammar, const alphabet& symbols) {
        symbol_props first, follow;
        build_follow(grammar, symbols, follow, first);
        util::vecset<std::vector<item>> states(std::set<std::vector<item>>{{
            closure({{0, 0, 0}}, grammar, symbols)}
        });

        parsing_table table{1};

        for (unsigned int i = 0; i < states.size(); ++i) {
            if (i > 0) {
                for (const auto& it : states[i]) {
                    const production& prod = grammar.ruleset.at(std::get<0>(it))[std::get<1>(it)];

                    if (std::get<2>(it) < prod.symbols.size()) {
                        continue;
                    }

                    parsing_action action{
                        .arg1 = std::get<0>(it),
                        .arg2 = std::get<1>(it)
                    };

                    if (std::get<0>(it) == grammar.start) {
                        action.type = parsing_action::action_type::ACCEPT;

                        insert(table, i, -2, action, symbols, grammar);
                        continue;
                    }

                    action.type = parsing_action::action_type::REDUCE;

                    for (int symbol : follow[std::get<0>(it)]) {
                        insert(table, i, symbol, action, symbols, grammar);
                    }
                }
            }

            for (unsigned int j = 1; j < symbols.size(); ++j) {
                std::vector<item> next = closure(move(states[i], j, grammar), grammar, symbols);

                if (next.empty()) {
                    continue;
                }

                unsigned int to_state = states.push_back(std::move(next));

                if (to_state >= table.size()) {
                    table.emplace_back();
                }

                insert(table, i, (int) j, {parsing_action::action_type::SHIFT, to_state}, symbols, grammar);
            }
        }

        return table;
    }

}

#endif //ALIEN_PARSER_SLR_ALGORITHM_H