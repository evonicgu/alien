#ifndef ALIEN_SETTINGS_TOKEN_H
#define ALIEN_SETTINGS_TOKEN_H

#include "util/token.h"
#include "util/u8string.h"

namespace alien::config::settings {

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
        T_BOOL,
        T_SPECIFIER,
        T_CODE
    };

    using base = util::token<token_type>;

    struct identifier_token : public base {
        util::u8string name;

        identifier_token(util::u8string&& name, util::pos start, util::pos end)
            : name(std::move(name)),
              base(token_type::T_IDENTIFIER, start, end) {}
    };

    struct number_token : public base {
        long long number;

        number_token(long long number, util::pos start, util::pos end)
            : number(number),
              base(token_type::T_NUMBER, start, end) {}
    };

    struct str_token : public base {
        util::u8string str;

        str_token(util::u8string&& str, util::pos start, util::pos end)
            : str(std::move(str)),
              base(token_type::T_STR, start, end) {}
    };

    struct bool_token : public base {
        bool value;

        bool_token(bool value, util::pos start, util::pos end)
            : value(value),
              base(token_type::T_BOOL, start, end) {}
    };

    struct specifier_token : public base {
        util::u8string name;

        specifier_token(util::u8string&& name, util::pos start, util::pos end)
            : name(std::move(name)),
              base(token_type::T_SPECIFIER, start, end) {}
    };

    struct code_token : public base {
        util::u8string code;

        enum class location {
            HEADERS,
            DECL,
            IMPL,
            CONTENT_DECL_PUBLIC,
            CONTENT_DECL_PRIVATE,
            CONTENT_IMPL
        } loc;

        code_token(util::u8string&& code, location loc, util::pos start, util::pos end)
            : code(std::move(code)),
              loc(loc),
              base(token_type::T_CODE, start, end) {}
    };

}

#endif //ALIEN_SETTINGS_TOKEN_H