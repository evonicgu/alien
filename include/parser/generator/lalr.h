#ifndef ALIEN_LALR_H
#define ALIEN_LALR_H

#include <set>
#include <tuple>

#include "clr.h"
#include "generator.h"
#include "parser/config/rules/rules.h"
#include "slr.h"
#include "util/vecset.h"

namespace alien::parser::generator {

    using lookahead_table = std::map<const slr::item*, std::set<std::ptrdiff_t>>;
    using propagation_table = std::map<const slr::item*, std::set<const slr::item*>>;

    class lalr_generator : public slr_generator, clr_generator {
        lookahead_table lookahead;
        propagation_table propagation;

    public:
        lalr_generator(alphabet::alphabet& alphabet, rules::rules& rules)
            : generator(alphabet, rules),
              slr_generator(alphabet, rules),
              clr_generator(alphabet, rules) {}

        parsing_table generate_table() override {
            util::vecset<std::set<slr::item>> slr_items = generate_slr_items();

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

                        auto moved = slr_closure(slr_move(slr_items[i], symbol));

                        action.arg1 = slr_items.find(moved) - slr_items.vbegin();
                        action.type = action_type::SHIFT;
                    }

                    insert(i, symbol, action);
                }
            }

            return table;
        }

    private:
        void generate_lookahead(const util::vecset<std::set<slr::item>>& slr_items) {
            lookahead[&*slr_items[0].begin()].insert(-2);

            for (std::size_t i = 0; i < slr_items.size(); ++i) {
                for (const auto& item: slr_items[i]) {
                    auto[rule, production, pos] = item;

                    if (pos == 0 && rule != 0) {
                        continue;
                    }

                    auto closure = clr_closure({{rule, production, pos, -3}});

                    for (const auto& clr_item: closure) {
                        auto[clr_rule, clr_production, clr_pos, clr_lookahead] = clr_item;

                        const rules::production& prod = rules.ruleset[clr_rule][clr_production];

                        if (clr_pos >= prod.symbols.size()) {
                            continue;
                        }

                        const auto& current_symbol = prod.symbols[clr_pos];

                        auto moved = slr_closure(slr_move(slr_items[i], current_symbol));
                        auto state = slr_items.find(moved);
                        auto slr_item = state->find({clr_rule, clr_production, clr_pos + 1});

                        if (clr_lookahead == -3) {
                            propagation[&item].insert(&*slr_item);
                        } else {
                            lookahead[&*slr_item].insert(clr_lookahead);
                        }
                    }
                }
            }
        }

        util::vecset<std::set<clr::item>> build_lalr_items(const util::vecset<std::set<slr::item>>& slr_items) {
            util::vecset<std::set<clr::item>> lalr_items;

            for (std::size_t i = 0; i < slr_items.size(); ++i) {
                lalr_items.push_back({});

                const auto& state = slr_items[i];
                std::set<clr::item> clr_state;

                for (const auto& item : state) {
                    auto [rule, production, pos] = item;

                    if (pos == 0 && rule != 0) {
                        continue;
                    }

                    for (std::ptrdiff_t symbol : lookahead[&item]) {
                        clr_state.emplace(rule, production, pos, symbol);
                    }
                }

                lalr_items[i] = clr_closure(clr_state);
            }

            return lalr_items;
        }
    };

}

#endif //ALIEN_LALR_H