#include "parser/generator/lalr.h"

namespace alien::parser::generator {

    parsing_table lalr_generator::generate_table() {
        util::vecset<std::vector<slr::item>> slr_items = simple_helper.generate_slr_items();

        generate_lookahead(slr_items);

        bool changes = true;

        while (changes) {
            changes = false;

            for (const auto& from : propagation) {
                for (const auto& to : from.second) {
                    for (const auto& symbol : lookahead[from.first]) {
                        changes = lookahead[to].insert(symbol).second || changes;
                    }
                }
            }
        }

        util::vecset<std::set<clr::item>> lalr_items = build_lalr_items(slr_items);

        table.resize(lalr_items.size());

        for (std::size_t i = 0; i < lalr_items.size(); ++i) {
            for (const auto& lalr_item : lalr_items[i]) {
                auto [rule, production, pos, lookahead_symbol] = lalr_item;

                const rules::production& prod = rules.ruleset[rule][production];

                parsing_action action;
                rules::grammar_symbol symbol;

                if (pos == prod.symbols.size()) {
                    symbol = {rules::symbol_type::TERMINAL, lookahead_symbol};

                    action.arg1 = rule;
                    action.arg2 = production;
                    action.type = rule == 0 ? action_type::ACCEPT : action_type::REDUCE;
                } else {
                    symbol = prod.symbols[pos];

                    auto moved = simple_helper.slr_closure(simple_helper.slr_move(slr_items[i], symbol));

                    action.arg1 = slr_items.find(moved) - slr_items.vbegin();
                    action.type = action_type::SHIFT;
                }

                insert(i, symbol, action);
            }
        }

        return table;
    }

    void lalr_generator::generate_lookahead(const util::vecset<std::vector<slr::item>>& slr_items) {
        lookahead[slr_items[0].begin()].insert(-2);

        for (std::size_t i = 0; i < slr_items.size(); ++i) {
            for (auto it = slr_items[i].begin(); it != slr_items[i].end(); ++it) {
                auto& [rule, production, pos] = *it;

                if (pos == 0 && rule != 0) {
                    continue;
                }

                auto closure = canonical_helper.clr_closure({{rule, production, pos, -3}});

                for (const auto& clr_item : closure) {
                    auto [clr_rule, clr_production, clr_pos, clr_lookahead] = clr_item;

                    const rules::production& prod = rules.ruleset[clr_rule][clr_production];

                    if (clr_pos >= prod.symbols.size()) {
                        continue;
                    }

                    const auto& current_symbol = prod.symbols[clr_pos];

                    auto moved = simple_helper.slr_closure(simple_helper.slr_move(slr_items[i], current_symbol));
                    auto state = slr_items.find(moved);
                    auto slr_item = std::find(state->begin(), state->end(), slr::item{clr_rule, clr_production, clr_pos + 1});

                    if (clr_lookahead == -3) {
                        propagation[it].insert(slr_item);
                    } else {
                        lookahead[slr_item].insert(clr_lookahead);
                    }
                }
            }
        }
    }

    util::vecset<std::set<clr::item>> lalr_generator::build_lalr_items(const util::vecset<std::vector<slr::item>>& slr_items) {
        util::vecset<std::set<clr::item>> lalr_items;

        for (std::size_t i = 0; i < slr_items.size(); ++i) {
            lalr_items.push_back({});

            const auto& state = slr_items[i];
            std::set<clr::item> clr_state;

            for (auto it = state.begin(); it != state.end(); ++it) {
                auto& [rule, production, pos] = *it;

                if (pos == 0 && rule != 0) {
                    continue;
                }

                for (std::ptrdiff_t symbol : lookahead[it]) {
                    clr_state.emplace(rule, production, pos, symbol);
                }
            }

            lalr_items[i] = canonical_helper.clr_closure(clr_state);
        }

        return lalr_items;
    }

}