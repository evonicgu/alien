#ifndef ALIEN_LEXER_CONFIG_RULES_LEXER_H
#define ALIEN_LEXER_CONFIG_RULES_LEXER_H

#include "generalized/generalized_lexer.h"
#include "generalized/generalized_exception.h"
#include "input/input.h"
#include "rules_token.h"
#include "util/lexing.h"
#include "util/u8string.h"

namespace alien::lexer::config::rules::lexer {

    static constexpr const char trailing_return_exception_str[] = "Trailing returns are turned off. ";

    using base_lexer = generalized::generalized_lexer<token_type>;

    using namespace util::literals;

    class lexer : base_lexer {
        bool ret;

    public:
        lexer(bool ret, input::stream_input& i) : ret(ret), base_lexer(i) {}

        using trailing_return_exception = generalized::generalized_exception<trailing_return_exception_str>;

        token* lex() override {
            util::u8char c = i.get();

            while (isspace(c)) {
                c = i.get();
            }

            switch (c) {
                case ':':
                    return new token(token_type::T_COLON);
                case ';':
                    return new token(token_type::T_SEMICOLON);
                case -2:
                    return new token(token_type::T_END);
                case '{': {
                    util::u8string code = util::get_code_block(i);

                    if (i.peek() != '[') {
                        return new action_token(std::move(code));
                    }

                    i.get();

                    util::u8string name = util::get_identifier<token_type>(i);

                    if (i.peek() != ']') {
                        throw lexer_exception("Invalid identifier name"_u8);
                    }

                    i.get();

                    if (!ret) {
                        throw trailing_return_exception("Change the settings to enable them"_u8);
                    }

                    tr_data tr{true, std::move(name)};

                    return new action_token(std::move(code), std::move(tr));
                }
                default: {
                    util::u8string regex{c};

                    if (c == '\\') {
                        regex += i.get();
                    }

                    c = i.peek();

                    while (c != ':') {
                        c = i.get();

                        regex += c;

                        if (regex.size() == 2 && regex == "%%"_u8) {
                            return new token(token_type::T_END);
                        }

                        if (c == '\\') {
                            regex += i.get();
                        }

                        c = i.peek();
                    }

                    return new regex_token(std::move(regex));
                }
            }
        }
    };

}

#endif //ALIEN_LEXER_CONFIG_RULES_LEXER_H