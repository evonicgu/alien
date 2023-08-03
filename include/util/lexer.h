#ifndef ALIEN_UTIL_LEXER_H
#define ALIEN_UTIL_LEXER_H

#include <list>

#include "input/input.h"
#include "token.h"
#include "util/u8string.h"

namespace alien::util {

    template<typename T>
    class lexer {
    protected:
        input::input& i;
        using token_t = token<T>;
        using type = T;

        std::list<util::u8string>& err;

    public:
        explicit lexer(input::input& i, std::list<util::u8string>& err)
            : i(i), err(err) {}

        virtual token_t* lex() = 0;
    };

}

#endif //ALIEN_UTIL_LEXER_H