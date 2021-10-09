#ifndef ALIEN_LEXER_CONFIG_RULES_LEXER_H
#define ALIEN_LEXER_CONFIG_RULES_LEXER_H

#include <string>
#include "generalized/generalized_lexer.h"
#include "generalized/generalized exception.h"
#include "input/input.h"
#include "rules_token.h"

namespace alien::lexer::config::rules::lexer {

    static constexpr const char trailing_return_exception_str[] = "Trailing returns are turned off. ";

    using base_lexer = generalized::generalized_lexer<token_type>;

    class lexer : base_lexer {
        bool ret;

    public:
        lexer(bool ret, input::stream_input& i) : ret(ret), base_lexer(i) {}

        using trailing_return_exception = generalized::generalized_exception<trailing_return_exception_str>;

        token* lex() override {
            char c = i.get();

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
                    unsigned int fold_level = 0;

                    std::string code;

                    c = i.get();

                    while (c != '}' || fold_level > 0) {
                        if (c == '{') {
                            ++fold_level;
                        }

                        if (c == '}') {
                            --fold_level;
                        }

                        code += c;

                        c = i.get();
                    }

                    if (i.peek() != '[') {
                        return new action_token(std::move(code));
                    }

                    i.get();

                    c = i.get();
                    std::string name = {c};

                    if (!isalpha(c) && c != '_' && c != '$') {
                        throw lexer_exception("Invalid identifier name");
                    }

                    c = i.peek();

                    while (c != ']') {
                        if (!isalnum(c) && c != '_' && c != '$') {
                            throw lexer_exception("Invalid identifier name");
                        }

                        name += i.get();

                        c = i.peek();
                    }

                    i.get();

                    if (!ret) {
                        throw trailing_return_exception("Change the settings to be able to use them");
                    }

                    tr_data tr{true, std::move(name)};

                    return new action_token(std::move(code), std::move(tr));
                }
                default: {
                    std::string regex{c};

                    if (c == '\\') {
                        regex += i.get();
                    }

                    c = i.peek();

                    while (c != ':') {
                        c = i.get();

                        regex += c;

                        if (regex.size() == 2 && regex == "%%") {
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