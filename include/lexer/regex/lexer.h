#ifndef ALIEN_REGEX_LEXER_H
#define ALIEN_REGEX_LEXER_H

#include <list>
#include <stdexcept>

#include "token.h"
#include "util/lexer.h"
#include "util/u8string.h"

namespace alien::lexer::regex {

    class lexer : public util::lexer<token_type> {
    public:
        explicit lexer(input::input& i, std::list<util::u8string>& err)
            : util::lexer<token_type>(i, err) {}

        token* lex() override {
            using namespace util::literals;

            util::pos start{i.line, i.column};
            util::u8char c = i.get();

            switch (c) {
                case '*':
                    return new token(token_type::T_STAR, start, {i.line, i.column});
                case '|':
                    return new token(token_type::T_OR, start, {i.line, i.column});
                case '+':
                    return new token(token_type::T_PLUS, start, {i.line, i.column});
                case '(':
                    return new token(token_type::T_PARENTHESIS_OPEN, start, {i.line, i.column});
                case ')':
                    return new token(token_type::T_PARENTHESIS_CLOSE, start, {i.line, i.column});
                case '{':
                    return new token(token_type::T_BRACE_OPEN, start, {i.line, i.column});
                case '}':
                    return new token(token_type::T_BRACE_CLOSE, start, {i.line, i.column});
                case ',':
                    return new token(token_type::T_COMMA, start, {i.line, i.column});
                case '-':
                    return new token(token_type::T_HYPHEN, start, {i.line, i.column});
                case '.':
                    return new token(token_type::T_DOT, start, {i.line, i.column});
                case '?':
                    return new token(token_type::T_QUESTION_MARK, start, {i.line, i.column});
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

                                if (i.peek() != '}') {
                                    err.push_back(
                                            "Expected '}' after multicharacter property name "_u8 + name
                                            );
                                } else {
                                    i.get();
                                }
                            } else if (isalpha(class_c)) {
                                class_c = get_class_char();

                                name += class_c;
                            }else {
                                throw std::runtime_error("Expected a valid property name after \\p");
                            }

                            return new symbol_class_token(std::move(name), start, {i.line, i.column});
                        }
                        case 'h':
                            return new token(token_type::T_HORIZONTAL_SPACE, start, {i.line, i.column});
                        case 'H':
                            return new token(token_type::T_NON_HORIZONTAL_SPACE, start, {i.line, i.column});
                        case 'X':
                            return new token(token_type::T_VALID_SEQUENCE, start, {i.line, i.column});
                        case 'R':
                            return new token(token_type::T_UNICODE_NEWLINE, start, {i.line, i.column});
                        case 's':
                            return new token(token_type::T_SPACE, start, {i.line, i.column});
                        case 'S':
                            return new token(token_type::T_NON_SPACE, start, {i.line, i.column});
                        case 'd':
                            return new token(token_type::T_DIGIT, start, {i.line, i.column});
                        case 'D':
                            return new token(token_type::T_NON_DIGIT, start, {i.line, i.column});
                        case 'n':
                            return new symbol_token('\n', start, {i.line, i.column});
                        case 'N':
                            return new token(token_type::T_NON_NEWLINE, start, {i.line, i.column});
                        case 't':
                            return new symbol_token('\t', start, {i.line, i.column});
                        case 'v':
                            return new token(token_type::T_VERTICAL_SPACE, start, {i.line, i.column});
                        case 'V':
                            return new token(token_type::T_NON_VERTICAL_SPACE, start, {i.line, i.column});
                        case 'w':
                            return new token(token_type::T_WORD_CHAR, start, {i.line, i.column});
                        case 'W':
                            return new token(token_type::T_NON_WORD_CHAR, start, {i.line, i.column});
                        case 'f':
                            return new symbol_token('\f', start, {i.line, i.column});
                        case 'r' :
                            return new symbol_token('\r', start, {i.line, i.column});
                        case '0':
                            return new symbol_token('\0', start, {i.line, i.column});
                        case 'U':
                        case 'u': {
                            if (i.get() != '{') {
                                throw std::runtime_error("Expected '{' after the start of the \\u escape sequence");
                            }

                            int codepoint = 0, counter = 0;

                            while (counter < 8 && i.peek() != '}') {
                                c = i.get();

                                codepoint = codepoint * 16 + util::xdigit_to_num(c);
                            }

                            if (i.get() != '}') {
                                throw std::runtime_error("Expected '}' after the end of the \\u escape sequence");
                            }

                            return new symbol_token(codepoint, start, {i.line, i.column});
                        }
                        default:
                            return new symbol_token(symbol, start, {i.line, i.column});
                    }
                }
                case '[':
                    return new token(token_type::T_SQUARE_BRACKET_OPEN, start, {i.line, i.column});
                case ']':
                    return new token(token_type::T_SQUARE_BRACKET_CLOSE, start, {i.line, i.column});
                case '^':
                    return new token(token_type::T_NEGATIVE_CLASS, start, {i.line, i.column});
                case -2:
                    return new token(token_type::T_END, start, {i.line, i.column});
                default:
                    if (isdigit(c)) {
                        int number = c - '0';

                        return new number_token(number, start, {i.line, i.column});
                    }

                    return new symbol_token(c, start, {i.line, i.column});
            }
        }

    private:
        util::u8char get_class_char() {
            util::u8char c = i.peek();

            if (!isalpha(c)) {
                throw std::runtime_error("Invalid property name character " + util::u8string_to_bytes({c}));
            }

            return tolower(i.get());
        }
    };

}

#endif //ALIEN_REGEX_LEXER_H