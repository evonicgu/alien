#ifndef ALIEN_PARSER_RULES_LEXER_H
#define ALIEN_PARSER_RULES_LEXER_H

#include <list>

#include "input/input.h"
#include "token.h"
#include "util/charutils.h"
#include "util/lexer.h"
#include "util/lexing.h"
#include "util/u8string.h"

namespace alien::parser::rules {

    class lexer : public util::lexer<token_type> {
    public:
        lexer(input::input& i, std::list<util::u8string>& err)
            : util::lexer<token_type>(i, err) {}

        token_t* lex() override;
    };

}

#endif //ALIEN_PARSER_RULES_LEXER_H