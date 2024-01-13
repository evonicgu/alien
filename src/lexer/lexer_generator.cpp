#include "lexer/lexer_generator.h"

#include "config/settings/settings.h"
#include "config/settings/lexer.h"
#include "lexer/config/settings/parser.h"
#include "lexer/config/rules/lexer.h"
#include "lexer/config/rules/parser.h"
#include "lexer/automata/dfa.h"
#include "util/to_json.h"
#include "lexer/automata/generator.h"
#include "util/typeutils.h"

namespace alien::lexer {

    settings::settings_t lexer_generator::parse_lexer_config() {
        using namespace util::literals;

        settings::settings_t lexer_settings;

        config::settings::lexer settings_lexer(input_stream, err);
        settings::settings_parser settings_parser(settings_lexer, err, alphabet);

        alphabet.terminals.push_back({
                                             config::settings::error_t,
                                             settings::default_token_typename
                                     });

        settings_parser.add_language_settings(language);

        settings_parser.parse();

        lexer_settings = settings_parser.get_settings();

        rules::lexer rules_lexer(input_stream, err);
        rules::parser rules_parser(rules_lexer, err, alphabet);

        rules_parser.parse();

        lexer_rules = rules_parser.get_rules();

        no_utf8 = util::check<config::settings::bool_value>(
                lexer_settings.config.at("generation.noutf8"_u8).get()
        )->val;

        return std::move(lexer_settings);
    }

    std::optional<nlohmann::json> lexer_generator::generate_lexer() {
        using namespace util::literals;

        std::vector<std::size_t> ctx_start_states;
        std::vector<rules::action> actions;
        std::vector<automata::dfa::dfa> automations;

        bool has_any_start_transitions = false;
        std::size_t current_states = 0;

        if (alphabet.terminals.size() == 0) {
            err.push_back("Cannot create lexer: no terminals are defined"_u8);
        }

        for (auto& rules : lexer_rules.ruleset) {
            if (rules.empty()) {
                err.push_back("Cannot define empty contexts"_u8);

                continue;
            }

            automata::generator gen(err, no_utf8);

            if (rules[0].rule_number >= actions.size()) {
                actions.resize(rules.back().rule_number + 1);
            }

            for (auto& rule : rules) {
                actions[rule.rule_number] = std::move(rule.act);
            }

            automations.push_back(gen.generate_automata(rules));
            ctx_start_states.push_back(automations.back().start_state + current_states);
            current_states += automations.back().states.size();

            has_any_start_transitions = has_any_start_transitions || automations.back().transitions_to_start;
        }

        if (!err.empty()) {
            return {};
        }


//        nlohmann::json data{
////                {"no_utf8",                   no_utf8},
////                {"code_headers",              std::move(lexer_settings.code_declarations[config::settings::code_token::location::HEADERS])},
////                {"code_decl",                 std::move(lexer_settings.code_declarations[config::settings::code_token::location::DECL])},
////                {"code_impl",                 std::move(lexer_settings.code_declarations[config::settings::code_token::location::IMPL])},
////                {"code_content_decl_private", std::move(lexer_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PRIVATE])},
////                {"code_content_decl_public",  std::move(lexer_settings.code_declarations[config::settings::code_token::location::CONTENT_DECL_PUBLIC])},
////                {"code_content_impl",         std::move(lexer_settings.code_declarations[config::settings::code_token::location::CONTENT_IMPL])},
//////                {"contexts",                  util::to_json(lexer_rules.ctx)},
//                {"macros",                    get_value(lexer_settings.config["generation.cpp.macros"_u8])},
////                {"emit_stream",               !no_utf8 &&
//                                              get_value(lexer_settings.config["generation.emit_stream"_u8])},
////                {"token_default",             token_type_default},
////                {"position_default",          position_type_default},
//                {"token_type",                std::move(token_type)},
//                {"position_type",             std::move(position_type)},
//                {"use_enum_class",            get_value(lexer_settings.config["generation.cpp.enum_class"_u8])},
//////                {"ctx_start_states",          std::move(ctx_start_states)},
//////                {"has_any_start_transitions", has_any_start_transitions},
//////                {"symbols",                   util::to_json(alphabet.terminals,
//                                                            [](const lexer::settings::lexer_symbol& symbol) {
//                                                                return util::u8string_to_bytes(symbol.name);
//                                                            })},
//                {"guard_prefix",              guard_prefix},
////                {"track_lines",               get_value(lexer_settings.config["generation.track_lines"_u8])},
////                {"buffer_size",               buffer_size},
////                {"lexeme_size",               lexeme_size},
//////                {"actions",                   std::move(actions)},
//////                {"on_eof",                    std::move(lexer_rules.on_eof)},
//////                {"automations",               automations},
////                {"custom_error",              get_value(lexer_settings.config["generation.custom_error"_u8])},
//                {"lexer_namespace",           std::move(lexer_namespace)},
////                {"monomorphic",               get_value(lexer_settings.config["generation.monomorphic"_u8])},
////                {"stream_type",               std::move(stream_type)},
//                {"is_header_only",            generator_config.header_only},
//                {"path_to_header",            path_to_header},
//                {"no_default_constructor",    get_value(
//                        lexer_settings.config["generation.cpp.no_default_constructor"_u8])}
//        };

        return {{
//                        {"no_utf8", no_utf8},
//                        {"code_headers", std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::HEADERS))},
//                        {"code_decl", std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::DECL))},
//                        {"code_impl", std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::IMPL))},
//                        {"code_content_decl_private", std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::CONTENT_DECL_PRIVATE))},
//                        {"code_content_decl_public", std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::CONTENT_DECL_PUBLIC))},
//                        {"code_content_impl", std::move(lexer_settings.code_declarations.at(config::settings::code_token::location::CONTENT_IMPL))},
                        {"contexts", util::to_json(lexer_rules.ctx)},
//                        {"emit_stream", !no_utf8 && get_value(lexer_settings.config.at("generation.emit_stream"_u8))},
//                        {"token_default", token_type_default},
//                        {"position_default", position_type_default},
                        {"ctx_start_states", std::move(ctx_start_states)},
                        {"has_any_start_transitions", has_any_start_transitions},
                        {"symbols", util::to_json(alphabet.terminals,
                                                  [](const settings::lexer_symbol& symbol) {
                                                      return util::u8string_to_bytes(symbol.name);
                                                  })},
//                        {"track_lines", get_value(lexer_settings.config.at("generation.track_lines"_u8))},
//                        {"buffer_size", buffer_size},
//                        {"lexeme_size", lexeme_size},
                        {"actions", std::move(actions)},
                        {"on_eof", std::move(lexer_rules.on_eof)},
                        {"is_default_token_type", get_default_token_type_info()},
                        {"automations", automations},
//                        {"custom_error", get_value(lexer_settings.config.at("generation.custom_error"_u8))},
//                        {"monomorphic", get_value(lexer_settings.config.at("generation.monomorphic"_u8))},
//                        {"stream_type", std::move(stream_type)},
                }};
    }

    std::vector<bool> lexer_generator::get_default_token_type_info() const {
        std::vector<bool> default_type_info(alphabet.terminals.size());

        for (std::size_t i = 0; i < alphabet.terminals.size(); ++i) {
            default_type_info[i] = alphabet.terminals[i].default_type;
        }

        return default_type_info;
    }

}