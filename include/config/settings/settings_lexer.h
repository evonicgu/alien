#ifndef ALIEN_CONFIG_SETTINGS_LEXER_H
#define ALIEN_CONFIG_SETTINGS_LEXER_H

#include <string>
#include "generalized/generalized_lexer.h"
#include "input/input.h"
#include "settings_token.h"
#include "util/lexing.h"

namespace alien::config::settings {

    using base_lexer = generalized::generalized_lexer<token_type>;

    using namespace util::literals;

    class lexer : base_lexer {
    public:
        explicit lexer(input::stream_input& i) : base_lexer(i) {}

        token* lex() override {
            util::u8char c = i.get();

            while (isspace(c)) {
                c = i.get();
            }

            switch (c) {
                case '#':
                    return new token(token_type::T_HASHTAG);
                case '.':
                    return new token(token_type::T_DOT);
                case '{':
                    return new token(token_type::T_OPEN_BRACE);
                case '}':
                    return new token(token_type::T_CLOSE_BRACE);
                case '=':
                    return new token(token_type::T_EQUALS);
                case ',':
                    return new token(token_type::T_COMMA);
                case '%':
                    if (i.peek() == '%') {
                        i.get();
                        return new token(token_type::T_END);
                    }

                    throw lexer_exception("Unexpected character"_u8);
                case '\"': {
                    util::u8string str;

                    util::u8char follow = i.peek();

                    while (follow != '\"') {
                        follow = i.get();

                        if (follow == '\\' && i.peek() != '\"') {
                            parse_escape(str);
                        } else {
                            str += follow;
                        }

                        follow = i.peek();
                    }

                    return new str_token(std::move(str));
                }
                case -2:
                    return new token(token_type::T_END);
                default: {
                    if (util::is_start_identifier_char(c)) {
                        util::u8string name = util::get_identifier<token_type>(i, c);

                        if (name == "true"_u8) {
                            return new bool_token(true);
                        }

                        if (name == "false"_u8) {
                            return new bool_token(false);
                        }

                        return new identifier_token(std::move(name));
                    }

                    if (isdigit(c)) {
                        int number;

                        util::u8char follow = i.peek();

                        while (isdigit(follow)) {
                            number = number * 10 + (i.get() - '0');

                            follow = i.peek();
                        }

                        return new number_token(number);
                    }

                    throw lexer_exception();
                }
            }
        }

    private:
        void parse_escape(util::u8string& str) {
            util::u8char follow = i.peek();
            util::u8string number_str;
            unsigned int digits_got = 0;

            while (follow != '\"' && digits_got != 3) {
                follow = i.get();

                number_str += follow;

                if (isdigit(follow)) {
                    ++digits_got;
                } else {
                    str += number_str;
                    return;
                }

                follow = i.peek();
            }

            int number = util::u8_stoi(number_str);

            if (digits_got == 3 && number < 128) {
                str += (char) number;
                return;
            }

            str += number_str;
        }
    };

}

#endif //ALIEN_CONFIG_SETTINGS_LEXER_H