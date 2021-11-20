#ifndef ALIEN_PARSER_ALGORITHM_H
#define ALIEN_PARSER_ALGORITHM_H

#include <set>
#include <variant>
#include <vector>
#include <unordered_map>
#include "parser/config/rules/rules.h"
#include "parser/config/rules/rules_parser.h"

namespace alien::parser::algorithm {

    using symbol_props = std::unordered_map<int, std::set<int>>;

    using namespace alien::parser::config::rules;
    using namespace util::literals;

    std::set<int> get_first(const std::vector<int>&, const rules&, const alphabet&, symbol_props&);
    void get_first(const std::vector<int>&, const rules&, const alphabet&, symbol_props&, std::set<int>&);

    const std::set<int>& get_first(int symbol, const rules& grammar, const alphabet& symbols, symbol_props& cache) {
        auto it = cache.find(symbol);

        if (it != cache.end()) {
            return cache[symbol];
        }

        if (symbol < 0 || symbols[symbol].type == grammar_symbol::symbol_type::TERMINAL) {
            return cache[symbol] = {symbol};
        }

        cache[symbol] = {};

        // left-recursive rules
        std::vector<std::pair<unsigned int, unsigned int>> lr_rules;

        for (unsigned int i = 0; i < grammar.ruleset.at(symbol).size(); ++i) {
            const production& prod = grammar.ruleset.at(symbol)[i];

            if (prod.symbols.empty()) {
                cache[symbol].insert(-1);
                continue;
            }

            if (prod.symbols[0] == symbol) {
                lr_rules.push_back({i, 1});
                continue;
            }

            bool nullable = true;

            for (unsigned int j = 0; j < prod.symbols.size() && nullable; ++j) {
                if (prod.symbols[j] == symbol) {
                    nullable = false;
                    lr_rules.push_back({i, j + 1});
                    break;
                }

                const std::set<int>& next_first = get_first(prod.symbols[j], grammar, symbols, cache);

                if (next_first.find(-1) == next_first.end()) {
                    nullable = false;
                }

                for (int id : next_first) {
                    if (id != -1)
                        cache[symbol].insert(id);
                }
            }

            if (nullable) {
                cache[symbol].insert(-1);
            }
        }

        if (lr_rules.size() == grammar.ruleset.at(symbol).size()) {
            throw parser::parser::invalid_grammar_exception("Invalid rule productions specification"_u8);
        }

        if (cache[symbol].find(-1) != cache[symbol].end()) {
            for (unsigned int i = 0; i < lr_rules.size(); ++i) {
                const production& prod = grammar.ruleset.at(symbol)[lr_rules[i].first];

                bool nullable = true;

                for (unsigned int j = lr_rules[i].second; j < prod.symbols.size() && nullable; ++i) {
                    const std::set<int>& next_first = get_first(prod.symbols[j], grammar, symbols, cache);

                    if (next_first.find(-1) == next_first.end()) {
                        nullable = false;
                    }

                    for (int id : next_first) {
                        cache[symbol].insert(id);
                    }
                }
            }
        }

        return cache[symbol];
    }

//    const std::set<int>& get_first(int symbol, const rules& grammar, const alphabet& symbols, symbol_props& cache) {
//        std::set<int> callstack;
//
//        return get_first(symbol, grammar, symbols, cache, callstack);
//    }

    void get_first(const std::vector<int>& str, const rules& grammar, const alphabet& symbols,
                            symbol_props& cache, std::set<int>& first) {
        if (str.empty()) {
            first.insert(-1);
            return;
        }

        bool nullable = true;

        for (unsigned int j = 0; j < str.size() && nullable; ++j) {
            const std::set<int>& next_first = get_first(str[j], grammar, symbols, cache);

            if (next_first.find(-1) == next_first.end()) {
                nullable = false;
            }

            for (int id : next_first) {
                if (id != -1) {
                    first.insert(id);
                }
            }
        }

        if (nullable) {
            first.insert(-1);
        }
    }

