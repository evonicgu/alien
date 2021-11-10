#ifndef ALIEN_PARSER_CONFIG_RULES_TOKEN_H
#define ALIEN_PARSER_CONFIG_RULES_TOKEN_H

#include "generalized/generalized_token.h"
#include "util/u8string.h"

namespace alien::parser::config::rules::lexer {

    enum class token_type {
        T_SEMICOLON,
        T_COLON,
        T_CODE_BLOCK,
        T_END,
        T_IDENTIFIER,
        T_TERMINAL,
        T_OR
    };

    using base_token = generalized::generalized_token<token_type>;

    struct identifier_token : base_token {
        util::u8string name;

        explicit identifier_token(const util::u8string& name) : name(name), base_token(token_type::T_IDENTIFIER) {}

        explicit identifier_token(util::u8string&& name) : name(std::move(name)),
                                                                   base_token(token_type::T_IDENTIFIER) {}
    };

    struct terminal_token : base_token {
        util::u8string name;

        explicit terminal_token(const util::u8string& name) : name(name), base_token(token_type::T_TERMINAL) {}

        explicit terminal_token(util::u8string&& name) : name(std::move(name)), base_token(token_type::T_TERMINAL) {}
    };

    struct code_token : base_token {
        util::u8string code;

        explicit code_token(const util::u8string& code) : code(code), base_token(token_type::T_CODE_BLOCK) {}

        explicit code_token(util::u8string&& code) : code(code), base_token(token_type::T_CODE_BLOCK) {}
    };

}

#endif //ALIEN_PARSER_CONFIG_RULES_TOKEN_H