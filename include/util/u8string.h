#ifndef ALIEN_U8STRING_H
#define ALIEN_U8STRING_H

#include <array>
#include <stdexcept>
#include <string>

#include "utf8proc.h"

namespace alien::util {

    using u8char = utf8proc_int32_t;
    using u8string = std::basic_string<u8char>;
    using u8string_view = std::basic_string_view<u8char>;

    u8string ascii_to_u8string(const std::string& str);

    namespace literals {

        u8string operator""_u8(const char* str, std::size_t size);

    }

    void u8string_to_bytes(const u8string& str, std::string& out);

    std::string u8string_to_bytes(const u8string& str);

    long long u8_stoi(const u8string& str);

    util::u8string to_u8string(unsigned long long number);

}

#endif //ALIEN_U8STRING_H