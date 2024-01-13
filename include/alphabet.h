#ifndef ALIEN_ALPHABET_H
#define ALIEN_ALPHABET_H

#include "lexer/config/settings/settings.h"
#include "parser/config/settings/settings.h"
#include "util/vecset.h"

namespace alien::alphabet {

    struct alphabet {
        util::vecset<lexer::settings::lexer_symbol> terminals;
        util::vecset<parser::settings::parser_symbol> non_terminals;
    };

}

#endif //ALIEN_ALPHABET_H