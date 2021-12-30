#ifndef ALIEN_LEXER_H
#define ALIEN_LEXER_H

#include <list>

#include "token.h"
#include "util/charutils.h"
#include "util/lexer.h"
#include "util/lexing.h"
#include "util/u8string.h"

namespace alien::config::settings {

    using namespace util::literals;

    class lexer : public util::lexer<token_type> {
    public:
        explicit lexer(input::input& i, std::list<util::u8string>& err)
            : util::lexer<token_type>(i, err) {}

        token* lex() override {
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
                case '#':
                    return new token(type::T_HASHTAG, {i.line, i.column - 1}, {i.line, i.column});
                case '.':
                    return new token(type::T_DOT, {i.line, i.column - 1}, {i.line, i.column});
                case '{':
                    return new token(type::T_OPEN_BRACE, {i.line, i.column - 1}, {i.line, i.column});
                case '}':
                    return new token(type::T_CLOSE_BRACE, {i.line, i.column - 1}, {i.line, i.column});
                case '=':
                    return new token(type::T_EQUALS, {i.line, i.column - 1}, {i.line, i.column});
                case ',':
                    return new token(type::T_COMMA, {i.line, i.column - 1}, {i.line, i.column});
                case '%': {
                    if (i.peek() == '%') {
                        i.get();
                        return new token(type::T_END, {i.line, i.column - 2}, {i.line, i.column});
                    }

                    util::pos start{i.line, i.column - 1};
                    c = i.get();

                    util::u8string name = util::get_identifier(i, c);

                    if (name.empty()) {
                        throw std::runtime_error("Expected a valid identifier at " + (std::string) start);
                    }

                    util::u8string_view view = name;

                    if (view.substr(0, 5) == "code-"_u8 || view == "code"_u8) {
                        code_token::location loc;

                        if (view.size() == 4) {
                            loc = code_token::location::DEFAULT;
                        } else if (view.size() == 8 && view.substr(4) == "-top"_u8) {
                            loc = code_token::location::TOP;
                        } else if (view.size() == 12 && view.substr(4) == "-content"_u8) {
                            loc = code_token::location::CONTENT;
                        } else {
                            util::u8string pos = (util::u8string) start;
                            err.push_back("Unknown code specifier '"_u8.append(view.substr(4)) + "' at "_u8 + pos);

                            loc = code_token::location::DEFAULT;
                        }

                        c = i.get();

                        while (util::isspace(c)) {
                            c = i.get();
                        }

                        if (c != '{') {
                            util::pos pos{i.line, i.column - 1};

                            err.push_back("Expected '{' after code declaration at "_u8 + (util::u8string) pos);
                        }

                        auto [code, eof] = util::get_code_block(i);

                        if (eof) {
                            throw std::runtime_error("Unexpected end of file");
                        }

                        return new code_token(std::move(code), loc, start, {i.line, i.column});
                    }

                    return new specifier_token(std::move(name), start, {i.line, i.column});
                }
                case '\"': {
                    util::pos start{i.line, i.column - 1};

                    util::u8string str;
                    util::u8char follow = i.peek();

                    while (follow != '\"') {
                        follow = i.get();

                        if (follow == -2) {
                            throw std::runtime_error("Unexpected end of file");
                        }

                        if (follow == '\\') {
                            util::u8char escaped = util::parse_escape(i);

                            if (escaped == -1) {
                                throw std::runtime_error(
                                        "Invalid codepoint in string literal at " + (std::string) start
                                        );
                            } else if (escaped == -2) {
                                throw std::runtime_error(
                                        "Invalid escape sequence in string literal at " + (std::string) start
                                        );
                            }

                            str.push_back(escaped);
                        } else {
                            str.push_back(follow);
                        }

                        follow = i.peek();
                    }

                    i.get();

                    return new str_token(std::move(str), start, {i.line, i.column});
                }
                case -2:
                    throw std::runtime_error("Unexpected end of file");
                default:
                    util::pos start{i.line, i.column - 1};

                    if (util::is_start_identifier_char(c)) {
                        util::u8string name = util::get_identifier(i, c);

                        if (c == ':') {
                            std::cout << "type";
                        }

                        if (name == "true"_u8) {
                            return new bool_token(true, start, {i.line, i.column - 1});
                        }

                        if (name == "false"_u8) {
                            return new bool_token(false, start, {i.line, i.column - 1});
                        }

                        return new identifier_token(std::move(name), start, {i.line, i.column - 1});
                    }

                    if (isdigit(c) || c == '-') {
                        util::u8string number_str{c};
                        c = i.peek();

                        while (isdigit(c)) {
                            c = i.get();
                            number_str.push_back(c);

                            c = i.peek();
                        }

                        if (number_str == "-"_u8) {
                            throw std::runtime_error("Invalid integer constant '-' at " + (std::string) start);
                        }

                        return new number_token(util::u8_stoi(number_str), start, {i.line, i.column - 1});
                    }

                    throw std::runtime_error("Tokenizing error at " + (std::string) start);
            }
        }
    };

}

#endif //ALIEN_LEXER_H