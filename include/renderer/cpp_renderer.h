#ifndef ALIEN_CPP_RENDERER_H
#define ALIEN_CPP_RENDERER_H

#include "inja/inja.hpp"

#include "base_renderer.h"
#include "alphabet.h"

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

        void render(inja::Environment& env, inja::json&& lexer_data, inja::json&& parser_data) override;

    private:
        void setup_types();

        void generate_lexer(inja::Environment& env, inja::json&& lexer_data);

        void generate_parser(inja::Environment& env, inja::json&& parser_data);

        inja::json lexer_config_to_json();

        inja::json parser_config_to_json();

        util::u8string get_lexer_relative_namespace() const;

        std::vector<util::u8string> get_parser_types(const util::u8string& relative_ns) const;
    };

}

#endif //ALIEN_CPP_RENDERER_H