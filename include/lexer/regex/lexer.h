#ifndef ALIEN_REGEX_LEXER_H
#define ALIEN_REGEX_LEXER_H

#include "generalized/generalized_lexer.h"
#include "input/input.h"
#include "token.h"
#include "util/u8string.h"

namespace alien::lexer::regex::lexer {

    using base_lexer = generalized::generalized_lexer<token_type>;
    static constexpr char class_name_exception_str[] = "Invalid class string, expected \\pX or \\p{X} or \\p{XX}";

    using namespace util::literals;

    class lexer : public base_lexer {
    public:
        explicit lexer(input::input& i) : base_lexer(i) {}

        using class_name_exception = generalized::generalized_exception<class_name_exception_str>;

        token* lex() override {
            util::u8char c = i.get();

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
                    util::u8char symbol = i.get();

                    switch (symbol) {
                        case 'p': {
                            util::u8string name;
                            util::u8char class_c = i.peek();

                            if (class_c == '{') {
                                i.get();

                                class_c = get_class_char();
                                name += class_c;

                                if (i.peek() != '}') {
                                    class_c = get_class_char();
                                    name += class_c;
                                }

                                if (i.get() != '}') {
                                    throw class_name_exception();
                                }
                            } else if (isalpha(class_c)) {
                                class_c = get_class_char();

                                name += class_c;
                            }else {
                                throw class_name_exception();
                            }

                            return new symbol_class_token(std::move(name));
                        }
                        case 'h':
                            return new token(token_type::T_HORIZONTAL_SPACE);
                        case 'H':
                            return new token(token_type::T_NON_HORIZONTAL_SPACE);
                        case 'X':
                            return new token(token_type::T_VALID_SEQUENCE);
                        case 'R':
                            return new token(token_type::T_UNICODE_NEWLINE);
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
                            return new token(token_type::T_VERTICAL_SPACE);
                        case 'V':
                            return new token(token_type::T_NON_VERTICAL_SPACE);
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

    private:
        util::u8char get_class_char() {
            util::u8char c = i.peek();

            if (!isalpha(c)) {
                throw class_name_exception();
            }

            return tolower(i.get());
        }
    };

}

#endif //ALIEN_REGEX_LEXER_H