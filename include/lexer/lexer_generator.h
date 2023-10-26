#ifndef ALIEN_LEXER_GENERATOR_H
#define ALIEN_LEXER_GENERATOR_H

#include "config/settings/settings.h"
#include "config/settings/token.h"
#include "config/generator_config.h"
#include "util/typeutils.h"
#include "util/u8string.h"
#include "config/settings/lexer.h"
#include "lexer/config/settings/settings.h"
#include "inja/inja.hpp"
#include "lexer/config/rules/rules.h"
#include "lexer/config/rules/lexer.h"
#include "lexer/config/rules/parser.h"
#include "lexer/automata/dfa.h"
#include "lexer/automata/generator.h"
#include "languages/base_language.h"
#include "lexer/config/settings/parser.h"

namespace alien::lexer {

    class lexer_generator {
        lexer::rules::rules lexer_rules;

        std::list<util::u8string>& err;
        alphabet::alphabet& alphabet;

        std::unique_ptr<languages::base_language>& language;

        input::stream_input& input_stream;

        bool no_utf8;

    public:
        lexer_generator(input::stream_input& input_stream,
                        std::unique_ptr<languages::base_language>& language,
                        alphabet::alphabet& alphabet,
                        std::list<util::u8string>& err)
                : input_stream(input_stream),
                  language(language),
                  alphabet(alphabet),
                  err(err) {}

        settings::settings_t parse_lexer_config();

        std::optional<inja::json> generate_lexer();

        std::vector<bool> get_default_token_type_info() const;
    };

}

#endif //ALIEN_LEXER_GENERATOR_H