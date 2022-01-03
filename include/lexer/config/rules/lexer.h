#ifndef ALIEN_LEXER_RULES_LEXER_H
#define ALIEN_LEXER_RULES_LEXER_H

#include <list>

#include "token.h"
#include "util/charutils.h"
#include "util/lexing.h"
#include "util/lexer.h"
#include "util/u8string.h"

namespace alien::lexer::rules {

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

            switch (c) {
                case ':':
                    return new token(type::T_COLON, {i.line, i.column - 1}, {i.line, i.column});
                case ';':
                    return new token(type::T_SEMICOLON, {i.line, i.column - 1}, {i.line, i.column});
                case -2:
                    throw std::runtime_error("Unexpected end of file");
                case '{': {
                    util::pos start{i.line, i.column - 1};
                    auto [code, eof] = util::get_code_block(i);

                    if (eof) {
                        throw std::runtime_error("Unexpected end of file");
                    }

                    if (i.peek() != '[') {
                        return new action_token(std::move(code), start, {i.line, i.column});
                    }

                    i.get();

                    util::u8string name = util::get_identifier(i);

                    if (name.empty()) {
                        util::pos pos{i.line, i.column};

                        throw std::runtime_error("Expected a valid identifier at " + (std::string) pos);
                    }

                    if (i.peek() != ']') {
                        util::pos pos{i.line, i.column};
                        err.push_back("Expected ']' after symbol name at "_u8 + (util::u8string) pos);
                    } else {
                        i.get();
                    }

                    return new action_token(std::move(code), std::move(name), start, {i.line, i.column});
                }
                default: {
                    util::pos start{i.line, i.column - 1};
                    util::u8string regex{c};

                    if (c == '\\') {
                        regex.push_back(i.get());
                    }

                    c = i.peek();

                    while (c != ':') {
                        c = i.get();

                        regex.push_back(c);

                        if (regex.size() == 2 && regex == "%%"_u8) {
                            return new token(type::T_END, start, {i.line, i.column});
                        }

                        if (regex.size() == 7 && regex == "%noutf8"_u8) {
                            return new token(type::T_NO_UTF8, start, {i.line, i.column});
                        }

                        if (c == '\\') {
                            regex.push_back(i.get());
                        }

                        c = i.peek();
                    }

                    if (regex == "<<<EOF>>>"_u8) {
                        return new token(type::T_EOF_RULE, start, {i.line, i.column});
                    }

                    if (is_valid_context(regex)) {
                        return new context_token(std::move(regex), start, {i.line, i.column});
                    }

                    return new regex_token(std::move(regex), start, {i.line, i.column});
                }
            }
        }

        static bool is_valid_context(const util::u8string& str) {
            return str.size() > 2 && str[0] == '<' && str.back() == '>';
        }
    };

}

#endif //ALIEN_LEXER_RULES_LEXER_H