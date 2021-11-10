#ifndef ALIEN_PARSER_CONFIG_RULES_LEXER_H
#define ALIEN_PARSER_CONFIG_RULES_LEXER_H

#include "generalized/generalized_lexer.h"
#include "input/input.h"
#include "rules_token.h"
#include "util/u8string.h"
#include "util/lexing.h"

namespace alien::parser::config::rules::lexer {

    using base_lexer = generalized::generalized_lexer<token_type>;

    using namespace util::literals;

    class lexer : public base_lexer {
    public:
        explicit lexer(input::stream_input& i) : base_lexer(i) {}

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
                case '|':
                    return new token(token_type::T_OR);
                case -2:
                    return new token(token_type::T_END);
                case '%':
                    if (i.peek() == '%') {
                        i.get();
                        return new token(token_type::T_END);
                    }
                    return new terminal_token(util::get_identifier<token_type>(i));
                case '{':
                    return new code_token(util::get_code_block(i));
                default:
                    return new identifier_token(util::get_identifier<token_type>(i, c));
            }
        }
    };

}

#endif //ALIEN_PARSER_CONFIG_RULES_LEXER_H