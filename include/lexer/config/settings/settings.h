#ifndef ALIEN_LEXER_SETTINGS_H
#define ALIEN_LEXER_SETTINGS_H

#include <list>
#include <memory>
#include <stdexcept>
#include <string>

#include "config/settings/settings.h"
#include "util/u8string.h"

namespace alien::lexer::settings {

    struct lexer_symbol {
        util::u8string name, type;
        bool default_type;

        std::ptrdiff_t prec = -1, assoc = -1;

        bool operator<(const lexer_symbol& other) const;
    };

    using settings_t = config::settings::settings<lexer::settings::lexer_symbol>;

    const util::u8string default_token_typename = util::ascii_to_u8string("token_t@default");

}

#endif //ALIEN_LEXER_SETTINGS_H