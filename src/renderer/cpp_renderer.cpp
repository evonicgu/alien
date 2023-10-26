#include "renderer/cpp_renderer.h"

namespace alien::renderer {

    const char* LEXER_HEADER_TEMPLATE = "resources/templates/cpp/headers/lexer.template.txt";
    const char* LEXER_SOURCE_TEMPLATE = "resources/templates/cpp/sources/lexer.template.txt";
    const char* TOKEN_SOURCE_TEMPLATE = "resources/templates/cpp/sources/token.template.txt";
    const char* PARSER_HEADER_TEMPLATE = "resources/templates/cpp/headers/parser.template.txt";
    const char* PARSER_SOURCE_TEMPLATE = "resources/templates/cpp/sources/parser.template.txt";

    void merge_jsons_inplace(inja::json& first, const inja::json& second) {
        first.insert(second.begin(), second.end());
    }

    util::u8string ns_continuation(const util::u8string& ns) {
        using namespace util::literals;

        if (ns.size() >= 2) {

        }

        return ns.empty() ? util::u8string{} : ns + "::"_u8;
    }

    void cpp_renderer::render(inja::Environment& env, inja::json&& lexer_data, inja::json&& parser_data) {
        setup_types();

        generate_lexer(env, std::move(lexer_data));

        generate_parser(env, std::move(parser_data));
    }

    void cpp_renderer::setup_types() {
        using namespace util::literals;

        const auto& token_type = config::get_string_value(lexer_settings.config.at("generation.token_type"_u8));
        const auto& position_type = config::get_string_value(lexer_settings.config.at("generation.position_type"_u8));

        const auto& token_namespace = config::get_string_value(lexer_settings.config.at("generation.cpp.token_namespace"_u8));

        const auto token_namespace_continuation = ns_continuation(token_namespace);

        if (token_type == config::settings::default_t) {
            real_token_type = "token"_u8;
        } else {
            real_token_type = token_namespace_continuation;
            real_token_type += token_type;
        }

        real_token_type += "<token_type>"_u8;

        if (position_type == config::settings::default_t) {
            real_position_type = "position"_u8;
        } else {
            real_position_type = token_namespace_continuation;
            real_position_type += position_type;
        }

        const auto& symbol_type = config::get_string_value(parser_settings.config.at("generation.symbol_type"_u8));

        const auto& symbol_namespace = config::get_string_value(parser_settings.config.at("generation.cpp.symbol_namespace"_u8));

        const auto symbol_namespace_continuation = ns_continuation(symbol_namespace);

        real_symbol_type = symbol_namespace_continuation;
        real_symbol_type += symbol_type;
    }

    void cpp_renderer::generate_lexer(inja::Environment& env, inja::json&& lexer_data) {
        using namespace util::literals;

        auto lexer_options = lexer_config_to_json();

        bool is_header_only = config::get_bool_value(lexer_settings.config.at("generation.cpp.header_only"_u8));

        std::filesystem::path header_output_dir = config.header_output_directory.value();

        auto header_template = env.parse_template(config.lexer_header_template.value_or(LEXER_HEADER_TEMPLATE));
        auto source_template = env.parse_template(config.lexer_template.value_or(LEXER_SOURCE_TEMPLATE));
        auto token_template = env.parse_template(config.token_template.value_or(TOKEN_SOURCE_TEMPLATE));

        std::ofstream header_out(header_output_dir / "parser.gen.h");
        std::ofstream token_out(header_output_dir / "token.gen.h");

        auto combined_json = inja::json{
                {"data", std::move(lexer_data)},
                {"options", std::move(lexer_options)}
        };

        env.render_to(header_out, header_template, combined_json);
        env.render_to(token_out, token_template, combined_json);

        std::ofstream source_out;

        if (is_header_only) {
            source_out = std::move(header_output_dir);
        } else {
            std::filesystem::path source_output_dir = config.output_directory.value();

            source_out = std::ofstream(source_output_dir / "parser.gen.cpp");
        }

        env.render_to(source_out, source_template, combined_json);
    }

    void cpp_renderer::generate_parser(inja::Environment& env, inja::json&& parser_data) {
        using namespace util::literals;

        auto parser_options = parser_config_to_json();

        bool is_header_only = config::get_bool_value(lexer_settings.config.at("generation.cpp.header_only"_u8));

        std::filesystem::path header_output_dir = config.header_output_directory.value();

        auto header_template = env.parse_template(config.parser_header_template.value_or(PARSER_HEADER_TEMPLATE));
        auto source_template = env.parse_template(config.parser_template.value_or(PARSER_SOURCE_TEMPLATE));

        std::ofstream header_out(header_output_dir / "parser.gen.h", std::ios_base::app);

        auto combined_json = inja::json{
                {"data", std::move(parser_data)},
                {"options", std::move(parser_options)}
        };

        env.render_to(header_out, header_template, combined_json);

        std::ofstream source_out;

        if (is_header_only) {
            source_out = std::move(header_output_dir);
        } else {
            std::filesystem::path source_output_dir = config.output_directory.value();

            source_out = std::ofstream(source_output_dir / "parser.gen.cpp", std::ios_base::app);
        }

        env.render_to(source_out, source_template, combined_json);
    }

