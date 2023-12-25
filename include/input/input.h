#ifndef ALIEN_INPUT_H
#define ALIEN_INPUT_H

#include <fstream>
#include <stdexcept>
#include <memory>

#include "util/charutils.h"
#include "util/u8string.h"
#include "util/token.h"

namespace alien::input {

    using namespace util;

    class input {
    protected:
        util::pos position{1, 1};

    public:
        virtual u8char get() = 0;

        virtual u8char peek() = 0;

        util::pos get_pos() const;

        void set_pos(util::pos pos);
    };

    class string_input : public input {
        util::u8string str;

        std::size_t pos = 0, end;

    public:
        explicit string_input(const util::u8string& str)
            : str(str) {
            end = this->str.size();
        }

        util::u8char get() override;

        util::u8char peek() override;

        string_input(const string_input& other) = default;

        string_input(string_input&& other) = default;
    };

    class stream_input : public input {
        std::unique_ptr<std::istream> stream;
        bool eof = false;

        utf8proc_uint8_t cbuffer[32771]{};
        util::u8char buffer[32768]{};

        std::size_t unread = 0, max = 0, pos = 0;

    public:
        explicit stream_input(std::unique_ptr<std::istream>&& opened_stream)
            : stream(std::move(opened_stream)) {
            if (!stream) {
                throw std::runtime_error("Unable to open input stream");
            }
        }

        explicit stream_input(const std::string& path)
            : stream(std::make_unique<std::ifstream>(path)) {
            if (!stream) {
                throw std::runtime_error("Unable to open input stream");
            }
        }

        util::u8char get() override;

        util::u8char peek() override;

        stream_input(const stream_input&) = delete;

        stream_input(stream_input&&) noexcept = default;

    private:
        void fill();

        static bool is_continuation_byte(utf8proc_uint8_t byte);

        static bool is_ascii_byte(utf8proc_uint8_t byte);

        std::size_t get_str_size();
    };

}

#endif //ALIEN_INPUT_H