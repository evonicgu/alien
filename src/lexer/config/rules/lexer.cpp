#include "lexer/config/rules/lexer.h"

#include <stdexcept>

#include "util/charutils.h"
#include "util/lexing.h"

namespace alien::lexer::rules {

    lexer::token_t* lexer::lex() {
        using namespace util::literals;

        util::u8char c = i.get();

        while (util::isspace(c)) {
            c = i.get();
        }

        switch (c) {
            case ':':
                return new token_t(type::T_COLON, i.get_pos().backwards(0, 1), i.get_pos());
            case ';':
                return new token_t(type::T_SEMICOLON, i.get_pos().backwards(0, 1), i.get_pos());
            case -2:
                throw std::runtime_error("Unexpected end of file");
            case '{': {
                util::pos start = i.get_pos().backwards(0, 1);
                auto [code, eof] = util::get_code_block(i);

                if (eof) {
                    throw std::runtime_error("Unexpected end of file");
                }

                if (i.peek() != '[') {
                    return new action_token(std::move(code), start, i.get_pos());
                }

                i.get();

                util::u8string name = util::get_identifier(i);

                if (name.empty()) {
                    util::pos pos = i.get_pos();

                    throw std::runtime_error("Expected a valid identifier at " + (std::string) pos);
                }

                if (i.peek() != ']') {
                    util::pos pos = i.get_pos();
                    err.push_back("Expected ']' after symbol name at "_u8 + (util::u8string) pos);
                } else {
                    i.get();
                }

                return new action_token(std::move(code), std::move(name), start, i.get_pos());
            }
            default: {
                util::pos start = i.get_pos().backwards(0, 1);
                util::u8string regex{c};

                if (c == '\\') {
                    regex.push_back(i.get());
                }

                c = i.peek();

                while (c != ':') {
                    c = i.get();

                    regex.push_back(c);

                    if (regex.size() == 2 && regex == "%%"_u8) {
                        return new token_t(type::T_END, start, i.get_pos());
                    }

                    if (regex.size() == 7 && regex == "%noutf8"_u8) {
                        return new token_t(type::T_NO_UTF8, start, i.get_pos());
                    }

                    if (c == '\\') {
                        regex.push_back(i.get());
                    }

                    c = i.peek();
                }

                if (regex == "<<<EOF>>>"_u8) {
                    return new token_t(type::T_EOF_RULE, start, i.get_pos());
                }

                if (is_valid_context(regex)) {
                    return new context_token(std::move(regex), start, i.get_pos());
                }

                return new regex_token(std::move(regex), start, i.get_pos());
            }
        }
    }

    bool lexer::is_valid_context(const util::u8string& str) {
        return str.size() > 2 && str[0] == '<' && str.back() == '>';
    }

}