#ifndef ALIEN_LEXER_RULES
#define ALIEN_LEXER_RULES

#include <map>
#include <vector>

#include "nlohmann/json.hpp"

#include "util/u8string.h"
#include "util/token.h"

namespace alien::lexer::rules {

    struct action {
        util::u8string code;

        util::u8string symbol;
    };

    struct rule {
        bool no_utf8 = false;

        util::u8string regex;

        action act;

        util::pos position;

        std::ptrdiff_t rule_number;
    };

    struct rules {
        std::vector<std::vector<rule>> ruleset;
        std::map<util::u8string, std::size_t> ctx;

        action on_eof;
    };

    void to_json(nlohmann::json& json, const action& act);
}

#endif //ALIEN_LEXER_RULES