#include "languages/cpp_language.h"
#include "renderer/cpp_renderer.h"

namespace alien::languages {

    void cpp_language::register_lexer_settings(lexer::settings::settings_t& settings) const {
        using namespace util::literals;

        std::pair<util::u8string, std::unique_ptr<config::settings::value>> values[] = {
                {"generation.cpp.header_only"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.cpp.macros"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.cpp.enum_class"_u8, std::make_unique<config::settings::bool_value>(true)},
                {"generation.cpp.namespace"_u8, std::make_unique<config::settings::string_value>("lexer"_u8)},
                {"generation.cpp.no_default_constructor"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"general.cpp.guard_prefix"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"generation.cpp.path_to_header"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"generation.cpp.token_namespace"_u8, std::make_unique<config::settings::string_value>(""_u8)},
        };

        for (auto& value : values) {
            settings.config.insert(std::move(value));
        }
    }

    void cpp_language::register_parser_settings(parser::settings::settings_t& settings) const {
        using namespace util::literals;

        std::pair<util::u8string, std::unique_ptr<config::settings::value>> values[] = {
                {"generation.cpp.symbol_namespace"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"generation.cpp.namespace"_u8, std::make_unique<config::settings::string_value>("parser"_u8)},
                {"generation.cpp.no_default_constructor"_u8, std::make_unique<config::settings::bool_value>(false)},
        };

        for (auto& value : values) {
            settings.config.insert(std::move(value));
        }
    }

    bool cpp_language::check_cpp_configuration(const lexer::settings::settings_t& lexer_settings,
                                               const parser::settings::settings_t& parser_settings) {
        using namespace util::literals;

        bool correct = true;

        if (config.header_output_directory->empty()) {
            err.push_back("Header output directory must be set"_u8);

            correct = false;
        }

        auto monomorphic = util::check<config::settings::bool_value>(
                lexer_settings.config.at("generation.monomorphic"_u8).get()
        )->val;

        auto header_only = util::check<config::settings::bool_value>(
                lexer_settings.config.at("generation.cpp.header_only"_u8).get()
        )->val;

        auto path_to_header = util::check<config::settings::string_value>(
                lexer_settings.config.at("generation.cpp.path_to_header"_u8).get()
        )->str;

        if (header_only) {
            if (!path_to_header.empty()) {
                err.push_back("Cannot set 'path_to_header' option in header-only mode"_u8);

                correct = false;
            }

            if (config.output_directory.has_value()) {
                err.push_back("Cannot set source output directory in header-only mode"_u8);

                correct = false;
            }
        } else {
            if (!monomorphic) {
                err.push_back("Polymorphic parsers and lexers require header-only mode"_u8);

                correct = false;
            }

            if (path_to_header.empty()) {
                err.push_back("Option 'path_to_header' is required in non header-only mode"_u8);

                correct = false;
            }
        }

        return correct;
    }

    std::optional<std::unique_ptr<renderer::base_renderer>> cpp_language::create_renderer(
            const alphabet::alphabet& symbols,
            lexer::settings::settings_t&& lexer_settings,
            parser::settings::settings_t&& parser_settings) {
        if (!check_base_configuration(lexer_settings, parser_settings) ||
            !check_cpp_configuration(lexer_settings, parser_settings)) {
            return {};
        }

        return std::make_unique<renderer::cpp_renderer>(config, symbols, std::move(lexer_settings), std::move(parser_settings));
    }

}