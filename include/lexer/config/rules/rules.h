#ifndef ALIEN_RULES_H
#define ALIEN_RULES_H

#include <string>
#include <utility>
#include <vector>

namespace alien::lexer::config::rules {

    struct action {
        std::string code;

        std::string trailing_return;
    };

    struct rule {
        std::string regex;
        action act;
        int rule_number;
    };

    struct rules {
        std::vector<std::vector<rule>> ruleset;
        std::map<std::string, unsigned int> context_mapping;

        action eof_action;
    };

}

#endif //ALIEN_RULES_H