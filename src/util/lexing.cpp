#include "util/lexing.h"

namespace alien::util {

    std::pair<u8string, bool> get_code_block(input::input& i) {
        std::size_t fold_level = 0;
        bool in_string = false, in_character = false;

        u8string code;

        u8char c = i.get();

        while (c != '}' || in_string || fold_level > 0) {
            if (c == -2) {
                break;
            }

            if (!in_character && c == '"') {
                in_string = !in_string;
            }

            if (!in_string && c == '\'') {
                in_character = !in_character;
            }

            if (!in_string && !in_character) {
                if (c == '{') {
                    ++fold_level;
                }

                if (c == '}') {
                    --fold_level;
                }
            } else if (c == '\\') {
                c = i.get();

                code += '\\';
            }

            code += c;

            c = i.get();
        }

        return {code, c == -2};
    }

    bool is_start_identifier_char(u8char c) {
        return isalpha(c) || c == '_' || c == '$';
    }

    bool is_continuation_identifier_char(util::u8char c) {
        return isalnum(c) || c == '_' || c == '$' || c == '-';
    }

    u8string get_identifier(input::input& i, u8char first) {
        if (!is_start_identifier_char(first)) {
            return {};
        }

        u8string name{first};
        u8char c = i.peek();

        while (is_continuation_identifier_char(c)) {
            name.push_back(i.get());

            c = i.peek();
        }

        return name;
    }

    u8string get_identifier(input::input& i) {
        if (!is_start_identifier_char(i.peek())) {
            return {};
        }

        return get_identifier(i, i.get());
    }

    short xdigit_to_num(u8char c) {
        if (c >= 'a' && c <= 'f') {
            return char(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            return char(c - 'A' + 10);
        } else if (c >= '0' && c <= '9') {
            return char(c - '0');
        }

        // Invalid xdigit
        return -1;
    }

    int hex_to_codepoint(input::input& input, unsigned short size) {
        short digits[8];

        for (unsigned short i = 0; i < size; ++i) {
            digits[i] = xdigit_to_num(input.get());

            if (digits[i] == -1) {
                return -1;
            }
        }

        int codepoint = 0, multiplier = 1;

        for (unsigned short i = 0; i < size; ++i) {
            codepoint += digits[size - i - 1] * multiplier;
            multiplier *= 16;
        }

        if (!utf8proc_codepoint_valid(codepoint)) {
            return -1;
        }

        return codepoint;
    }

    u8char parse_escape(input::input& i) {
        u8char c = i.get(), codepoint;

        switch (c) {
            case '\'':
                codepoint = '\'';
                break;
            case '"':
                codepoint = '"';
                break;
            case '?':
                codepoint = '?';
                break;
            case '\\':
                codepoint = '\\';
                break;
            case 'a':
                codepoint = '\a';
                break;
            case 'b':
                codepoint = '\b';
                break;
            case 'f':
                codepoint = '\f';
                break;
            case 'n':
                codepoint = '\n';
                break;
            case 'r':
                codepoint = '\r';
                break;
            case 't':
                codepoint = '\t';
                break;
            case 'v':
                codepoint = '\v';
                break;
            case 'X':
                codepoint = hex_to_codepoint(i, 2);
                break;
            case 'u':
                codepoint = hex_to_codepoint(i, 4);
                break;
            case 'U':
                codepoint = hex_to_codepoint(i, 8);
                break;
            default:
                codepoint = -2;
        }

        return codepoint;
    }

}