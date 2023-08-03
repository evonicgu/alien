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
#include <filesystem>

namespace alien::lexer {

    class lexer_generator {
        using settings_t = config::settings::settings<lexer::settings::lexer_symbol>;

        lexer::rules::rules lexer_rules;

        bool token_type_default = false, position_type_default = false;

        std::list<util::u8string>& err;
        alphabet::alphabet& alphabet;

        const config::generator_config& generator_config;

        config::generator_streams& generator_streams;

        settings_t lexer_settings;

    public:
        lexer_generator(const config::generator_config& generator_config,
                        config::generator_streams& generator_streams,
                        alphabet::alphabet& alphabet,
                        std::list<util::u8string>& err)
                : generator_config(generator_config),
                  generator_streams(generator_streams),
                  alphabet(alphabet),
                  err(err) {}

        const std::unique_ptr<config::settings::value>& get_param(const util::u8string& param) const;

        void parse_lexer_config();

        void generate_lexer(inja::Environment& env, const util::u8string& guard_prefix);

    private:
        static bool get_value(const std::unique_ptr<config::settings::value>& ptr);

        void check_monomorphization_config() const;
    };

}

#endif //ALIEN_LEXER_GENERATOR_H