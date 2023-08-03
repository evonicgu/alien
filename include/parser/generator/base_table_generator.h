#ifndef ALIEN_PARSER_TABLE_GENERATOR_H
#define ALIEN_PARSER_TABLE_GENERATOR_H

#include <cstdlib>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "nlohmann/json.hpp"

#include "alphabet.h"
#include "base_helper.h"
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

        bool operator==(const parsing_action& other) const;

        bool operator!=(const parsing_action& other) const;

        bool operator<(const parsing_action& other) const;
    };

    void to_json(nlohmann::json& json, const parsing_action& action);

    using parsing_table = std::vector<std::map<rules::grammar_symbol, parsing_action>>;

    using table_transform_result = std::vector<std::vector<std::pair<parser::generator::parsing_action, std::vector<std::ptrdiff_t>>>>;

    table_transform_result transform_table(const parser::generator::parsing_table& table, std::ptrdiff_t terminals);

    class base_table_generator {
    protected:
        symbol_props first, follow;

        parsing_table table;

        alphabet::alphabet& alphabet;
        rules::rules& rules;

    public:
        virtual parsing_table generate_table() = 0;

        base_table_generator(alphabet::alphabet& alphabet, rules::rules& rules)
                : alphabet(alphabet),
                  rules(rules) {
            first.resize(alphabet.non_terminals.size());
            follow.resize(alphabet.non_terminals.size());
        }

        virtual ~base_table_generator() = default;

    protected:
        void insert(std::size_t state, rules::grammar_symbol symbol, parsing_action action);
    };
}

#endif //ALIEN_PARSER_TABLE_GENERATOR_H