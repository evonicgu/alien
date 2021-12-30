#ifndef ALIEN_INPUT_H
#define ALIEN_INPUT_H

#include <fstream>
#include <stdexcept>

#include "util/charutils.h"
#include "util/u8string.h"

namespace alien::input {

    using namespace util;

    class input {
    public:
        std::size_t line = 1, column = 1;

        virtual u8char get() = 0;

        virtual u8char peek() = 0;
    };

    class string_input : public input {
        const util::u8string& str;

        std::size_t pos = 0, end;

    public:
        explicit string_input(const util::u8string& str)
            : str(str) {
            end = this->str.size();
        }

        util::u8char get() override {
            if (pos == end) {
                return -2;
            }

            u8char c = str[pos++], c_class = util::get_class(c);

            if (c_class == -5 || c_class == -31 || c_class == -32) {
                ++line;
                column = 1;
            } else {
                ++column;
            }

            return c;
        }

        util::u8char peek() override {
            return pos == end ? -2 : str[pos];
        }
    };

    class stream_input : public input {
        std::ifstream& stream;
        bool eof = false;

        utf8proc_uint8_t cbuffer[32771];
        util::u8char buffer[8192];

        std::size_t unread = 0, max = 0, pos = 0;

    public:
        explicit stream_input(std::ifstream& stream)
            : stream(stream) {
            if (stream.fail() || stream.bad()) {
                throw std::runtime_error("Unable to open input stream");
            }
        }

        util::u8char get() override {
            if (pos == max) {
                fill();
            }

            if (eof) {
                return -2;
            }

            u8char c = buffer[pos++], c_class = get_class(c);

            if (c_class == -5 || c_class == -31 || c_class == -32) {
                ++line;
                column = 1;
            } else {
                ++column;
            }

            return c;
        }

        util::u8char peek() override {
            if (pos == max) {
                fill();
            }

            if (eof) {
                return -2;
            }

            return buffer[pos];
        }

    private:
        void fill() {
            if (stream.eof() && unread == 0) {
                eof = true;
                return;
            }

            stream.read((char*) cbuffer + unread, sizeof cbuffer - unread);

            std::size_t str_size = get_str_size();

            if (stream.eof()) {
                unread = 0;
            } else {
                unread = sizeof cbuffer - str_size;
            }

            pos = 0;
            max = utf8proc_decompose(cbuffer, str_size, buffer, sizeof buffer, utf8proc_option_t::UTF8PROC_REJECTNA);

            for (int i = 0; i < unread; ++i) {
                cbuffer[unread - i - 1] = cbuffer[sizeof cbuffer - i - 1];
            }

            if (max < 0) {
                throw std::runtime_error(utf8proc_errmsg(max));
            }
        }

        static bool is_continuation_byte(utf8proc_uint8_t byte) {
            return byte >> 6 == 2;
        }

        static bool is_ascii_byte(utf8proc_uint8_t byte) {
            return (byte & 0x80) == 0;
        }

        std::size_t get_str_size() {
            if (stream.eof()) {
                return unread + stream.gcount();
            }

            int str_size = sizeof cbuffer - 3;

            if (is_ascii_byte(cbuffer[str_size - 1])) {
                return str_size;
            }

            while (str_size < sizeof cbuffer && is_continuation_byte(cbuffer[str_size])) {
                ++str_size;
            }

            return str_size;
        }
    };

}

#endif //ALIEN_INPUT_H