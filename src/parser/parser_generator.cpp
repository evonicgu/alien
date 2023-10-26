#include "parser/parser_generator.h"

namespace alien::parser {

    settings::settings_t parser_generator::parse_parser_config() {
        using namespace util::literals;

        settings::settings_t parser_settings;

        config::settings::lexer settings_lexer(input_stream, err);
        parser::settings::settings_parser settings_parser(settings_lexer, err, alphabet);

        alphabet.non_terminals.push_back({
                                                 "@$S"_u8,
                                                 settings::void_type
                                         });

        settings_parser.add_language_settings(language);

        settings_parser.parse();

        parser_settings = settings_parser.get_settings();

        const auto& parser_type = util::check<config::settings::string_value>(
                parser_settings.config["generation.type"_u8].get()
        )->str;

        const auto& first_symbol = util::check<config::settings::string_value>(
                parser_settings.config["general.start"_u8].get()
        )->str;

        parser::rules::lexer rules_lexer(input_stream, err);
        parser::rules::parser rules_parser(rules_lexer, err, alphabet, first_symbol);

        rules_parser.parse();

        parser_rules = rules_parser.get_rules();

        if (parser_type == "lalr"_u8) {
            table_generator = make_gen<parser::generator::lalr_generator>();
        } else if (parser_type == "slr"_u8) {
            table_generator = make_gen<parser::generator::slr_generator>();
        } else if (parser_type == "clr"_u8) {
            table_generator = make_gen<parser::generator::clr_generator>();
        } else {
            err.push_back("Invalid parser type: "_u8 + parser_type);
        }

        return std::move(parser_settings);
    }

    std::optional<inja::json> parser_generator::generate_parser() {
        using namespace util::literals;

        if (alphabet.non_terminals.size() == 1) {
            err.push_back("Cannot create parser: no symbols are defined"_u8);
            return {};
        }

        for (std::size_t i = 0; i < alphabet.non_terminals.size(); ++i) {
            if (parser_rules.ruleset[i].empty()) {
                err.push_back("No productions are defined for symbol "_u8 + alphabet.non_terminals[i].name);
                return {};
            }
        }

        parser::generator::parsing_table table = table_generator->generate_table();

        std::vector<bool> shift_err_states(table.size(), false),
                reduce_err_states(table.size(), false);

        std::vector<parser::generator::parsing_action> reduce_err_actions(table.size());

        bool has_error_productions = false;

        for (std::size_t i = 0; i < table.size(); ++i) {
            auto it = table[i].find({parser::rules::symbol_type::TERMINAL, 0});

            if (it != table[i].end()) {
                if (it->second.type == parser::generator::action_type::SHIFT) {
                    shift_err_states[i] = true;
                } else {
                    reduce_err_states[i] = true;
                    reduce_err_actions[i] = it->second;
                }
                has_error_productions = true;
            }
        }

        if (!err.empty()) {
            return {};
        }

//        util::u8string symbol_type = std::move(util::check<config::settings::string_value>(
//                parser_settings.config["generation.symbol_type"_u8].get()
//        )->str);
//
//        util::u8string parser_namespace = std::move(util::check<config::settings::string_value>(
//                parser_settings.config["generation.namespace"_u8].get()
//        )->str);

        return {{
////                {"code_headers",              std::move(parser_settings.code_declarations[config::settings::code_token::location::HEADERS])},
////                {"code_decl",                 std::move(parser_settings.code_declarations[config::settings::code_token::location::DECL])},
////                {"code_impl",                 std::move(parser_settings.code_declarations[config::settings::code_token::location::IMPL])},
////                {"code_content_decl_private", std::move(parser_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PRIVATE])},
////                {"code_content_decl_public",  std::move(parser_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PUBLIC])},
////                {"code_content_impl",         std::move(parser_settings.code_declarations[config::settings::code_token::location::CONTENT_IMPL])},
                {"table",                     parser::generator::transform_table(table, (std::ptrdiff_t) alphabet.terminals.size())},
                {"terminals",                 alphabet.terminals.size()},
//                {"symbol_type",               std::move(symbol_type)},
                {"lengths",                   get_parser_lengths()},
//                {"types",                     get_parser_types()},
                {"rules",                     get_parser_rules()},
//                {"guard_prefix",              guard_prefix},
                {"start_rule",                parser_rules.start + alphabet.terminals.size()},
                {"actions",                   get_parser_actions()},
////                {"custom_error",              get_value(parser_settings.config["generation.custom_error"_u8])},
////                {"use_token_to_str",          get_value(parser_settings.config["generation.use_token_to_str"_u8])},
////                {"default_token_to_str",      get_value(parser_settings.config["generation.default_token_to_str"_u8])},
                {"shift_err_states",          std::move(shift_err_states)},
                {"reduce_err_states",         std::move(reduce_err_states)},
                {"reduce_err_actions",        std::move(reduce_err_actions)},
                {"has_error_productions",     has_error_productions},
//                {"is_header_only",            generator_config.header_only},
////                {"monomorphic",               monomorphic},
                {"tokens",                    util::to_json(alphabet.terminals,
                                                            [](const lexer::settings::lexer_symbol& symbol) {
                                                                return util::u8string_to_bytes(symbol.name);
                                                            })},
////                {"track_lines",               track_lines},
//                {"parser_namespace",          std::move(parser_namespace)},
//                {"no_default_constructor",    get_value(parser_settings.config["generation.cpp.no_default_constructor"_u8])},
//                {"lexer_relative_namespace",  lexer_relative_namespace}
        }};

//        inja::Template header_tmpl = env.parse_file(generator_config.parser_header_template);
//
//        env.render_to(generator_streams.parser_header_out, header_tmpl, data);
//
//        inja::Template source_tmpl = env.parse_file(generator_config.parser_template);
//
//        auto& out_stream = generator_config.header_only ?
//                           generator_streams.parser_header_out :
//                           generator_streams.parser_source_out.value();
//
//        env.render_to(out_stream, source_tmpl, data);
    }

