#ifndef ALIEN_REGEX_LEXER_H
#define ALIEN_REGEX_LEXER_H

#include <string>
#include "generalized/generalized_lexer.h"
#include "token.h"
#include "input/input.h"

namespace alien::lexer::regex::lexer {

    using base_lexer = generalized::generalized_lexer<token_type>;

    class lexer : public base_lexer {
    public:
        explicit lexer(input::input& i) : base_lexer(i) {}

        token* lex() override {
            char c = i.get();

            switch (c) {
                case '*':
                    return new token(token_type::T_STAR);
                case '|':
                    return new token(token_type::T_OR);
                case '+':
                    return new token(token_type::T_PLUS);
                case '(':
                    return new token(token_type::T_PARENTHESIS_OPEN);
                case ')':
                    return new token(token_type::T_PARENTHESIS_CLOSE);
                case '{':
                    return new token(token_type::T_BRACE_OPEN);
                case '}':
                    return new token(token_type::T_BRACE_CLOSE);
                case ',':
                    return new token(token_type::T_COMMA);
                case '-':
                    return new token(token_type::T_HYPHEN);
                case '.':
                    return new token(token_type::T_DOT);
                case '?':
                    return new token(token_type::T_QUESTION_MARK);
                case '\\': {
                    char symbol = i.get();

                    switch (symbol) {
                        case 's':
                            return new token(token_type::T_SPACE);
                        case 'S':
                            return new token(token_type::T_NON_SPACE);
                        case 'd':
                            return new token(token_type::T_DIGIT);
                        case 'D':
                            return new token(token_type::T_NON_DIGIT);
                        case 'n':
                            return new symbol_token('\n');
                        case 'N':
                            return new token(token_type::T_NON_NEWLINE);
                        case 't':
                            return new symbol_token('\t');
                        case 'v':
                            return new symbol_token('\v');
                        case 'V':
                            return new token(token_type::T_NON_VERTICAL_TABULATION);
                        case 'w':
                            return new token(token_type::T_WORD_CHAR);
                        case 'W':
                            return new token(token_type::T_NON_WORD_CHAR);
                        case 'f':
                            return new symbol_token('\f');
                        case 'r' :
                            return new symbol_token('\r');
                        case '0':
                            return new symbol_token('\0');
                        default:
                            return new symbol_token(symbol);
                    }
                }
                case '[':
                    return new token(token_type::T_SQUARE_BRACKET_OPEN);
                case ']':
                    return new token(token_type::T_SQUARE_BRACKET_CLOSE);
                case '^':
                    return new token(token_type::T_NEGATIVE_CLASS);
                case -2:
                    return new token(token_type::T_END);
                default:
                    if (isdigit(c)) {
                        int number = c - '0';

                        return new number_token(number);
                    }

                    return new symbol_token(c);
            }
        }
    };

}

#endif //ALIEN_REGEX_LEXER_H