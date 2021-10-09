#ifndef ALIEN_GENERALIZED_LEXER_H
#define ALIEN_GENERALIZED_LEXER_H

#include "generalized exception.h"
#include "generalized_token.h"
#include "input/input.h"

namespace alien::generalized {

    static constexpr char lexer_exception_str[] = "Unable to tokenize input. ";

    template<typename T>
    class generalized_lexer {
    protected:
        input::input& i;

    public:
        using token = generalized_token<T>;

        explicit generalized_lexer(input::input& i) : i(i) {}

        using lexer_exception = generalized_exception<lexer_exception_str>;

        virtual token* lex() = 0;
    };

}

#endif //ALIEN_GENERALIZED_LEXER_H