    std::set<int> get_first(const std::vector<int>& str, const rules& grammar, const alphabet& symbols,
                            symbol_props& cache) {
        std::set<int> first;

        get_first(str, grammar, symbols, cache, first);

        return first;
    }

    void build_follow(const rules& grammar, const alphabet& symbols, symbol_props& follow, symbol_props& first) {
        bool changes = true;

        follow[0].insert(-2);

        while (changes) {
            changes = false;

            for (const auto& nterm : grammar.ruleset) {
                for (const production& prod : nterm.second) {
                    if (prod.symbols.empty()) {
                        continue;
                    }

                    std::set<int> next_first = follow[nterm.first];

                    for (int id : next_first) {
                        changes = changes || follow[prod.symbols[prod.symbols.size() - 1]].insert(id).second;
                    }

                    for (unsigned int j = prod.symbols.size() - 1; j >= 1; --j) {
                        const std::set<int>& current_first = get_first(prod.symbols[j], grammar, symbols, first);

                        if (current_first.find(-1) == current_first.end()) {
                            next_first = current_first;
                        } else {
                            for (int id : current_first) {
                                next_first.insert(id);
                            }
                        }

                        for (int id : next_first) {
                            changes = changes || follow[prod.symbols[j - 1]].insert(id).second;
                        }
                    }
                }
            }
        }
    }

    struct parsing_action {
        enum class action_type {
            REDUCE,
            SHIFT,
            ACCEPT
        } type;

        unsigned int arg1 = 0, arg2 = 0;

        bool operator==(const parsing_action& other) const {
            return type == other.type && arg1 == other.arg1 && arg2 == other.arg2;
        }

        bool operator!=(const parsing_action& other) const {
            return !operator==(other);
        }
    };

    using parsing_table = std::vector<std::unordered_map<int, parsing_action>>;

    void insert(parsing_table& table, unsigned int state, int symbol, parsing_action action, const alphabet& symbols,
                const rules& grammar) {
        auto it = table[state].find(symbol);

        using type = parsing_action::action_type;

        if (it != table[state].end()) {
            if (it->second == action) {
                return;
            }

            if (it->second.type == type::REDUCE && action.type == type::REDUCE) {
                // Reduce/Reduce conflict
                throw parser::parser::invalid_grammar_exception("Reduce/Reduce conflict"_u8);
            }

            /* Shift/Reduce conflict. Try to solve it using precedences
             * This type of conflict can happen only with symbol being a terminal
             * If the rule or the symbol do not have precedence, i.e. rule has no
             * terminals in its body, or the last terminal has no precedence, then
             * the grammar is considered invalid
             */
            const auto& shift_action = it->second.type == type::REDUCE ? action : it->second;
            const auto& reduce_action = it->second.type == type::REDUCE ? it->second : action;

            const production& prod = grammar.ruleset.at(reduce_action.arg1)[reduce_action.arg2];
            const auto& symbol_info = symbols[symbol];

            if (symbol_info.prec == -1 || prod.prec == -1) {
                throw parser::parser::invalid_grammar_exception("Shift/Reduce conflict cannot be resolved"_u8);
            }

            if (symbol_info.prec > prod.prec) {
                table[state][symbol] = shift_action;
            } else if (symbol_info.prec == prod.prec) {
                if (symbol_info.assoc == -1 || prod.assoc == -1 || symbol_info.assoc != prod.assoc) {
                    throw parser::parser::invalid_grammar_exception("Associativity conflict cannot be resolved"_u8);
                }

                if (prod.assoc == 0) {
                    table[state][symbol] = reduce_action;
                } else {
                    table[state][symbol] = shift_action;
                }
            } else {
                table[state][symbol] = reduce_action;
            }

            return;
        }

        table[state][symbol] = action;
    }

}

#endif //ALIEN_PARSER_ALGORITHM_H