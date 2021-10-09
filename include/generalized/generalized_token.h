#ifndef ALIEN_GENERALIZED_TOKEN_H
#define ALIEN_GENERALIZED_TOKEN_H

#include <type_traits>

namespace alien::generalized {

    template<typename T>
    struct generalized_token {
        static_assert(std::is_enum<T>(), "Template type must be an enum");

        T type;

        explicit generalized_token(T type) : type(type) {}

        virtual ~generalized_token() = default;
    };

}

#endif //ALIEN_GENERALIZED_TOKEN_H