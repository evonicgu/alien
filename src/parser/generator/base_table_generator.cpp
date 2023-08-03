#include "parser/generator/base_table_generator.h"

namespace alien::parser::generator {

    bool parsing_action::operator==(const parsing_action& other) const {
        return type == other.type && arg1 == other.arg1 && arg2 == other.arg2;
    }

    bool parsing_action::operator!=(const parsing_action& other) const {
        return !operator==(other);
    }

    bool parsing_action::operator<(const parsing_action& other) const {
        if (type == other.type) {
            if (arg1 == other.arg1) {
                return arg2 < other.arg2;
            }

            return arg1 < other.arg1;
        }

        return type < other.type;
    }

    void to_json(nlohmann::json& json, const parsing_action& action) {
        json["type"] = action.type;
        json["arg1"] = action.arg1;
        json["arg2"] = action.arg2;
    }

    table_transform_result transform_table(const parsing_table& table, std::ptrdiff_t terminals) {
        std::vector<std::vector<std::pair<parser::generator::parsing_action, std::vector<std::ptrdiff_t>>>> json;

        json.resize(table.size());

        for (std::size_t i = 0; i < table.size(); ++i) {
            std::map<parser::generator::parsing_action, std::vector<std::ptrdiff_t>> indices;

            for (auto& transition: table[i]) {
                auto& [symbol, action] = transition;

                using t = parser::rules::symbol_type;

                std::ptrdiff_t index = symbol.index + (symbol.type == t::NON_TERMINAL) * terminals;
                indices[action].push_back((std::ptrdiff_t) index);
            }

            for (auto& index: indices) {
                json[i].emplace_back(index.first, std::move(index.second));
            }
        }

        return json;
    }

    void base_table_generator::insert(std::size_t state, rules::grammar_symbol symbol, parsing_action action) {
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

}