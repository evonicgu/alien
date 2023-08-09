#include "lexer/lexer_generator.h"

namespace alien::lexer {

    void lexer_generator::parse_lexer_config() {
        using namespace util::literals;

        config::settings::lexer settings_lexer(generator_streams.in, err);
        lexer::settings::settings_parser settings_parser(settings_lexer, err);

        settings_parser.parse();

        lexer_settings = settings_parser.get_settings();

        check_monomorphization_config();

        token_type_default = settings_parser.token_default;
        position_type_default = settings_parser.position_default;

        alphabet.terminals.push_back({
                                             "error"_u8,
                                             settings::default_token_typename
                                     });

        for (std::size_t i = 0; i < lexer_settings.symbols.size(); ++i) {
            lexer::settings::lexer_symbol symbol = std::move(lexer_settings.symbols[i]);

            alphabet.terminals.push_back(std::move(symbol));
        }

        lexer::rules::lexer rules_lexer(generator_streams.in, err);
        lexer::rules::parser rules_parser(rules_lexer, err, alphabet);

        rules_parser.parse();

        lexer_rules = rules_parser.get_rules();
    }

    void lexer_generator::generate_lexer(inja::Environment& env, const util::u8string& guard_prefix) {
        using namespace util::literals;

        std::vector<std::size_t> ctx_start_states;
        std::vector<lexer::rules::action> actions;
        std::vector<lexer::automata::dfa::dfa> automations;

        bool no_utf8 = get_value(lexer_settings.config["generation.noutf8"_u8]);
        bool has_any_start_transitions = false;
        std::size_t current_states = 0;

        if (alphabet.terminals.size() == 0) {
            err.push_back("Cannot create lexer: no terminals are defined"_u8);
        }

        for (auto& rules: lexer_rules.ruleset) {
            if (rules.empty()) {
                err.push_back("Cannot define empty contexts"_u8);

                continue;
            }

            lexer::automata::generator gen(err, no_utf8);

            if (rules[0].rule_number >= actions.size()) {
                actions.resize(rules.back().rule_number + 1);
            }

            for (auto& rule: rules) {
                actions[rule.rule_number] = std::move(rule.act);
            }

            automations.push_back(gen.generate_automata(rules));
            ctx_start_states.push_back(automations.back().start_state + current_states);
            current_states += automations.back().states.size();

            has_any_start_transitions = has_any_start_transitions || automations.back().transitions_to_start;
        }

        if (!err.empty()) {
            return;
        }

        long long buffer_size = util::check<config::settings::number_value>(
                lexer_settings.config["generation.buffer_size"_u8].get()
        )->number;
        long long lexeme_size = util::check<config::settings::number_value>(
                lexer_settings.config["generation.lexeme_size"_u8].get()
        )->number;

        if (buffer_size <= 0 || lexeme_size <= 0) {
            throw std::runtime_error("Buffer or lexeme size must be greater than zero");
        }

        util::u8string token_type = util::check<config::settings::string_value>(
                lexer_settings.config["generation.token_type"_u8].get()
        )->str + "<token_type>"_u8;

        util::u8string position_type = std::move(util::check<config::settings::string_value>(
                lexer_settings.config["generation.position_type"_u8].get()
        )->str);

        util::u8string lexer_namespace = std::move(util::check<config::settings::string_value>(
                lexer_settings.config["generation.namespace"_u8].get()
        )->str);

        util::u8string stream_type = std::move(util::check<config::settings::string_value>(
                lexer_settings.config.at("generation.stream_type"_u8).get()
        )->str);

        std::string path_to_header = util::u8string_to_bytes(util::check<config::settings::string_value>(
                lexer_settings.config.at("generation.cpp.path_to_header"_u8).get()
        )->str);

        if (path_to_header.empty() && !generator_config.header_only) {
            path_to_header = std::filesystem::relative(
                    generator_config.header_output_directory,
                    generator_config.output_directory.value()
            ) / "parser.gen.h";
        }

        inja::json data{
                {"no_utf8",                   no_utf8},
                {"code_headers",              std::move(lexer_settings.code_declarations[config::settings::code_token::location::HEADERS])},
                {"code_decl",                 std::move(lexer_settings.code_declarations[config::settings::code_token::location::DECL])},
                {"code_impl",                 std::move(lexer_settings.code_declarations[config::settings::code_token::location::IMPL])},
                {"code_content_decl_private", std::move(lexer_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PRIVATE])},
                {"code_content_decl_public",  std::move(lexer_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PUBLIC])},
                {"code_content_impl",         std::move(lexer_settings.code_declarations[config::settings::code_token::location::CONTENT_IMPL])},
                {"contexts",                  util::to_json(lexer_rules.ctx)},
                {"macros",                    get_value(lexer_settings.config["generation.cpp.macros"_u8])},
                {"emit_stream",               !no_utf8 &&
                                              get_value(lexer_settings.config["generation.emit_stream"_u8])},
                {"token_default",             token_type_default},
                {"position_default",          position_type_default},
                {"token_type",                std::move(token_type)},
                {"position_type",             std::move(position_type)},
                {"use_enum_class",            get_value(lexer_settings.config["generation.cpp.enum_class"_u8])},
                {"ctx_start_states",          std::move(ctx_start_states)},
                {"has_any_start_transitions", has_any_start_transitions},
                {"symbols",                   util::to_json(alphabet.terminals,
                                                            [](const lexer::settings::lexer_symbol& symbol) {
                                                                return util::u8string_to_bytes(symbol.name);
                                                            })},
                {"guard_prefix",              guard_prefix},
                {"track_lines",               get_value(lexer_settings.config["generation.track_lines"_u8])},
                {"buffer_size",               buffer_size},
                {"lexeme_size",               lexeme_size},
                {"actions",                   std::move(actions)},
                {"on_eof",                    std::move(lexer_rules.on_eof)},
                {"automations",               automations},
                {"custom_error",              get_value(lexer_settings.config["generation.custom_error"_u8])},
                {"lexer_namespace",           std::move(lexer_namespace)},
                {"monomorphic",               get_value(lexer_settings.config["generation.monomorphic"_u8])},
                {"stream_type",               std::move(stream_type)},
                {"is_header_only",            generator_config.header_only},
                {"path_to_header",            path_to_header},
                {"no_default_constructor",    get_value(
                        lexer_settings.config["generation.cpp.no_default_constructor"_u8])}
        };

        inja::Template token_tmpl = env.parse_file(generator_config.token_template);
        env.render_to(generator_streams.token_out, token_tmpl, data);

        inja::Template header_tmpl = env.parse_file(generator_config.lexer_header_template);
        env.render_to(generator_streams.parser_header_out, header_tmpl, data);

        inja::Template source_tmpl = env.parse_file(generator_config.lexer_template);

        auto& out_stream = generator_config.header_only ?
                           generator_streams.parser_header_out :
                           generator_streams.parser_source_out.value();

        env.render_to(out_stream, source_tmpl, data);
    }

    void lexer_generator::check_monomorphization_config() const {
        using namespace util::literals;

        auto& stream_type = util::check<config::settings::string_value>(
                lexer_settings.config.at("generation.stream_type"_u8).get()
        )->str;

        auto monomorphic = get_value(lexer_settings.config.at("generation.monomorphic"_u8));

        auto header_only = generator_config.header_only;

        if (monomorphic && stream_type.empty()) {
            throw std::runtime_error("Cannot create monomorphic lexer: stream_type is not set");
        }

        if (!monomorphic && !header_only) {
            throw std::runtime_error("Cannot create polymorphic lexer: requested generation type is non header-only");
        }

        auto& path_to_header = util::check<config::settings::string_value>(
                lexer_settings.config.at("generation.cpp.path_to_header"_u8).get()
        )->str;

        if (header_only && !path_to_header.empty()) {
            throw std::runtime_error("Path to header cannot be set in header-only mode");
        }
    }

    const std::unique_ptr<config::settings::value>& lexer_generator::get_param(const util::u8string& param) const {
        return lexer_settings.config.at(param);
    }

    bool lexer_generator::get_value(const std::unique_ptr<config::settings::value>& ptr) {
        return util::check<config::settings::bool_value>(ptr.get())->val;
    }
}