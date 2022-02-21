#ifndef ALIEN_CLR_H
#define ALIEN_CLR_H

#include <map>
#include <set>
#include <tuple>

#include "generator.h"
#include "parser/config/rules/rules.h"
#include "util/vecset.h"

namespace alien::parser::generator {

    namespace clr {

        using item = std::tuple<std::size_t, std::size_t, std::size_t, std::ptrdiff_t>;

    }

    class clr_generator : public virtual generator {
        std::map<std::set<clr::item>, std::set<clr::item>> cache;

    public:
        clr_generator(alphabet::alphabet& alphabet, rules::rules& rules)
                : generator(alphabet, rules) {}

        parsing_table generate_table() override {
            util::vecset<std::set<clr::item>> states{clr_closure({{0, 0, 0, -2}})};

            table.resize(1);

            for (std::size_t i = 0; i < states.size(); ++i) {
                for (const auto& item : states[i]) {
                    auto [rule, production, pos, lookahead] = item;

                    const rules::production& prod = rules.ruleset[rule][production];

                    if (pos < prod.symbols.size()) {
                        continue;
                    }

                    parsing_action action{
                            action_type::REDUCE,
                            rule,
                            production
                    };

                    if (rule == 0) {
                        action.type = action_type::ACCEPT;
                    }

                    insert(i, {rules::symbol_type::TERMINAL, lookahead}, action);
                }

                for (std::ptrdiff_t j = 1; j < alphabet.non_terminals.size(); ++j) {
                    auto next = clr_closure(clr_move(states[i], {rules::symbol_type::NON_TERMINAL, j}));

                    if (next.empty()) {
                        continue;
                    }

                    insert(i, {rules::symbol_type::NON_TERMINAL, j}, {action_type::SHIFT, clr_transition(states, next)});
                }

                for (std::ptrdiff_t j = 0; j < alphabet.terminals.size(); ++j) {
                    auto next = clr_closure(clr_move(states[i], {rules::symbol_type::TERMINAL, j}));

                    if (next.empty()) {
                        continue;
                    }

                    insert(i, {rules::symbol_type::TERMINAL, j}, {action_type::SHIFT, clr_transition(states, next)});
                }
            }

            return table;
        }

    protected:
        std::size_t clr_transition(util::vecset<std::set<clr::item>>& states, std::set<clr::item>& next) {
            std::size_t to_state = states.push_back(std::move(next));

            if (to_state >= table.size()) {
                table.emplace_back();
            }

            return to_state;
        }

        std::set<clr::item> clr_closure(const std::set<clr::item>& items) {
            auto it = cache.find(items);

            if (it != cache.end()) {
                return it->second;
            }

            util::vecset<clr::item> closure{items};

            for (std::size_t i = 0; i < closure.size(); ++i) {
                auto [rule, production, pos, lookahead] = closure[i];

                const rules::production& prod = rules.ruleset[rule][production];

                if (pos >= prod.symbols.size()) {
                    continue;
                }

                const rules::grammar_symbol& symbol = prod.symbols[pos];

                if (symbol.type == rules::symbol_type::TERMINAL) {
                    continue;
                }

                std::vector<rules::grammar_symbol> str{prod.symbols.begin(), prod.symbols.end()};
                str.push_back({rules::symbol_type::TERMINAL, lookahead});

                auto first = get_first(str, pos + 1);

                for (std::size_t j = 0; j < rules.ruleset[symbol.index].size(); ++j) {
                    for (std::ptrdiff_t s : first) {
                        closure.push_back({symbol.index, j, 0, s});
                    }
                }
            }

            return cache.insert({items, (std::set<clr::item>) closure}).first->second;
        }

        std::set<clr::item> clr_move(const std::set<clr::item>& items, const rules::grammar_symbol& symbol) {
            std::set<clr::item> moved;

            for (const clr::item& item : items) {
                auto [rule, production, pos, lookahead] = item;

                const rules::production& prod = rules.ruleset[rule][production];

                if (pos < prod.symbols.size() && prod.symbols[pos] == symbol) {
                    moved.emplace(rule, production, pos + 1, lookahead);
                }
            }

            return moved;
        }
    };

}

#endif //ALIEN_CLR_H