#ifndef ALIEN_CPP_LANGUAGE_H
#define ALIEN_CPP_LANGUAGE_H

#include "base_language.h"

namespace alien::languages {

    class cpp_language : public base_language {
    public:
        cpp_language(const config::generator_config& config, std::list<util::u8string>& err)
            : base_language(config, err) {}

        void register_lexer_settings(lexer::settings::settings_t& settings) const override;

        void register_parser_settings(parser::settings::settings_t& settings) const override;

        std::optional<std::unique_ptr<renderer::base_renderer>> create_renderer(
                const alphabet::alphabet& symbols,
                lexer::settings::settings_t&& lexer_settings,
                parser::settings::settings_t&& parser_settings) override;

    private:
        bool check_cpp_configuration(const lexer::settings::settings_t& lexer_settings,
                                     const parser::settings::settings_t& parser_settings);
    };

}

#endif //ALIEN_CPP_LANGUAGE_H