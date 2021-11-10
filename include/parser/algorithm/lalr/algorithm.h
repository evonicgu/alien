#ifndef ALIEN_PARSER_LALR_ALGORITHM_H
#define ALIEN_PARSER_LALR_ALGORITHM_H

#include <set>
#include "parser/algorithm/algorithm.h"
#include "parser/algorithm/slr/algorithm.h"
#include "parser/algorithm/clr/algorithm.h"

namespace alien::parser::algorithm::lalr {

    using item_index = std::tuple<unsigned int, unsigned int>;
    using lookahead_table = std::map<item_index, std::set<int>>;
    using propagation_table = std::map<item_index, std::set<item_index>>;
    using set_mapping = std::vector<std::vector<unsigned int>>;

    template<typename Item>
    using pitems = std::vector<std::vector<Item>>;

    using namespace util::literals;

    pitems<slr::item> build_items(const rules& grammar, const alphabet& symbols, symbol_props& first,
                                  set_mapping& mapping) {
        symbol_props follow;
        mapping = {std::vector<unsigned int>(symbols.size())};
        build_follow(grammar, symbols, follow, first);

        util::vecset<std::vector<slr::item>> states(std::set<std::vector<slr::item>>{{
            slr::closure({{0, 0, 0}}, grammar, symbols)
        }});

        for (unsigned int i = 0; i < states.size(); ++i) {
            for (unsigned int j = 1; j < symbols.size(); ++j) {
                std::vector<slr::item> next = slr::closure(slr::move(states[i], j, grammar), grammar, symbols);

                if (next.empty()) {
                    continue;
                }

                unsigned int to_state = states.push_back(std::move(next));

                if (to_state == states.size() - 1) {
                    mapping.push_back({std::vector<unsigned int>(symbols.size())});
                }

                mapping[i][j] = to_state;
            }
        }

        return (pitems<slr::item>) states;
    }

    struct lookahead_producer {
        const alphabet& symbols;
        const rules& grammar;
        symbol_props& first;
        const set_mapping& mapping;
        const pitems<slr::item>& items;

        void get_lookahead(unsigned int from, lookahead_table& table, propagation_table& prop_table);
    };

    void lookahead_producer::get_lookahead(unsigned int from, lookahead_table& table, propagation_table& prop_table) {
        for (unsigned int i = 0; i < items[from].size(); ++i) {
            const slr::item& item = items[from][i];

            if (!slr::is_base(item)) {
                break;
            }

            auto [rule, prod, pos] = item;

            for (unsigned int j = 1; j < symbols.size(); ++j) {
                std::vector<clr::item> closure = clr::closure({{rule, prod, pos, -3}}, grammar, symbols, first);

                for (auto& it : closure) {
                    auto [crule, cprod, cpos, clookahead] = it;

                    const production& item_prod = grammar.ruleset[crule][cprod];

                    if (cpos >= item_prod.first.size() || item_prod.first[cpos] != j) {
                        continue;
                    }

                    slr::item lookup_item = {crule, cprod, cpos + 1};
                    unsigned int to_state = mapping[from][j];
                    auto to_item = std::find(items[to_state].begin(), items[to_state].end(), lookup_item);

                    if (to_item == items[to_state].end()) {
                        throw std::runtime_error("LR(0) sets computation error");
                    }

                    if (clookahead == -3) {
                        prop_table[{from, i}].insert({to_state, to_item - items[to_state].begin()});
                        continue;
                    }

                    table[{to_state, to_item - items[to_state].begin()}].insert(clookahead);
                }
            }
        }
    }

    pitems<clr::item> build_lalr_items(pitems<slr::item>& items, lookahead_table &table, const rules& grammar,
                                       const alphabet& symbols, symbol_props& first) {
        pitems<clr::item> citems(items.size());

        for (unsigned int i = 0; i < items.size(); ++i) {
            const std::vector<slr::item>& state = items[i];
            std::vector<clr::item> cstate;

            for (unsigned int j = 0; j < state.size(); ++j) {
                if (!slr::is_base(state[j])) {
                    break;
                }

                auto [rule, prod, pos] = state[j];

                for (int symbol : table[{i, j}]) {
                    cstate.push_back({rule, prod, pos, symbol});
                }
            }

            citems[i] = clr::closure(cstate, grammar, symbols, first);
        }

        return citems;
    }

    parsing_table build_lalr_table(const pitems<clr::item>& items, const rules& grammar, const alphabet& symbols,
                                   const set_mapping& mapping) {
        parsing_table table{items.size()};

        for (unsigned int i = 0; i < items.size(); ++i) {
            for (unsigned int j = 0; j < items[i].size(); ++j) {
                auto [rule, prod, pos, lookahead] = items[i][j];

                const production& item_prod = grammar.ruleset[rule][prod];

                parsing_action action{};
                int symbol;

                if (pos == item_prod.first.size()) {
                    symbol = lookahead;

                    if (rule == 0) {
                        action.type = parsing_action::action_type::ACCEPT;
                    } else {
                        action.type = parsing_action::action_type::REDUCE;
                        action.arg = (unsigned int) item_prod.first.size();
                    }
                } else {
                    symbol = item_prod.first[pos];

                    action.type = parsing_action::action_type::SHIFT;
                    action.arg = mapping[i][symbol];
                }

                insert(table, i, symbol, action, "Invalid LALR grammar"_u8);
            }
        }

        return table;
    }

    parsing_table build_table(const rules& grammar, const alphabet& symbols) {
        symbol_props first;
        set_mapping mapping;
        std::vector<std::vector<slr::item>> items = build_items(grammar, symbols, first, mapping);
        lookahead_producer producer{symbols, grammar, first, mapping, items};

        lookahead_table table{{{0, 0}, {-2}}};
        propagation_table prop_table;

        for (unsigned int i = 0; i < items.size(); ++i) {
            producer.get_lookahead(i, table, prop_table);
        }

        bool changes = true;
        while (changes) {
            changes = false;

            for (const auto& propagation : prop_table) {
                for (const auto& to : propagation.second) {
                    for (const auto& lookahead : table[propagation.first]) {
                        changes = changes || table[to].insert(lookahead).second;
                    }
                }
            }
        }

        pitems<clr::item> citems = build_lalr_items(items, table, grammar, symbols, first);

        return build_lalr_table(citems, grammar, symbols, mapping);
    }

}

#endif //ALIEN_PARSER_LALR_ALGORITHM_H