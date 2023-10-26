#ifndef ALIEN_LEXER_SETTINGS_PARSER_H
#define ALIEN_LEXER_SETTINGS_PARSER_H

#include "config/settings/parser.h"
#include "config/settings/settings.h"
#include "util/u8string.h"
#include "languages/base_language.h"

namespace alien::lexer::settings {

    class settings_parser : public config::settings::parser<lexer_symbol> {
        std::ptrdiff_t prec_level = 0;
        alphabet::alphabet& symbols;

    public:
        settings_parser(lexer_t& l, std::list<util::u8string>& err, alphabet::alphabet& symbols)
                : config::settings::parser<lexer_symbol>(l, err, symbols.terminals),
                  symbols(symbols) {
            setup_settings();
        }

        void add_language_settings(const std::unique_ptr<languages::base_language>& language);

    private:
        void setup_settings();

        void add_types() override;

        void specifier() override;

        void error(type expected, type got) override;
    };

}

#endif //ALIEN_LEXER_SETTINGS_PARSER_H
