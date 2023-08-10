#ifndef ALIEN_RENDERER_H
#define ALIEN_RENDERER_H

#include "config/generator_config.h"
#include "config/settings/settings.h"

namespace alien::renderer {

    struct language_info {
        std::string display_name;
        bool has_headers;
    };

    std::unordered_map<config::language, language_info> languages{
            {config::language::CPP, {"C++", true}}
    };

    class base_renderer {
        const config::generator_config& config;

        const config::settings::settings<lexer::settings::lexer_symbol>& lexer_settings;
        const config::settings::settings<parser::settings::parser_symbol>& parser_settings;

    public:
        base_renderer(const config::generator_config& config,
                      const config::settings::settings<lexer::settings::lexer_symbol>& lexer_settings,
                      const config::settings::settings<parser::settings::parser_symbol>& parser_settings)
            : config(config),
              lexer_settings(lexer_settings),
              parser_settings(parser_settings) {}

        virtual void check_configuration();

        virtual void render(inja::Environment& env, inja::json&& lexer_data, inja::json&& parser_data) = 0;

    private:
        static std::string error_message(const std::string& str, const language_info& info);
    };

}

#endif //ALIEN_RENDERER_H