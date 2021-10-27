#ifndef ALIEN_UTIL_U8STRING_H
#define ALIEN_UTIL_U8STRING_H

#include <stdexcept>
#include <string>
#include "utf8proc.h"

namespace alien::util {

    using u8char = utf8proc_int32_t;
    using u8string = std::basic_string<u8char>;

    u8string ascii_to_u8string(const std::string& str) {
        u8string out;
        out.reserve(str.size());

        for (unsigned char c: str) {
            if (c > 127) {
                throw std::invalid_argument("Not a valid ASCII character");
            }

            out.push_back(c);
        }

        return out;
    }

    namespace literals {

        u8string operator"" _u8(const char* str, unsigned int size) {
            return ascii_to_u8string(std::string(str, size));
        }

    }

    std::string u8string_to_bytes(const u8string& str) {
        std::string byte_str;
        byte_str.reserve(str.size() * 4);

        utf8proc_uint8_t buffer[5];

        for (u8char c : str) {
            utf8proc_ssize_t c_size = utf8proc_encode_char(c, buffer);

            if (c_size == 0) {
                throw std::invalid_argument("Invalid utf-8 codepoint");
            }

            buffer[c_size] = 0;
            byte_str += (char*) buffer;
        }

        return byte_str;
    }

    void u8string_to_bytes(const u8string& str, std::string& out) {
        out = u8string_to_bytes(str);
    }

    int _u8_stoi(const u8string& str, unsigned int pos) {
        if (!isdigit(str[pos])) {
            throw std::invalid_argument("String must consist of digits");
        }

        if (pos == str.size() - 1) {
            return str[pos];
        }

        return _u8_stoi(str, pos + 1) * 10 + str[pos] - '0';
    }

    int u8_stoi(const u8string& str) {
        if (str.empty()) {
            throw std::invalid_argument("String must not be empty");
        }

        return _u8_stoi(str, 0);
    }

}

#endif //ALIEN_UTIL_U8STRING_H