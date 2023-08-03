#ifndef ALIEN_PARSER_SETTING_H
#define ALIEN_PARSER_SETTING_H

#include <list>

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

    class settings_parser : public config::settings::parser<parser_symbol> {
    public:
        settings_parser(lexer_t& l, std::list<util::u8string>& err)
            : config::settings::parser<parser_symbol>(l, err) {
            setup_settings();
        }

    private:
        void setup_settings();

        void specifier() override;

        void add_types() override;

        void error(type expected, type got) override;
    };

}

#endif //ALIEN_PARSER_SETTING_H