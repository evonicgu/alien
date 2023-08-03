#ifndef ALIEN_PARSER_RULES_TOKEN_H
#define ALIEN_PARSER_RULES_TOKEN_H

#include "util/token.h"
#include "util/u8string.h"

namespace alien::parser::rules {

    enum class token_type {
        T_IDENTIFIER,
        T_COLON,
        T_OR,
        T_TERMINAL,
        T_ERROR,
        T_MIDRULE_ACTION,
        T_PREC,
        T_ACTION,
        T_SEMICOLON,
        T_END
    };

    using base_token = util::token<token_type>;

    struct identifier_token : public base_token {
        util::u8string name;

        identifier_token(util::u8string&& name, util::pos start, util::pos end)
            : name(std::move(name)),
              base_token(token_type::T_IDENTIFIER, start, end) {}
    };

    struct terminal_token : public base_token {
        util::u8string name;

        terminal_token(util::u8string&& name, util::pos start, util::pos end)
            : name(std::move(name)),
                  base_token(token_type::T_TERMINAL, start, end) {}
    };

    struct midrule_action_token : public base_token {
        util::u8string code, type;

        midrule_action_token(util::u8string&& code, util::u8string&& type, util::pos start, util::pos end)
            : code(std::move(code)),
              type(std::move(type)),
              base_token(token_type::T_MIDRULE_ACTION, start, end) {}
    };

    struct action_token : public base_token {
        util::u8string code;

        action_token(util::u8string&& code, util::pos start, util::pos end)
            : code(std::move(code)),
              base_token(token_type::T_ACTION, start, end) {}
    };

}

#endif //ALIEN_PARSER_RULES_TOKEN_H