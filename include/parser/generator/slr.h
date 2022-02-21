#ifndef ALIEN_SLR_H
#define ALIEN_SLR_H

#include <map>
#include <set>
#include <tuple>

#include "generator.h"
#include "parser/config/rules/rules.h"
#include "util/vecset.h"

namespace alien::parser::generator {

    namespace slr {

        using item = std::tuple<std::size_t, std::size_t, std::size_t>;

    }

    class slr_generator : public virtual generator {
        std::map<std::set<slr::item>, std::set<slr::item>> cache;

    public:
        slr_generator(alphabet::alphabet& alphabet, rules::rules& rules)
            : generator(alphabet, rules) {}

        parsing_table generate_table() override {
            build_follow();

            util::vecset<std::set<slr::item>> states = generate_slr_items();

            table.resize(states.size());

            for (std::size_t i = 0; i < states.size(); ++i) {
                for (const auto& item : states[i]) {
                    auto [rule, production, pos] = item;

                    const rules::production& prod = rules.ruleset[rule][production];

                    if (pos < prod.symbols.size()) {
                        const rules::grammar_symbol& next_symbol = prod.symbols[pos];

                        auto next = slr_closure(slr_move(states[i], next_symbol));

                        std::size_t next_index = states.find(next) - states.vbegin();

                        insert(i, next_symbol, {action_type::SHIFT, next_index});

                        continue;
                    }

                    parsing_action action{
                            action_type::REDUCE,
                            rule,
                            production
                    };

                    if (rule == rules.start) {
                        action.type = action_type::ACCEPT;

                        insert(i, {rules::symbol_type::TERMINAL, -2}, action);
                        continue;
                    }

                    for (std::ptrdiff_t symbol : follow[rule]) {
                        insert(i, {rules::symbol_type::TERMINAL, symbol}, action);
                    }
                }
            }

            return table;
        }

    protected:
        util::vecset<std::set<slr::item>> generate_slr_items() {
            util::vecset<std::set<slr::item>> states{slr_closure({{0, 0, 0}})};

            for (std::size_t i = 0; i < states.size(); ++i) {
                for (std::ptrdiff_t j = 1; j < alphabet.non_terminals.size(); ++j) {
                    auto next = slr_closure(slr_move(states[i], {rules::symbol_type::NON_TERMINAL, j}));

                    if (next.empty()) {
                        continue;
                    }

                    states.push_back(std::move(next));
                }

                for (std::ptrdiff_t j = 0; j < alphabet.terminals.size(); ++j) {
                    auto next = slr_closure(slr_move(states[i], {rules::symbol_type::TERMINAL, j}));

                    if (next.empty()) {
                        continue;
                    }

                    states.push_back(std::move(next));
                }
            }

            return states;
        }

        std::set<slr::item> slr_closure(const std::set<slr::item>& items) {
            auto it = cache.find(items);

            if (it != cache.end()) {
                return it->second;
            }

            util::vecset<slr::item> closure{items};

            for (std::size_t i = 0; i < closure.size(); ++i) {
                auto [rule, production, pos] = closure[i];

                const rules::production& prod = rules.ruleset[rule][production];

                if (pos >= prod.symbols.size()) {
                    continue;
                }

                const rules::grammar_symbol& symbol = prod.symbols[pos];

                if (symbol.type == rules::symbol_type::TERMINAL) {
                    continue;
                }

                for (std::size_t j = 0; j < rules.ruleset[symbol.index].size(); ++j) {
                    closure.push_back({symbol.index, j, 0});
                }
            }

            return cache.insert({items, (std::set<slr::item>) closure}).first->second;
        }

        std::set<slr::item> slr_move(const std::set<slr::item>& items, const rules::grammar_symbol& symbol) const {
            std::set<slr::item> moved;

            for (const slr::item& item : items) {
                auto [rule, production, pos] = item;

                const rules::production& prod = rules.ruleset[rule][production];

                if (pos < prod.symbols.size() && prod.symbols[pos] == symbol) {
                    moved.emplace(rule, production, pos + 1);
                }
            }

            return moved;
        }
    };

}

#endif //ALIEN_SLR_H