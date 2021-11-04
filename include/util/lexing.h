#ifndef ALIEN_UTIL_LEXING_H
#define ALIEN_UTIL_LEXING_H

#include "generalized/generalized_lexer.h"
#include "input/input.h"
#include "util/u8string.h"

namespace alien::util {

    using namespace util::literals;

    util::u8string get_code_block(input::input& i) {
        unsigned int fold_level = 0;
        bool in_string = false, in_character = false;

        util::u8string code;

        util::u8char c = i.get();

        while (c != '}' || in_string || fold_level > 0) {
            if (!in_character && c == '"') {
                in_string = !in_string;
            }

            if (!in_string && c == '\'') {
                in_character = !in_character;
            }

            if (!in_string && !in_character) {
                if (c == '{') {
                    ++fold_level;
                }

                if (c == '}') {
                    --fold_level;
                }
            } else if (c == '\\') {
                c = i.get();

                code += '\\';
            }

            code += c;

            c = i.get();
        }

        return code;
    }

    bool is_start_identifier_char(util::u8char c) {
        return isalpha(c) || c == '_' || c == '$';
    }

    bool is_continuation_identifier_char(util::u8char c) {
        return isalnum(c) || c == '_' || c == '$';
    }

    util::u8string _get_rest_identifier(input::input& i) {
        util::u8char c = i.peek();
        util::u8string rest;

        while (is_continuation_identifier_char(c)) {
            rest += i.get();

            c = i.peek();
        }

        return rest;
    }

    template<typename T>
    util::u8string get_identifier(input::input& i, util::u8char sc) {
        if (!is_start_identifier_char(sc)) {
            throw typename generalized::generalized_lexer<T>::lexer_exception("Invalid identifier name"_u8);
        }

        return sc + _get_rest_identifier(i);
    }

    template<typename T>
    util::u8string get_identifier(input::input& i) {
        util::u8char c = i.peek();

        if (!is_start_identifier_char(c)) {
            throw typename generalized::generalized_lexer<T>::lexer_exception("Invalid identifier name"_u8);
        }

        return _get_rest_identifier(i);
    }

}

#endif //ALIEN_UTIL_LEXING_H