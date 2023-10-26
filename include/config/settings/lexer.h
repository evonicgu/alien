#ifndef ALIEN_SETTINGS_LEXER_H
#define ALIEN_SETTINGS_LEXER_H

#include <list>

#include "token.h"
#include "util/charutils.h"
#include "util/lexer.h"
#include "util/lexing.h"
#include "util/u8string.h"

namespace alien::config::settings {

    using namespace util::literals;

    class lexer : public util::lexer<token_type> {
    public:
        explicit lexer(input::input& i, std::list<util::u8string>& err)
            : util::lexer<token_type>(i, err) {}

        token_t* lex() override;
    };



}

#endif //ALIEN_SETTINGS_LEXER_H