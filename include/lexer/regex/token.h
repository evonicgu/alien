#ifndef ALIEN_REGEX_TOKEN_H
#define ALIEN_REGEX_TOKEN_H

#include "util/token.h"

namespace alien::lexer::regex {

    enum class token_type {
        T_STAR,
        T_OR,
        T_PLUS,
        T_PARENTHESIS_OPEN,
        T_PARENTHESIS_CLOSE,
        T_BRACE_OPEN,
        T_BRACE_CLOSE,
        T_COMMA,
        T_HYPHEN,
        T_SYMBOL,
        T_NUMBER,
        T_DOT,
        T_QUESTION_MARK,
        T_SQUARE_BRACKET_OPEN,
        T_SQUARE_BRACKET_CLOSE,
        T_SPACE,
        T_NON_SPACE,
        T_CLASS,
        T_NEGATIVE_CLASS,
        T_DIGIT,
        T_NON_DIGIT,
        T_NON_NEWLINE,
        T_WORD_CHAR,
        T_NON_WORD_CHAR,
        T_HORIZONTAL_SPACE,
        T_NON_HORIZONTAL_SPACE,
        T_VERTICAL_SPACE,
        T_NON_VERTICAL_SPACE,
        T_VALID_SEQUENCE,
        T_UNICODE_NEWLINE,
        T_END
    };

    using base = util::token<token_type>;

    struct symbol_token : public base {
        util::u8char symbol;

        explicit symbol_token(util::u8char c, util::pos start, util::pos end)
            : symbol(c),
              base(token_type::T_SYMBOL, start, end) {}
    };

    struct number_token : public base {
        uint8_t number;

        explicit number_token(int number, util::pos start, util::pos end)
            : number(number),
              base(token_type::T_NUMBER, start, end) {}
    };

    struct symbol_class_token : public base {
        util::u8string class_name;

        explicit symbol_class_token(util::u8string&& class_name, util::pos start, util::pos end)
            : class_name(std::move(class_name)),
              base(token_type::T_CLASS, start, end) {}
    };

}

#endif //ALIEN_REGEX_TOKEN_H