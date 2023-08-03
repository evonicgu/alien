#ifndef ALIEN_LEXER_RULES_LEXER_H
#define ALIEN_LEXER_RULES_LEXER_H

#include <list>

#include "token.h"
#include "util/charutils.h"
#include "util/lexing.h"
#include "util/lexer.h"
#include "util/u8string.h"

namespace alien::lexer::rules {

    class lexer : public util::lexer<token_type> {
    public:
        lexer(input::input& i, std::list<util::u8string>& err)
            : util::lexer<token_type>(i, err) {}

        token_t* lex() override;

        static bool is_valid_context(const util::u8string& str);
    };

}

#endif //ALIEN_LEXER_RULES_LEXER_H