#include "parser/config/rules/lexer.h"

namespace alien::parser::rules {

    lexer::token_t* lexer::lex() {
        using namespace util::literals;

        util::u8char c = i.get();

        while (util::isspace(c)) {
            c = i.get();
        }

        while (c == '/') {
            if (i.peek() != '/') {
                throw std::runtime_error("Unexpected character at " + (std::string) (i.get_pos().backwards(0, 1)));
            }

            bool reached_newline = false;

            while ((!reached_newline || util::isspace(c)) && c != -2) {
                c = i.get();
                util::u8char c_class = util::get_class(c);
                reached_newline = reached_newline || c_class == -5 || c_class == -31 || c_class == -32;
            }
        }

        switch (c) {
            case ':':
                return new token_t(type::T_COLON, i.get_pos().backwards(0, 1), i.get_pos());
            case ';':
                return new token_t(type::T_SEMICOLON, i.get_pos().backwards(0, 1), i.get_pos());
            case '|':
                return new token_t(type::T_OR, i.get_pos().backwards(0, 1), i.get_pos());
            case '%': {
                util::pos start = i.get_pos().backwards(0, 1);

                if (i.peek() == '%') {
                    i.get();
                    return new token_t(type::T_END, start, i.get_pos());
                }

                util::u8string name = util::get_identifier(i);

                if (name.empty()) {
                    start.column += 1;

                    throw std::runtime_error("Expected a valid identifier at " + (std::string) start);
                }

                if (name == "prec"_u8) {
                    return new token_t(type::T_PREC, start, i.get_pos());
                }

                return new terminal_token(std::move(name), start, i.get_pos());
            }
            case -2:
                throw std::runtime_error("Unexpected end of file");
            case '<': {
                util::pos start = i.get_pos().backwards(0, 1);
                util::u8string type = util::get_identifier(i);

                if (type.empty()) {
                    start.column += 1;

                    throw std::runtime_error("Expected a valid identifier at " + (std::string) start);
                }

                if (i.peek() != '>') {
                    err.push_back(
                            "Expected matching '>' at "_u8 + (util::u8string) i.get_pos()
                    );
                } else {
                    i.get();
                }

                if (i.peek() != '{') {
                    err.push_back(
                            "Expected '{' after midrule type at "_u8 + (util::u8string) i.get_pos()
                    );
                } else {
                    i.get();
                }

                auto [code, eof] = util::get_code_block(i);

                if (eof) {
                    throw std::runtime_error("Unexpected end of file");
                }

                return new midrule_action_token(std::move(code), std::move(type), start, i.get_pos());
            }
            case '{': {
                util::pos start = i.get_pos().backwards(0, 1);
                auto [code, eof] = util::get_code_block(i);

                if (eof) {
                    throw std::runtime_error("Unexpected end of file");
                }

                return new action_token(std::move(code), start, i.get_pos());
            }
            default: {
                util::pos start = i.get_pos().backwards(0, 1);

                if (util::is_start_identifier_char(c)) {
                    util::u8string identifier = util::get_identifier(i, c);

                    if (identifier == "error"_u8) {
                        return new token_t(token_type::T_ERROR, start, i.get_pos());
                    }

                    return new identifier_token(std::move(identifier), start, i.get_pos());
                }

                throw std::runtime_error("Tokenizing error at " + (std::string) start);
            }
        }
    }


}