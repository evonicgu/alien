#include "input/input.h"

namespace alien::input {

    util::u8char string_input::get() {
        if (pos == end) {
            return -2;
        }

        u8char c = str[pos++], c_class = util::get_class(c);

        if (c_class == -5 || c_class == -31 || c_class == -32) {
            ++position.line;
            position.column = 1;
        } else {
            ++position.column;
        }

        return c;
    }

    util::u8char string_input::peek() {
        return pos == end ? -2 : str[pos];
    }

    util::u8char stream_input::get() {
        if (pos == max) {
            fill();
        }

        if (eof) {
            return -2;
        }

        u8char c = buffer[pos++], c_class = get_class(c);

        if (c_class == -5 || c_class == -31 || c_class == -32) {
            ++position.line;
            position.column = 1;
        } else {
            ++position.column;
        }

        return c;
    }

    std::size_t stream_input::get_str_size() {
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

    void stream_input::fill() {
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

    util::u8char stream_input::peek() {
        if (pos == max) {
            fill();
        }

        if (eof) {
            return -2;
        }

        return buffer[pos];
    }

    bool stream_input::is_continuation_byte(utf8proc_uint8_t byte) {
        return byte >> 6 == 2;
    }

    bool stream_input::is_ascii_byte(utf8proc_uint8_t byte) {
        return (byte & 0x80) == 0;
    }

    util::pos input::get_pos() const {
        return position;
    }

    void input::set_pos(util::pos pos) {
        position = pos;
    }
}