#include "config/settings/lexer.h"

namespace alien::config::settings {

    lexer::token_t* lexer::lex() {
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
            case '#':
                return new token_t(type::T_HASHTAG, i.get_pos().backwards(0, 1), i.get_pos());
            case '.':
                return new token_t(type::T_DOT, i.get_pos().backwards(0, 1), i.get_pos());
            case '{':
                return new token_t(type::T_OPEN_BRACE, i.get_pos().backwards(0, 1), i.get_pos());
            case '}':
                return new token_t(type::T_CLOSE_BRACE, i.get_pos().backwards(0, 1), i.get_pos());
            case '=':
                return new token_t(type::T_EQUALS, i.get_pos().backwards(0, 1), i.get_pos());
            case ',':
                return new token_t(type::T_COMMA, i.get_pos().backwards(0, 1), i.get_pos());
            case '%': {
                if (i.peek() == '%') {
                    i.get();
                    return new token_t(type::T_END, i.get_pos().backwards(0, 2), i.get_pos());
                }

                util::pos start = i.get_pos().backwards(0, 1);
                c = i.get();

                util::u8string name = util::get_identifier(i, c);

                if (name.empty()) {
                    throw std::runtime_error("Expected a valid identifier at " + (std::string) start);
                }

                util::u8string_view view = name;

                if (view == "code"_u8) {
                    c = i.get();

                    while (util::isspace(c)) {
                        c = i.get();
                    }

                    auto specifier_pos = i.get_pos().backwards(0, 1);
                    util::u8string pos_specifier = util::get_identifier(i, c);

                    code_token::location loc;

                    if (pos_specifier == "headers"_u8) {
                        loc = code_token::location::HEADERS;
                    } else if (pos_specifier == "decl"_u8) {
                        loc = code_token::location::DECL;
                    } else if (pos_specifier == "impl"_u8) {
                        loc = code_token::location::IMPL;
                    } else if (pos_specifier == "content-impl"_u8) {
                        loc = code_token::location::CONTENT_IMPL;
                    } else if (pos_specifier == "content-decl"_u8) {
                        c = i.get();

                        while (util::isspace(c)) {
                            c = i.get();
                        }

                        auto visibility_pos = i.get_pos().backwards(0, 1);

                        util::u8string visibility_specifier = util::get_identifier(i, c);

                        if (visibility_specifier == "public"_u8) {
                            loc = code_token::location::CONTENT_DECL_PUBLIC;
                        } else if (visibility_specifier == "private"_u8) {
                            loc = code_token::location::CONTENT_DECL_PRIVATE;
                        } else {
                            err.push_back("Unknown visibility specifier '"_u8 + visibility_specifier + "' at"_u8 +
                                          (util::u8string) visibility_pos);

                            loc = code_token::location::CONTENT_DECL_PUBLIC;
                        }
                    } else {
                        err.push_back("Unknown code specifier '"_u8 + pos_specifier + "' at"_u8 +
                                      (util::u8string) specifier_pos);

                        loc = code_token::location::DECL;
                    }

                    c = i.get();

                    while (util::isspace(c)) {
                        c = i.get();
                    }

                    if (c != '{') {
                        err.push_back("Expected '{' after code declaration at "_u8 + (util::u8string) (i.get_pos().backwards(0, 1)));
                    }

                    auto [code, eof] = util::get_code_block(i);

                    if (eof) {
                        throw std::runtime_error("Unexpected end of file");
                    }

                    return new code_token(std::move(code), loc, start, i.get_pos());
                }

                return new specifier_token(std::move(name), start, i.get_pos());
            }
            case '\"': {
                util::pos start = i.get_pos().backwards(0, 1);

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

                return new str_token(std::move(str), start, i.get_pos());
            }
            case -2:
                throw std::runtime_error("Unexpected end of file");
            default:
                util::pos start = i.get_pos().backwards(0, 1);

                if (util::is_start_identifier_char(c)) {
                    util::u8string name = util::get_identifier(i, c);

                    if (name == "true"_u8) {
                        return new bool_token(true, start, i.get_pos().backwards(0, 1));
                    }

                    if (name == "false"_u8) {
                        return new bool_token(false, start, i.get_pos().backwards(0, 1));
                    }

                    return new identifier_token(std::move(name), start, i.get_pos().backwards(0, 1));
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

                    return new number_token(util::u8_stoi(number_str), start, i.get_pos().backwards(0, 1));
                }

                throw std::runtime_error("Tokenizing error at " + (std::string) start);
        }
    }

}