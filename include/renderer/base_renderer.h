#ifndef ALIEN_RENDERER_H
#define ALIEN_RENDERER_H

#include <string>

#include "inja/inja.hpp"
#include "nlohmann/json.hpp"

#include "config/generator_config.h"
#include "config/settings/settings.h"
#include "lexer/config/settings/settings.h"
#include "parser/config/settings/settings.h"

namespace alien::renderer {

    class base_renderer {
    protected:
        const config::generator_config& config;

        lexer::settings::settings_t lexer_settings;
        parser::settings::settings_t parser_settings;

    public:
        base_renderer(const config::generator_config& config,
                      lexer::settings::settings_t&& lexer_settings,
                      parser::settings::settings_t&& parser_settings)
            : config(config),
              lexer_settings(std::move(lexer_settings)),
              parser_settings(std::move(parser_settings)) {}

        virtual void render(inja::Environment& env, nlohmann::json&& lexer_data, nlohmann::json&& parser_data) = 0;

        virtual ~base_renderer() = default;

        const lexer::settings::settings_t& get_lexer_config() const;

        const parser::settings::settings_t& get_parser_config() const;

    protected:
        nlohmann::json base_lexer_config_to_json();

        nlohmann::json base_parser_config_to_json();

        static std::string error_message(const std::string& str, const std::string& display_name);
    };

}

#endif //ALIEN_RENDERER_H