    std::vector<std::vector<std::vector<std::ptrdiff_t>>> parser_generator::get_parser_rules() {
        std::vector<std::vector<std::vector<std::ptrdiff_t>>> json_rules{parser_rules.ruleset.size()};

        for (std::size_t i = 0; i < parser_rules.ruleset.size(); ++i) {
            json_rules[i].reserve(parser_rules.ruleset[i].size());

            for (const auto& production : parser_rules.ruleset[i]) {
                std::vector<std::ptrdiff_t> prod;
                prod.reserve(production.symbols.size());

                const auto& symbols = production.symbols;

                for (const auto& symbol : symbols) {
                    using t = parser::rules::symbol_type;

                    if (symbol.type == t::NON_TERMINAL) {
                        bool is_midrule = alphabet.non_terminals[symbol.index].is_midrule;
                        auto& midrule_symbols = parser_rules.ruleset[symbol.index][0].symbols;

                        if (is_midrule && midrule_symbols.empty()) {
                            midrule_symbols = {symbols.begin(), symbols.begin() + prod.size()};
                        }
                    }

                    prod.push_back(symbol.index + (symbol.type == t::NON_TERMINAL) * alphabet.terminals.size());
                }

                json_rules[i].push_back(std::move(prod));
            }
        }

        return json_rules;
    }

    std::vector<std::vector<std::size_t>> parser_generator::get_parser_lengths() {
        std::vector<std::vector<std::size_t>> lengths{parser_rules.ruleset.size()};

        for (std::size_t i = 0; i < parser_rules.ruleset.size(); ++i) {
            for (const auto& prod : parser_rules.ruleset[i]) {
                lengths[i].push_back(prod.symbols.size());
            }
        }

        return lengths;
    }

    std::vector<std::vector<std::string>> parser_generator::get_parser_actions() {
        std::vector<std::vector<std::string>> actions{parser_rules.ruleset.size()};

        for (std::size_t i = 0; i < parser_rules.ruleset.size(); ++i) {
            for (auto& prod : parser_rules.ruleset[i]) {
                actions[i].push_back(util::u8string_to_bytes(prod.action));
            }
        }

        return actions;
    }
}