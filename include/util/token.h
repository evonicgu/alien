#ifndef ALIEN_UTIL_TOKEN_H
#define ALIEN_UTIL_TOKEN_H

#include <string>
#include <type_traits>

#include "util/u8string.h"

namespace alien::util {

    struct pos {
        std::size_t line, column;

        explicit operator std::string() const {
            return std::to_string(line) + ':' + std::to_string(column);
        }

        explicit operator u8string() const {
            return to_u8string(line) + (u8char) ':' + to_u8string(column);
        }
    };

    template<typename Ts>
    struct token {
        static_assert(std::is_enum_v<Ts>, "Template parameter must be an enumeration");

        pos start, end;
        Ts type;

        token(Ts type, pos start, pos end)
            : type(type),
              start(start),
              end(end) {}

        virtual ~token() = default;
    };

}

#endif //ALIEN_UTIL_TOKEN_H