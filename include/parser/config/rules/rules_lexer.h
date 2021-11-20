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
                case '%': {
                    if (i.peek() == '%') {
                        i.get();
                        return new token(token_type::T_END);
                    }

                    auto str = util::get_identifier<token_type>(i);

                    if (str == "prec"_u8 || str == "assoc"_u8) {
                        c = i.get();

                        if (c != ':') {
                            throw lexer_exception("Expected semicolon after precedence declaration"_u8);
                        }

                        c = i.peek();
                        int value = 0;

                        if (!isdigit(c)) {
                            throw lexer_exception("Precedence must be an integer"_u8);
                        }

                        while (isdigit(c)) {
                            i.get();

                            value = value * 10 + (c - '0');

                            c = i.peek();
                        }

                        using spec_type = spec_declaration_token::spec_type;

                        return new spec_declaration_token(value, str == "prec"_u8 ? spec_type::PREC : spec_type::ASSOC);
                    } else {
                        return new terminal_token(std::move(str));
                    }
                }
                case '<': {
                    util::u8string code_type = util::get_identifier<token_type>(i);

                    if (i.get() != '>') {
                        throw lexer_exception("Expected '>' after midrule type declaration"_u8);
                    }

                    if (i.get() != '{') {
                        throw lexer_exception("Expected code block to start with '{'"_u8);
                    }

                    //
                    return new midrule_token(util::get_code_block(i), std::move(code_type));
                }
                case '{':
                    return new code_token(util::get_code_block(i));
                default:
                    return new identifier_token(util::get_identifier<token_type>(i, c));
            }
        }
    };

}

#endif //ALIEN_PARSER_CONFIG_RULES_LEXER_H