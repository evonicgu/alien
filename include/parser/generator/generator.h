#ifndef ALIEN_PARSER_GENERATOR_H
#define ALIEN_PARSER_GENERATOR_H

#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "nlohmann/json.hpp"

#include "alphabet.h"
#include "parser/config/rules/rules.h"

namespace alien::parser::generator {

    enum class action_type {
        SHIFT,
        REDUCE,
        ACCEPT
    };

    struct parsing_action {
        action_type type;

        std::size_t arg1 = 0, arg2 = 0;

        bool operator==(const parsing_action& other) const {
            return type == other.type && arg1 == other.arg1 && arg2 == other.arg2;
        }

        bool operator!=(const parsing_action& other) const {
            return !operator==(other);
        }

        bool operator<(const parsing_action& other) const {
            if (type == other.type) {
                if (arg1 == other.arg1) {
                    return arg2 < other.arg2;
                }

                return arg1 < other.arg1;
            }

            return type < other.type;
        }
    };

    void to_json(nlohmann::json& json, const parsing_action& action) {
        json["type"] = action.type;
        json["arg1"] = action.arg1;
        json["arg2"] = action.arg2;
    }

    using parsing_table = std::vector<std::map<rules::grammar_symbol, parsing_action>>;

    auto transform_table(const parser::generator::parsing_table& table, std::ptrdiff_t terminals) {
        std::vector<std::vector<std::pair<parser::generator::parsing_action, std::vector<std::ptrdiff_t>>>> json;

        json.resize(table.size());

        for (std::size_t i = 0; i < table.size(); ++i) {
            std::map<parser::generator::parsing_action, std::vector<std::ptrdiff_t>> indices;

            for (auto& transition : table[i]) {
                auto& [symbol, action] = transition;

                using t = parser::rules::symbol_type;

                std::ptrdiff_t index = symbol.index + (symbol.type == t::NON_TERMINAL) * terminals;
                indices[action].push_back((std::ptrdiff_t) index);
            }

            for (auto& index : indices) {
                json[i].emplace_back(index.first, std::move(index.second));
            }
        }

        return json;
    }

    class generator {
    protected:
        using symbol_props = std::vector<std::unordered_set<std::ptrdiff_t>>;

        parsing_table table;
        symbol_props first, follow;

        alphabet::alphabet& alphabet;
        rules::rules& rules;

        void insert(std::size_t state, rules::grammar_symbol symbol, parsing_action action) {
            auto it = table[state].find(symbol);

            if (it == table[state].end()) {
                table[state].insert({symbol, action});
                return;
            }

            if (it->second == action) {
                return;
            }

            if (it->second.type == action.type) {
                throw std::runtime_error("Unable to resolve grammar conflict");
            }

            auto shift_action = it->second.type == action_type::SHIFT ? it->second : action;
            auto reduce_action = it->second.type == action_type::REDUCE ? it->second : action;

            const auto& prod = rules.ruleset[reduce_action.arg1][reduce_action.arg2];
            std::ptrdiff_t prec, assoc;

            prec = alphabet.terminals[symbol.index].prec;
            assoc = alphabet.terminals[symbol.index].assoc;

            if (prec == -1 || prod.prec == -1) {
                throw std::runtime_error("Unable to resolve Shift/Reduce conflict");
            }

            if (prec > prod.prec) {
                table[state][symbol] = shift_action;
            } else if (prec == prod.prec) {
                if (assoc == -1) {
                    throw std::runtime_error("Associativity conflict cannot be resolved");
                }

                if (assoc == -2) {
                    table[state].erase(table[state].find(symbol));
                    return;
                }

                if (prod.assoc == 0) {
                    table[state][symbol] = reduce_action;
                } else {
                    table[state][symbol] = shift_action;
                }
            } else {
                table[state][symbol] = reduce_action;
            }
        }

        const std::unordered_set<std::ptrdiff_t>& get_first(std::size_t symbol) {
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

        std::unordered_set<std::ptrdiff_t> get_first(const std::vector<rules::grammar_symbol>& str, std::size_t start_index = 0) {
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

        void build_follow() {
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
                            auto next_first = get_first(prod.symbols, j + 1);

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

    public:
        virtual parsing_table generate_table() = 0;

        generator(alphabet::alphabet& alphabet, rules::rules& rules)
                : alphabet(alphabet),
                  rules(rules) {
            first.resize(alphabet.non_terminals.size());
            follow.resize(alphabet.non_terminals.size());
        }
    };
}

#endif //ALIEN_PARSER_GENERATOR_H