    inja::json cpp_renderer::lexer_config_to_json() {
        using namespace util::literals;

        auto base = base_lexer_config_to_json();

        auto cpp = inja::json{
                {"macros",                 config::get_bool_value(lexer_settings.config.at("generation.cpp.macros"_u8))},
                {"token_type",             real_token_type},
                {"position_type",          real_position_type},
                {"use_enum_class",         config::get_bool_value(lexer_settings.config.at("generation.cpp.enum_class"_u8))},
                {"guard_prefix",           util::u8string_to_bytes(
                        config::get_string_value(lexer_settings.config.at("general.cpp.guard_prefix"_u8)))},
                {"lexer_namespace",        util::u8string_to_bytes(
                        config::get_string_value(lexer_settings.config.at("generation.cpp.namespace"_u8)))},
                {"is_header_only",         config::get_bool_value(lexer_settings.config.at("generation.cpp.header_only"_u8))},
                {"path_to_header",         util::u8string_to_bytes(
                        config::get_string_value(lexer_settings.config.at("generation.cpp.path_to_header"_u8)))},
                {"no_default_constructor", config::get_bool_value(lexer_settings.config.at("generation.cpp.no_default_constructor"_u8))}
        };

        merge_jsons_inplace(base, cpp);

        return base;
    }

    inja::json cpp_renderer::parser_config_to_json() {
        using namespace util::literals;

        auto base = base_parser_config_to_json();

        auto relative_ns = get_lexer_relative_namespace();

        auto cpp = inja::json{
                {"symbol_type",              util::u8string_to_bytes(real_symbol_type)},
                {"types",                    get_parser_types(relative_ns)},
                {"guard_prefix",             util::u8string_to_bytes(
                        config::get_string_value(lexer_settings.config.at("general.cpp.guard_prefix"_u8)))},
                {"is_header_only",           config::get_bool_value(lexer_settings.config.at("generation.cpp.header_only"_u8))},
                {"parser_namespace",         util::u8string_to_bytes(
                        config::get_string_value(parser_settings.config.at("generation.cpp.namespace"_u8)))},
                {"no_default_constructor",   config::get_bool_value(parser_settings.config.at("generation.cpp.no_default_constructor"_u8))},
                {"lexer_relative_namespace", util::u8string_to_bytes(relative_ns)}
        };

        merge_jsons_inplace(base, cpp);

        return base;
    }

    util::u8string cpp_renderer::get_lexer_relative_namespace() const {
        using namespace util::literals;

        auto& lexer_namespace = config::get_string_value(lexer_settings.config.at("generation.cpp.namespace"_u8));
        auto& parser_namespace = config::get_string_value(parser_settings.config.at("generation.cpp.namespace"_u8));

        const std::size_t max_common_length = std::min(lexer_namespace.size(), parser_namespace.size());

        /**
         * Namespace point is a point that is either before or after each identifier in the namespace,
         * in other words, the start of the namespace, before every ::, after every ::, end of the namespace
         */
        std::size_t last_common_ns_point_pos = 0;
        bool prev_colon = false;

        for (std::size_t i = 0; i < max_common_length; ++i) {
            if (lexer_namespace[i] != parser_namespace[i]) {
                break;
            }

            if (lexer_namespace[i] == ':') {
                if (prev_colon) {
                    last_common_ns_point_pos = i + 1;
                } else {
                    last_common_ns_point_pos = i;
                }

                prev_colon = !prev_colon;
            }

            // the namespaces are equal
            if (i == max_common_length - 1) {
                last_common_ns_point_pos = max_common_length;
            }
        }

        util::u8string_view relative_namespace = util::u8string_view(lexer_namespace).substr(last_common_ns_point_pos);

        if (relative_namespace.starts_with("::"_u8)) {
            relative_namespace = relative_namespace.substr(2);
        }

        return ns_continuation((util::u8string) relative_namespace);
    }

    std::vector<util::u8string> cpp_renderer::get_parser_types(const util::u8string& relative_ns) const {
        using namespace util::literals;

        std::vector<util::u8string> types;
        types.reserve(symbols.terminals.size() + symbols.non_terminals.size());

        const auto& token_namespace = config::get_string_value(lexer_settings.config.at("generation.cpp.token_namespace"_u8));

        const auto token_namespace_continuation = ns_continuation(token_namespace);

        for (std::size_t i = 0; i < symbols.terminals.size(); ++i) {
            if (symbols.terminals[i].type == lexer::settings::default_token_typename) {
                types.push_back(relative_ns + "token_t"_u8);
            } else {
                types.push_back(token_namespace_continuation + symbols.terminals[i].type);
            }
        }

        const auto& symbol_namespace = config::get_string_value(parser_settings.config.at("generation.cpp.symbol_namespace"_u8));

        const auto symbol_namespace_continuation = ns_continuation(symbol_namespace);

        for (std::size_t i = 0; i < symbols.non_terminals.size(); ++i) {
            if (symbols.non_terminals[i].type != parser::settings::void_type) {
                types.push_back(symbol_namespace_continuation + symbols.non_terminals[i].type);
            } else {
                types.push_back(symbols.non_terminals[i].type);
            }
        }

        return types;
    }

}