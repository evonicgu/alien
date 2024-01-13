#ifndef ALIEN_CPP_RENDERER_H
#define ALIEN_CPP_RENDERER_H

#include <vector>

#include "inja/inja.hpp"
#include "nlohmann/json.hpp"

#include "base_renderer.h"
#include "alphabet.h"
#include "lexer/config/settings/settings.h"
#include "parser/config/settings/settings.h"
#include "util/u8string.h"

namespace alien::renderer {

    class cpp_renderer : public base_renderer {
        util::u8string real_token_type, real_position_type, real_symbol_type;
        const alphabet::alphabet& symbols;

    public:
        cpp_renderer(const config::generator_config& config,
                     const alphabet::alphabet& symbols,
                     lexer::settings::settings_t&& lexer_settings,
                     parser::settings::settings_t&& parser_settings)
            : base_renderer(config, std::move(lexer_settings), std::move(parser_settings)),
              symbols(symbols) {}

        void render(inja::Environment& env, nlohmann::json&& lexer_data, nlohmann::json&& parser_data) override;

    private:
        void setup_types();

        void generate_lexer(inja::Environment& env, nlohmann::json&& lexer_data);

        void generate_parser(inja::Environment& env, nlohmann::json&& parser_data);

        nlohmann::json lexer_config_to_json();

        nlohmann::json parser_config_to_json();

        util::u8string get_lexer_relative_namespace() const;

        std::vector<util::u8string> get_parser_types() const;
    };

}

#endif //ALIEN_CPP_RENDERER_H