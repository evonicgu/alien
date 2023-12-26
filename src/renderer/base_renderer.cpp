#include "renderer/base_renderer.h"

#include "config/config_value_access.h"
#include "util/u8string.h"

namespace alien::renderer {

    const lexer::settings::settings_t& base_renderer::get_lexer_config() const {
        return lexer_settings;
    }

    const parser::settings::settings_t& base_renderer::get_parser_config() const {
        return parser_settings;
    }

    nlohmann::json base_renderer::base_lexer_config_to_json() {
        using namespace util::literals;

        bool no_utf8 = config::get_bool_value(lexer_settings.config.at("generation.noutf8"_u8));

        return {
                {"no_utf8", no_utf8},
                {"code_headers",              std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::HEADERS))},
                {"code_decl",                 std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::DECL))},
                {"code_impl",                 std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::IMPL))},
                {"code_content_decl_private", std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::CONTENT_DECL_PRIVATE))},
                {"code_content_decl_public",  std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::CONTENT_DECL_PUBLIC))},
                {"code_content_impl",         std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::CONTENT_IMPL))},
                {"emit_stream",               !no_utf8 && config::get_bool_value(lexer_settings.config.at("generation.emit_stream"_u8))},
                {"token_default",             config::get_string_value(lexer_settings.config.at("generation.token_type"_u8)) == config::settings::default_t},
                {"position_default",          config::get_string_value(lexer_settings.config.at("generation.position_type"_u8)) == config::settings::default_t},
                {"track_lines",               config::get_bool_value(lexer_settings.config.at("generation.track_lines"_u8))},
                {"buffer_size",               config::get_number_value(lexer_settings.config.at("generation.buffer_size"_u8))},
                {"lexeme_size",               config::get_number_value(lexer_settings.config.at("generation.lexeme_size"_u8))},
                {"custom_error",              config::get_bool_value(lexer_settings.config.at("generation.custom_error"_u8))},
                {"monomorphic",               config::get_bool_value(lexer_settings.config.at("generation.monomorphic"_u8))},
                {"stream_type",               config::get_string_value(lexer_settings.config.at("generation.stream_type"_u8))}
        };
    }

    nlohmann::json base_renderer::base_parser_config_to_json() {
        using namespace util::literals;

        return {
                {"code_headers",              std::move(parser_settings.code_declarations[config::settings::code_token::location::HEADERS])},
                {"code_decl",                 std::move(parser_settings.code_declarations[config::settings::code_token::location::DECL])},
                {"code_impl",                 std::move(parser_settings.code_declarations[config::settings::code_token::location::IMPL])},
                {"code_content_decl_private", std::move(parser_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PRIVATE])},
                {"code_content_decl_public",  std::move(parser_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PUBLIC])},
                {"code_content_impl",         std::move(parser_settings.code_declarations[config::settings::code_token::location::CONTENT_IMPL])},
                {"custom_error",              config::get_bool_value(parser_settings.config.at("generation.custom_error"_u8))},
                {"use_token_to_str",          config::get_bool_value(parser_settings.config.at("generation.use_token_to_str"_u8))},
                {"default_token_to_str",      config::get_bool_value(parser_settings.config.at("generation.default_token_to_str"_u8))},
                {"monomorphic",               config::get_bool_value(lexer_settings.config.at("generation.monomorphic"_u8))},
                {"track_lines",               config::get_bool_value(lexer_settings.config.at("generation.track_lines"_u8))},
        };
    }

    std::string base_renderer::error_message(const std::string& str, const std::string& display_name) {
        return str + " for " + display_name;
    }

}