#include "parser/generator/slr.h"

namespace alien::parser::generator {

    util::vecset<std::vector<slr::item>> slr_helper::generate_slr_items() {
        util::vecset<std::vector<slr::item>> states{slr_closure({{0, 0, 0}})};

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

    std::vector<slr::item> slr_helper::slr_closure(const std::vector<slr::item>& items) {
        auto it = slr_closure_cache.find(items);

        if (it != slr_closure_cache.end()) {
            return it->second;
        }

        memset(non_terminal_productions, 0, rules.ruleset.size());

        std::vector<slr::item> closure = items;

        for (std::size_t i = 0; i < closure.size(); ++i) {
            auto [rule, production, pos] = closure[i];

            if (rule == 0) {
                non_terminal_productions[0] = true;
            }

            const rules::production& prod = rules.ruleset[rule][production];

            if (pos >= prod.symbols.size()) {
                continue;
            }

            const rules::grammar_symbol& symbol = prod.symbols[pos];

            if (symbol.type == rules::symbol_type::TERMINAL) {
                continue;
            }

            if (non_terminal_productions[symbol.index]) {
                continue;
            }

            for (std::size_t j = 0; j < rules.ruleset[symbol.index].size(); ++j) {
                closure.emplace_back(symbol.index, j, 0);
            }

            non_terminal_productions[symbol.index] = true;
        }

        return slr_closure_cache.insert({items, closure}).first->second;
    }

    std::vector<slr::item> slr_helper::slr_move(const std::vector<slr::item>& items, const rules::grammar_symbol& symbol) const {
        std::vector<slr::item> moved;

        for (const slr::item& item : items) {
            auto [rule, production, pos] = item;

            const rules::production& prod = rules.ruleset[rule][production];

            if (pos < prod.symbols.size() && prod.symbols[pos] == symbol) {
                moved.emplace_back(rule, production, pos + 1);
            }
        }

        return moved;
    }

    parsing_table slr_generator::generate_table() {
        build_follow();

        util::vecset<std::vector<slr::item>> states = helper.generate_slr_items();

        table.resize(states.size());

        for (std::size_t i = 0; i < states.size(); ++i) {
            for (const auto& item : states[i]) {
                auto [rule, production, pos] = item;

                const rules::production& prod = rules.ruleset[rule][production];

                if (pos < prod.symbols.size()) {
                    const rules::grammar_symbol& next_symbol = prod.symbols[pos];

                    auto next = helper.slr_closure(helper.slr_move(states[i], next_symbol));

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

    void slr_generator::build_follow() {
        bool changes = true;

        follow[0].insert(-2);

        while (changes) {
            changes = false;

            for (std::size_t i = 0; i < rules.ruleset.size(); ++i) {
                for (const rules::production& prod : rules.ruleset[i]) {
                    if (prod.symbols.empty()) {
                        continue;
                    }

                    for (std::size_t j = 0; j < prod.symbols.size(); ++j) {
                        const auto& symbol = prod.symbols[j];

                        if (symbol.type == rules::symbol_type::TERMINAL) {
                            continue;
                        }

                        const auto& parent_follow = follow[i];
                        auto next_first = helper.get_first(prod.symbols, j + 1);

                        auto it = next_first.find(-1);

                        if (j == prod.symbols.size() - 1 || it != next_first.end()) {
                            if (it != next_first.end()) {
                                next_first.erase(it);
                            }

                            for (auto terminal : parent_follow) {
                                changes = follow[symbol.index].insert(terminal).second || changes;
                            }
                        }

                        for (auto terminal : next_first) {
                            changes = follow[symbol.index].insert(terminal).second || changes;
                        }
                    }
                }
            }
        }
    }

}