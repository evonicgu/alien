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
        if (!cache[symbol].empty()) {
            return cache[symbol];
        }

        if (symbol < 0 || symbols[symbol].type == grammar_symbol::symbol_type::TERMINAL) {
            return cache[symbol] = {symbol};
        }

        // left-recursive rules
        std::vector<unsigned int> lr_rules;

        for (unsigned int i = 0; i < grammar.ruleset[symbol].size(); ++i) {
            const production& prod = grammar.ruleset[symbol][i];

            if (prod.first.empty()) {
                cache[symbol].insert(-1);
                continue;
            }

            if (prod.first[0] == symbol) {
                lr_rules.push_back(i);
                continue;
            }

            get_first(prod.first, grammar, symbols, cache, cache[symbol]);
        }

        if (cache[symbol].find(-1) != cache[symbol].end()) {
            /* In this case FIRST(A) contains -1 (epsilon)
             *
             * This is computed using non-left-recursive productions only
             * And this means, that all left-recursive productions (A -> A...)
             * can actually be A -> ..., i.e. without A in the front, which
             * is replaced by -1 (epsilon)
             *
             * So FIRST(A) = previous FIRST(A) + FIRST(A -> ...) + FIRST(A -> ...) + ...
             */

            for (unsigned int i = 0; i < lr_rules.size(); ++i) {
                const production& prod = grammar.ruleset[symbol][i];

                bool nullable = true;

                for (unsigned int j = 1; j < prod.first.size() && nullable; ++i) {
                    const std::set<int>& next_first = get_first(prod.first[j], grammar, symbols, cache);

                    if (next_first.find(-1) == next_first.end()) {
                        nullable = false;
                    }

                    for (int id : next_first) {
                        cache[symbol].insert(id);
                    }
                }
            }
        }

        /*
         * All "A" productions are left-recursive, e.g. A -> A... | A... | ...
         *
         * This rule specification is invalid
         */
        if (cache[symbol].empty()) {
            throw parser::parser::invalid_grammar_exception("Invalid rule productions specification"_u8);
        }

        return cache[symbol];
    }

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
                if (id == -1) {
                    continue;
                }

                first.insert(id);
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

            for (unsigned int i = 0; i < grammar.ruleset.size(); ++i) {
                for (const production& prod : grammar.ruleset[i]) {
                    if (prod.first.empty()) {
                        continue;
                    }

                    std::set<int> next_first = follow[i];

                    for (int id : next_first) {
                        changes = changes || follow[prod.first[prod.first.size() - 1]].insert(id).second;
                    }

                    for (unsigned int j = prod.first.size() - 1; j >= 1; --j) {
                        const std::set<int>& current_first = get_first(prod.first[j], grammar, symbols, first);

                        if (current_first.find(-1) == current_first.end()) {
                            next_first = current_first;
                        } else {
                            for (int id : current_first) {
                                next_first.insert(id);
                            }
                        }

                        for (int id : next_first) {
                            changes = changes || follow[prod.first[j - 1]].insert(id).second;
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

        unsigned int arg;

        bool operator==(const parsing_action& other) const {
            return type == other.type && arg == other.arg;
        }

        bool operator!=(const parsing_action& other) const {
            return !operator==(other);
        }
    };

    using parsing_table = std::vector<std::unordered_map<int, parsing_action>>;

    void insert(parsing_table& table, unsigned int state, int symbol, parsing_action action, util::u8string&& str) {
        if (table[state].find(symbol) != table[state].end() && table[state][symbol] != action) {
            throw parser::parser::invalid_grammar_exception(str);
        }

        table[state][symbol] = action;
    }

}

#endif //ALIEN_PARSER_ALGORITHM_H