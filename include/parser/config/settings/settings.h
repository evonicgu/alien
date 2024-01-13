#ifndef ALIEN_PARSER_SETTING_H
#define ALIEN_PARSER_SETTING_H

#include "config/settings/parser.h"
#include "config/settings/settings.h"
#include "util/u8string.h"

namespace alien::parser::settings {

    struct parser_symbol {
        util::u8string name, type;

        bool is_first = false, is_midrule = false;

        bool operator<(const parser_symbol& other) const;

        friend bool operator<(const parser_symbol& lhs, const util::u8string& rhs);

        friend bool operator<(const util::u8string& lhs, const parser_symbol& rhs);
    };

    const util::u8string void_type = util::ascii_to_u8string("void@default");

    using settings_t = config::settings::settings<parser_symbol>;

}

#endif //ALIEN_PARSER_SETTING_H