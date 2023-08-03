#ifndef ALIEN_LEXER_SETTINGS_H
#define ALIEN_LEXER_SETTINGS_H

#include <list>
#include <memory>
#include <stdexcept>
#include <string>

#include "config/settings/parser.h"
#include "config/settings/settings.h"
#include "util/u8string.h"

namespace alien::lexer::settings {

    struct lexer_symbol {
        util::u8string name, type;
        bool default_type;

        std::ptrdiff_t prec = -1, assoc = -1;

        bool operator<(const lexer_symbol& other) const;
    };

    const auto default_token_typename = util::ascii_to_u8string("token_t@default");

    class settings_parser : public config::settings::parser<lexer_symbol> {
        std::ptrdiff_t prec_level = 0;

    public:
        bool token_default = false, position_default = false;

        settings_parser(lexer_t& l, std::list<util::u8string>& err)
            : config::settings::parser<lexer_symbol>(l, err) {
            setup_settings();
        }

    private:
        void setup_settings();

        void add_types() override;

        void specifier() override;

        void error(type expected, type got) override;
    };

}

#endif //ALIEN_LEXER_SETTINGS_H