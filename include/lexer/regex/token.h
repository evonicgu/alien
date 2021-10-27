#ifndef ALIEN_REGEX_TOKEN_H
#define ALIEN_REGEX_TOKEN_H

#include "generalized/generalized_token.h"
#include "util/u8string.h"

namespace alien::lexer::regex::lexer {

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

    using base_token = generalized::generalized_token<token_type>;

    struct symbol_token : public base_token {
        util::u8char symbol;

        explicit symbol_token(util::u8char c) : symbol(c), base_token(token_type::T_SYMBOL) {}
    };

    struct number_token : public base_token {
        uint8_t number;

        explicit number_token(int number) : number(number), base_token(token_type::T_NUMBER) {}
    };

    struct symbol_class_token : public base_token {
        util::u8string class_name;

        explicit symbol_class_token(const util::u8string& class_name) : class_name(class_name),
                                                                                base_token(token_type::T_CLASS) {}

        explicit symbol_class_token(util::u8string&& class_name) : class_name(std::move(class_name)),
                                                                           base_token(token_type::T_CLASS) {}
    };
}

#endif //ALIEN_REGEX_TOKEN_H