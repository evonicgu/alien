#ifndef ALIEN_BASE_LANGUAGE_H
#define ALIEN_BASE_LANGUAGE_H

#include "lexer/config/settings/settings.h"
#include "parser/config/settings/settings.h"
#include "renderer/base_renderer.h"
#include "alphabet.h"

namespace alien::languages {

    class base_language {

    public:
        base_language(const config::generator_config& config, std::list<util::u8string>& err)
            : config(config),
              err(err) {}

        virtual void register_lexer_settings(lexer::settings::settings_t& settings) const = 0;

        virtual void register_parser_settings(parser::settings::settings_t& settings) const = 0;

        virtual std::optional<std::unique_ptr<renderer::base_renderer>> create_renderer(
                const alphabet::alphabet& symbols,
                lexer::settings::settings_t&& lexer_settings,
                parser::settings::settings_t&& parser_settings) = 0;

        virtual ~base_language() = default;

    protected:
        bool check_base_configuration(const lexer::settings::settings_t& lexer_settings,
                                      const parser::settings::settings_t& parser_settings);

        const config::generator_config& config;
        std::list<util::u8string>& err;
    };

}

#endif //ALIEN_BASE_LANGUAGE_H