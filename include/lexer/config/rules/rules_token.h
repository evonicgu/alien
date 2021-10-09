#ifndef ALIEN_LEXER_CONFIG_RULES_TOKEN_H
#define ALIEN_LEXER_CONFIG_RULES_TOKEN_H

#include <string>
#include "generalized/generalized_token.h"

namespace alien::lexer::config::rules::lexer {

    enum class token_type {
        T_SEMICOLON,
        T_COLON,
        T_REGULAR_EXPRESSION,
        T_CODE_BLOCK,
        T_END
    } type;

    using base_token = generalized::generalized_token<token_type>;

    struct regex_token : public base_token {
        std::string regex;

        explicit regex_token(std::string&& regex) : regex(std::move(regex)), base_token(token_type::T_REGULAR_EXPRESSION) {}
    };

    struct tr_data {
        bool returned = false;

        std::string name;
    };

    struct action_token : public base_token {
        std::string code;

        tr_data tr;

        explicit action_token(std::string&& code) : code(std::move(code)), base_token(token_type::T_CODE_BLOCK) {
            tr.returned = false;
        }

        explicit action_token(std::string&& code, tr_data&& tr) : code(std::move(code)), tr(std::move(tr)),
                                                                  base_token(token_type::T_CODE_BLOCK) {}
    };
}

#endif //ALIEN_LEXER_CONFIG_RULES_TOKEN_H