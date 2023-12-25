#include "util/u8string.h"

namespace alien::util {

    u8string ascii_to_u8string(const std::string& str) {
        u8string out;
        out.reserve(str.size());

        for (std::size_t i = 0; i < str.size(); ++i) {
            if (str[i] < 0) {
                throw std::invalid_argument(std::string("Invalid ASCII char at index ") + std::to_string(i));
            }
            out.push_back(str[i]);
        }

        return out;
    }

    namespace literals {

        u8string operator ""_u8(const char* str, std::size_t size) {
            return ascii_to_u8string({str, size});
        }
    }

    void u8string_to_bytes(const u8string& str, std::string& out) {
        utf8proc_uint8_t buffer[5];

        for (std::size_t i = 0; i < str.size(); ++i) {
            if (str[i] == -2) {
                out += "EOF";
                continue;
            }

            if (!utf8proc_codepoint_valid(str[i])) {
                throw std::invalid_argument(std::string("Invalid utf-8 codepoint at index ") + std::to_string(i));
            }

            utf8proc_size_t size = utf8proc_encode_char(str[i], buffer);
            buffer[size] = 0;

            out += (char*) buffer;
        }
    }

    std::string u8string_to_bytes(const u8string& str) {
        std::string bytes;

        u8string_to_bytes(str, bytes);

        return bytes;
    }

    long long u8_stoi(const u8string& str) {
        if (str.empty()) {
            throw std::invalid_argument("String cannot be empty");
        }

        long long number = 0;
        short multiplier = 1;
        std::size_t start = 0;

        if (str[0] == '-') {
            start = 1;
            multiplier = -1;
        }

        for (std::size_t i = start; i < str.size(); ++i) {
            if (!isdigit(str[i])) {
                throw std::invalid_argument("Invalid integer string");
            }

            number = number * 10 + (str[i] - '0') * multiplier;
        }

        return number;
    }

    util::u8string to_u8string(unsigned long long int number) {
        using namespace literals;

        if (number == 0) {
            return "0"_u8;
        }

        std::array<u8char, 26> symbols{};
        std::size_t filled = 0;

        while (number != 0) {
            symbols[filled++] = (u8char) number % 10 + '0';
            number /= 10;
        }

        util::u8string str;
        str.resize(filled);

        for (std::size_t i = 0; i < filled; ++i) {
            str[filled - i - 1] = symbols[i];
        }

        return str;
    }

}