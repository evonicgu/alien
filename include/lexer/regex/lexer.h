#ifndef ALIEN_REGEX_LEXER_H
#define ALIEN_REGEX_LEXER_H

#include <list>
#include <stdexcept>

#include "token.h"
#include "util/lexer.h"
#include "util/u8string.h"
#include "util/lexing.h"

namespace alien::lexer::regex {

    class lexer : public util::lexer<token_type> {
    public:
        explicit lexer(input::input& i, std::list<util::u8string>& err)
            : util::lexer<token_type>(i, err) {}

        token_t* lex() override;

    private:
        util::u8char get_class_char();
    };

}

#endif //ALIEN_REGEX_LEXER_H