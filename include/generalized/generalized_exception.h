#ifndef ALIEN_GENERALIZED_EXCEPTION_H
#define ALIEN_GENERALIZED_EXCEPTION_H

#include <exception>
#include <string>
#include "util/u8string.h"

namespace alien::generalized {

    template<const char* Str>
    class generalized_exception : public std::exception {
        std::string byte_str;

        void initialize(const util::u8string& description) {
            util::u8string_to_bytes(util::ascii_to_u8string(Str) + description, byte_str);
        }

        void initialize(util::u8string&& description) {
            util::u8string_to_bytes(util::ascii_to_u8string(Str) + std::move(description), byte_str);
        }

    public:
        explicit generalized_exception() = default;

        explicit generalized_exception(const util::u8string& description) {
            initialize(description);
        }

        explicit generalized_exception(util::u8string& description) {
            initialize(description);
        }

        const char* what() const noexcept override {
            return byte_str.c_str();
        }
    };

}

#endif //ALIEN_GENERALIZED_EXCEPTION_H