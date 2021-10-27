#ifndef ALIEN_INPUT_H
#define ALIEN_INPUT_H

#include <fstream>
#include <vector>
#include <stdexcept>
#include "util/u8string.h"

namespace alien::input {

//    class input {
//    public:
//        virtual char get() = 0;
//
//        virtual char peek() = 0;
//
//        input(const input&) = delete;
//
//        input() = default;
//    };
//
//    class string_input : public input {
//        std::string str;
//
//        unsigned int pos = 0, end;
//
//    public:
//        explicit string_input(const std::string& str) : str(str) {
//            end = this->str.size();
//        }
//
//        explicit string_input(std::string&& str) : str(str) {
//            end = this->str.size();
//        }
//
//        char get() override {
//            return pos == end ? -2 : str[pos++];
//        }
//
//        char peek() override {
//            return pos == end ? -2 : str[pos];
//        }
//    };
//
//    class stream_input : public input {
//        std::istream& stream;
//
//        char buffer[4096];
//        unsigned int pos, max;
//
//        bool end = false;
//
//    public:
//        explicit stream_input(std::istream& stream) : stream(stream) {
//            fill();
//        }
//
//        char get() override {
//            if (end) {
//                return -2;
//            }
//
//            if (pos == max) {
//                fill();
//            }
//
//            return buffer[pos++];
//        }
//
//        char peek() override {
//            if (end) {
//                return -2;
//            }
//
//            if (pos == max) {
//                fill();
//            }
//
//            return buffer[pos];
//        }
//
//    private:
//        void fill() {
//            stream.read(buffer, 4096);
//
//            pos = 0;
//            max = stream.gcount();
//
//            if (max == 0) {
//                end = true;
//                buffer[0] = -2;
//            }
//        }
//    };
    class input {
    public:
        virtual util::u8char get() = 0;

        virtual util::u8char peek() = 0;

        input(const input&) = delete;

        input() = default;
    };

    class string_input : public input {
        util::u8string str;

        unsigned int pos = 0, end;

    public:
        explicit string_input(const util::u8string& str) : str(str) {
            end = this->str.size();
        }

        explicit string_input(util::u8string&& str) : str(std::move(str)) {
            end = this->str.size();
        }

        util::u8char get() override {
            return pos == end ? -2 : str[pos++];
        }

        util::u8char peek() override {
            return pos == end ? -2 : str[pos];
        }
    };

    class stream_input : public input {
        std::ifstream& stream;
        bool eof = false;

        utf8proc_uint8_t cbuffer[16387];
        util::u8char buffer[16384];

        int unread = 0, max = 0, pos = 0;

    public:
        explicit stream_input(std::ifstream& stream) : stream(stream) {}

        util::u8char get() override {
            if (pos == max) {
                fill();
            }

            if (eof) {
                return -2;
            }

            return buffer[pos++];
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

            stream.read((char*) cbuffer + unread, (int) sizeof cbuffer - unread);

            int str_size = get_str_size();
            if (stream.eof()) {
                unread = 0;
            } else {
                unread = (int) sizeof cbuffer - str_size;
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

        int get_str_size() {
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