#ifndef ALIEN_LEXER_CONFIG_RULES_H
#define ALIEN_LEXER_CONFIG_RULES_H

#include <map>
#include <vector>
#include "util/u8string.h"

namespace alien::lexer::config::rules {

    struct action {
        util::u8string code;

        util::u8string trailing_return;
    };

    struct rule {
        util::u8string regex;
        action act;
        int rule_number;
    };

    struct rules {
        std::vector<std::vector<rule>> ruleset;
        std::map<util::u8string, unsigned int> context_mapping;

        action eof_action;
    };

}

#endif //ALIEN_LEXER_CONFIG_RULES_H