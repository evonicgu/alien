#include "lexer/regex/lexer.h"

#include <stdexcept>

#include "util/lexing.h"

namespace alien::lexer::regex {

    lexer::token_t* lexer::lex() {
        using namespace util::literals;

        util::pos start = i.get_pos();
        util::u8char c = i.get();

        switch (c) {
            case '*':
                return new token_t(token_type::T_STAR, start, i.get_pos());
            case '|':
                return new token_t(token_type::T_OR, start, i.get_pos());
            case '+':
                return new token_t(token_type::T_PLUS, start, i.get_pos());
            case '(':
                return new token_t(token_type::T_PARENTHESIS_OPEN, start, i.get_pos());
            case ')':
                return new token_t(token_type::T_PARENTHESIS_CLOSE, start, i.get_pos());
            case '{':
                return new token_t(token_type::T_BRACE_OPEN, start, i.get_pos());
            case '}':
                return new token_t(token_type::T_BRACE_CLOSE, start, i.get_pos());
            case ',':
                return new token_t(token_type::T_COMMA, start, i.get_pos());
            case '-':
                return new token_t(token_type::T_HYPHEN, start, i.get_pos());
            case '.':
                return new token_t(token_type::T_DOT, start, i.get_pos());
            case '?':
                return new token_t(token_type::T_QUESTION_MARK, start, i.get_pos());
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
                                        "Expected '}' after multicharacter unicode property name "_u8 + name
                                );
                            } else {
                                i.get();
                            }
                        } else if (isalpha(class_c)) {
                            class_c = get_class_char();

                            name += class_c;
                        } else {
                            throw std::runtime_error("Expected a valid unicode property name after \\p");
                        }

                        return new symbol_class_token(std::move(name), start, i.get_pos());
                    }
                    case 'h':
                        return new token_t(token_type::T_HORIZONTAL_SPACE, start, i.get_pos());
                    case 'H':
                        return new token_t(token_type::T_NON_HORIZONTAL_SPACE, start, i.get_pos());
                    case 'X':
                        return new token_t(token_type::T_VALID_SEQUENCE, start, i.get_pos());
                    case 'R':
                        return new token_t(token_type::T_UNICODE_NEWLINE, start, i.get_pos());
                    case 's':
                        return new token_t(token_type::T_SPACE, start, i.get_pos());
                    case 'S':
                        return new token_t(token_type::T_NON_SPACE, start, i.get_pos());
                    case 'd':
                        return new token_t(token_type::T_DIGIT, start, i.get_pos());
                    case 'D':
                        return new token_t(token_type::T_NON_DIGIT, start, i.get_pos());
                    case 'n':
                        return new symbol_token('\n', start, i.get_pos());
                    case 'N':
                        return new token_t(token_type::T_NON_NEWLINE, start, i.get_pos());
                    case 't':
                        return new symbol_token('\t', start, i.get_pos());
                    case 'v':
                        return new token_t(token_type::T_VERTICAL_SPACE, start, i.get_pos());
                    case 'V':
                        return new token_t(token_type::T_NON_VERTICAL_SPACE, start, i.get_pos());
                    case 'w':
                        return new token_t(token_type::T_WORD_CHAR, start, i.get_pos());
                    case 'W':
                        return new token_t(token_type::T_NON_WORD_CHAR, start, i.get_pos());
                    case 'f':
                        return new symbol_token('\f', start, i.get_pos());
                    case 'r' :
                        return new symbol_token('\r', start, i.get_pos());
                    case '0':
                        return new symbol_token('\0', start, i.get_pos());
                    case 'U':
                    case 'u': {
                        if (i.get() != '{') {
                            throw std::runtime_error("Expected '{' after the \\u in the escape sequence");
                        }

                        int codepoint = 0, counter = 0;

                        while (counter < 8 && i.peek() != '}') {
                            c = i.get();

                            codepoint = codepoint * 16 + util::xdigit_to_num(c);
                            ++counter;
                        }

                        if (i.get() != '}') {
                            throw std::runtime_error("Expected '}' after the \\u in the escape sequence");
                        }

                        return new symbol_token(codepoint, start, i.get_pos());
                    }
                    default:
                        return new symbol_token(symbol, start, i.get_pos());
                }
            }
            case '[':
                return new token_t(token_type::T_SQUARE_BRACKET_OPEN, start, i.get_pos());
            case ']':
                return new token_t(token_type::T_SQUARE_BRACKET_CLOSE, start, i.get_pos());
            case '^':
                return new token_t(token_type::T_NEGATIVE_CLASS, start, i.get_pos());
            case -2:
                return new token_t(token_type::T_END, start, i.get_pos());
            default:
                if (isdigit(c)) {
                    int number = c - '0';

                    return new number_token(number, start, i.get_pos());
                }

                return new symbol_token(c, start, i.get_pos());
        }
    }

    util::u8char lexer::get_class_char() {
        util::u8char c = i.peek();

        if (!isalpha(c)) {
            throw std::runtime_error("Invalid property name character " + util::u8string_to_bytes({c}));
        }

        return tolower(i.get());
    }


}