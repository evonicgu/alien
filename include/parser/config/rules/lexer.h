#ifndef ALIEN_PARSER_RULES_LEXER_H
#define ALIEN_PARSER_RULES_LEXER_H

#include <list>

#include "input/input.h"
#include "token.h"
#include "util/charutils.h"
#include "util/lexer.h"
#include "util/lexing.h"
#include "util/u8string.h"

namespace alien::parser::rules {

    class lexer : public util::lexer<token_type> {
    public:
        lexer(input::input& i, std::list<util::u8string>& err)
            : util::lexer<token_type>(i, err) {}

        token* lex() override {
            using namespace util::literals;

            util::u8char c = i.get();

            while (util::isspace(c)) {
                c = i.get();
            }

            while (c == '/') {
                if (i.peek() != '/') {
                    util::pos err_pos{i.line, i.column - 1};
                    throw std::runtime_error("Unexpected character at " + (std::string) err_pos);
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
                    return new token(type::T_COLON, {i.line, i.column - 1}, {i.line, i.column});
                case ';':
                    return new token(type::T_SEMICOLON, {i.line, i.column - 1}, {i.line, i.column});
                case '|':
                    return new token(type::T_OR, {i.line, i.column - 1}, {i.line, i.column});
                case '%': {
                    util::pos start{i.line, i.column - 1};

                    if (i.peek() == '%') {
                        i.get();
                        return new token(type::T_END, start, {i.line, i.column});
                    }

                    util::u8string name = util::get_identifier(i);

                    if (name.empty()) {
                        start.column += 1;

                        throw std::runtime_error("Expected a valid identifier at " + (std::string) start);
                    }

                    if (name == "prec"_u8) {
                        return new token(type::T_PREC, start, {i.line, i.column});
                    }

                    return new terminal_token(std::move(name), start, {i.line, i.column});
                }
                case -2:
                    throw std::runtime_error("Unexpected end of file");
                case '<': {
                    util::pos start{i.line, i.column - 1};
                    util::u8string type = util::get_identifier(i);

                    if (type.empty()) {
                        start.column += 1;

                        throw std::runtime_error("Expected a valid identifier at " + (std::string) start);
                    }

                    if (i.peek() != '>') {
                        util::pos err_pos{i.line, i.column};

                        err.push_back(
                                "Expected matching '>' at "_u8 + (util::u8string) err_pos
                                );
                    } else {
                        i.get();
                    }

                    if (i.peek() != '{') {
                        util::pos err_pos{i.line, i.column};

                        err.push_back(
                                "Expected '{' after midrule type at "_u8 + (util::u8string) err_pos
                                );
                    } else {
                        i.get();
                    }

                    auto [code, eof] = util::get_code_block(i);

                    if (eof) {
                        throw std::runtime_error("Unexpected end of file");
                    }

                    return new midrule_action_token(std::move(code), std::move(type), start, {i.line, i.column});
                }
                case '{': {
                    util::pos start{i.line, i.column - 1};
                    auto [code, eof] = util::get_code_block(i);

                    if (eof) {
                        throw std::runtime_error("Unexpected end of file");
                    }

                    return new action_token(std::move(code), start, {i.line, i.column});
                }
                default: {
                    util::pos start{i.line, i.column - 1};

                    if (util::is_start_identifier_char(c)) {
                        util::u8string identifier = util::get_identifier(i, c);

                        if (identifier == "error"_u8) {
                            return new token(token_type::T_ERROR, start, {i.line, i.column});
                        }

                        return new identifier_token(std::move(identifier), start, {i.line, i.column});
                    }

                    throw std::runtime_error("Tokenizing error at " + (std::string) start);
                }
            }
        }
    };

}

#endif //ALIEN_PARSER_RULES_LEXER_H