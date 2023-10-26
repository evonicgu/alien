#ifndef ALIEN_PARSER_H
#define ALIEN_PARSER_H

#include "config/settings/parser.h"
#include "settings.h"
#include "languages/base_language.h"

namespace alien::parser::settings {

    class settings_parser : public config::settings::parser<parser_symbol> {
        alphabet::alphabet& symbols;

    public:
        settings_parser(lexer_t& l, std::list<util::u8string>& err, alphabet::alphabet& symbols)
                : config::settings::parser<parser_symbol>(l, err, symbols.non_terminals),
                  symbols(symbols) {
            setup_settings();
        }

        void add_language_settings(const std::unique_ptr<languages::base_language>& language);

    private:
        void setup_settings();

        void specifier() override;

        void add_types() override;

        void error(type expected, type got) override;
    };

}

#endif //ALIEN_PARSER_H
