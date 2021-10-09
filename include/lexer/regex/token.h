#ifndef ALIEN_REGEX_TOKEN_H
#define ALIEN_REGEX_TOKEN_H

#include "generalized/generalized_token.h"

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
        T_NEGATIVE_CLASS,
        T_DIGIT,
        T_NON_DIGIT,
        T_NON_NEWLINE,
        T_NON_VERTICAL_TABULATION,
        T_WORD_CHAR,
        T_NON_WORD_CHAR,
        T_END
    } type;

    using base_token = generalized::generalized_token<token_type>;

    struct symbol_token : public base_token {
        char symbol;

        explicit symbol_token(char c) : symbol(c), base_token(token_type::T_SYMBOL) {}
    };

    struct number_token : public base_token {
        uint8_t number;

        explicit number_token(int number) : number(number), base_token(token_type::T_NUMBER) {}
    };

}

#endif //ALIEN_REGEX_TOKEN_H