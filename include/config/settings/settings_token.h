#ifndef ALIEN_CONFIG_SETTINGS_TOKEN_H
#define ALIEN_CONFIG_SETTINGS_TOKEN_H

#include <string>
#include "generalized/generalized_token.h"

namespace alien::lexer::config::settings::lexer {

    enum class token_type {
        T_HASHTAG,
        T_IDENTIFIER,
        T_COMMA,
        T_EQUALS,
        T_DOT,
        T_OPEN_BRACE,
        T_CLOSE_BRACE,
        T_NUMBER,
        T_STR,
        T_END,
        T_BOOL
    } type;

    using base_token = generalized::generalized_token<token_type>;

    struct identifier_token : public base_token {
        std::string name;

        explicit identifier_token(const std::string& name) : name(name), base_token(token_type::T_IDENTIFIER) {}

        explicit identifier_token(std::string&& name) : name(std::move(name)), base_token(token_type::T_IDENTIFIER) {}
    };

    struct number_token : public base_token {
        int number;

        explicit number_token(int number) : number(number), base_token(token_type::T_NUMBER) {}
    };

    struct str_token : public base_token {
        std::string str;

        explicit str_token(const std::string& str) : str(str), base_token(token_type::T_STR) {}

        explicit str_token(std::string&& str) : str(std::move(str)), base_token(token_type::T_STR) {}
    };

    struct bool_token : public base_token {
        bool value;

        explicit bool_token(bool value) : value(value), base_token(token_type::T_BOOL) {}
    };

}

#endif //ALIEN_CONFIG_SETTINGS_TOKEN_H