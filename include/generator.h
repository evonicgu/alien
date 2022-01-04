#ifndef ALIEN_GENERATOR_H
#define ALIEN_GENERATOR_H

#include <fstream>
#include <list>
#include <memory>
#include <vector>

#include "inja/inja.hpp"

#include "alphabet.h"
#include "fwd/lexer_fwd.h"
#include "fwd/parser_fwd.h"
#include "input/input.h"
#include "util/to_json.h"
#include "util/typeutils.h"
#include "util/u8string.h"

namespace alien {

    class generator {
        input::stream_input input;
        std::ofstream& output;
        std::list<util::u8string>& err;

        config::settings::settings<lexer::settings::lexer_symbol> lconfig;
        config::settings::settings<parser::settings::parser_symbol> pconfig;

        lexer::rules::rules lrules;
        parser::rules::rules prules;

        bool token_type_default = false, position_type_default = false;

        util::u8string lexer_relative_namespace;

        alphabet::alphabet alphabet;
        inja::Environment env;

        std::shared_ptr<parser::generator::generator> parser_generator;

    public:
        generator(std::ifstream& in, std::ofstream& out, std::list<util::u8string>& err)
            : input(in),
              output(out),
              err(err) {
            env.add_callback("bytes", 1, [](inja::Arguments& args) {
                return util::u8string_to_bytes(args.at(0)->get<util::u8string>());
            });

            env.add_callback("lexer_collect", 2, [](inja::Arguments& args) {
                auto& state = *args.at(0);
                auto& automata = *args.at(1);

                inja::json json;

                std::vector<std::vector<util::u8char>> symbols(automata["states"].size());
                for (const auto& transition_index : state["transitions"]) {
                    auto& transition = automata["transitions"][transition_index.get<std::size_t>()];

                    symbols[transition["head"].get<std::size_t>()].push_back(transition["label"].get<util::u8char>());
                }

                return symbols;
            });

            env.add_callback("create_lexer_range", 2, [](inja::Arguments& args) {
                util::u8char start = args.at(0)->get<util::u8char>(), end = args.at(1)->get<util::u8char>();
                std::string c_varname = "c";

                if (start < 0) {
                    c_varname = "c_class";
                }

                return create_range(c_varname, start, end);
            });

            env.add_callback("create_parser_range", 3, [](inja::Arguments& args) {
                std::ptrdiff_t start = args.at(0)->get<std::ptrdiff_t>(), end = args.at(1)->get<std::ptrdiff_t>();
                bool is_rule = args.at(2)->get<bool>();

                std::string l_varname = "lookahead.index";

                if (is_rule) {
                    l_varname = "rule_lookahead.index";
                }

                return create_range(l_varname, start, end);
            });

            env.set_comment("#{", "}#");
        }

        void generate(const std::string& ltemplate, const std::string& ptemplate) {
            using namespace util::literals;

            parse_lexer_config();

            parse_parser_config();

            util::u8string& lexer_namespace = util::check<config::settings::string_value>(
                    lconfig.config["generation.namespace"_u8].get()
            )->str;

            util::u8string& parser_namespace = util::check<config::settings::string_value>(
                    pconfig.config["generation.namespace"_u8].get()
            )->str;

            std::size_t common_prefix_length = 0;

            for (std::size_t i = 0; i < std::min(lexer_namespace.size(), parser_namespace.size()); ++i) {
                if (lexer_namespace[i] != parser_namespace[i]) {
                    break;
                }

                ++common_prefix_length;
            }

            lexer_relative_namespace = lexer_namespace.substr(common_prefix_length);

            if (lexer_relative_namespace.find("::"_u8) == 0) {
                lexer_relative_namespace = lexer_relative_namespace.substr(2);
            }

            if (!lexer_relative_namespace.empty()) {
                lexer_relative_namespace += "::"_u8;
            }

            for (std::size_t i = 0; i < alphabet.terminals.size(); ++i) {
                if (alphabet.terminals[i].type == "__::token_t"_u8) {
                    alphabet.terminals[i].type = lexer_relative_namespace + "token_t"_u8;
                }
            }

            if (!err.empty()) {
                // there are errors
                return;
            }

            generate_lexer(ltemplate);

            generate_parser(ptemplate);
        }

    private:
        void parse_lexer_config() {
            using namespace util::literals;

            config::settings::lexer settings_lexer(input, err);
            lexer::settings::settings_parser settings_parser(settings_lexer, err);

            settings_parser.parse();

            lconfig = settings_parser.get_settings();
            token_type_default = settings_parser.token_default;
            position_type_default = settings_parser.position_default;

            alphabet.terminals.push_back({
                "error"_u8,
                "lexer::token_t"_u8
            });

            for (std::size_t i = 0; i < lconfig.symbols.size(); ++i) {
                lexer::settings::lexer_symbol symbol{
                        std::move(lconfig.symbols[i].name),
                        std::move(lconfig.symbols[i].type),
                        lconfig.symbols[i].prec,
                        lconfig.symbols[i].assoc
                };

                alphabet.terminals.push_back(std::move(symbol));
            }

            lexer::rules::lexer rules_lexer(input, err);
            lexer::rules::parser rules_parser(rules_lexer, err, alphabet);

            rules_parser.parse();

            lrules = rules_parser.get_rules();
        }

        void parse_parser_config() {
            using namespace util::literals;

            config::settings::lexer settings_lexer(input, err);
            parser::settings::settings_parser settings_parser(settings_lexer, err);

            settings_parser.parse();

            pconfig = settings_parser.get_settings();

            alphabet.non_terminals.push_back({
                "@$S"_u8,
                "void"_u8
            });

            for (std::size_t i = 0; i < pconfig.symbols.size(); ++i) {
                alphabet.non_terminals.push_back(std::move(pconfig.symbols[i]));
            }

            const auto& parser_type = util::check<config::settings::string_value>(
                    pconfig.config["generation.type"_u8].get()
            )->str;

            const auto& first_symbol = util::check<config::settings::string_value>(
                    pconfig.config["general.start"_u8].get()
            )->str;

            parser::rules::lexer rules_lexer(input, err);
            parser::rules::parser rules_parser(rules_lexer, err, alphabet, first_symbol);

            rules_parser.parse();

            prules = rules_parser.get_rules();

            if (parser_type == "lalr"_u8) {
                parser_generator = std::make_shared<parser::generator::lalr_generator>(alphabet, prules);
            } else if (parser_type == "slr"_u8) {
                parser_generator = std::make_shared<parser::generator::slr_generator>(alphabet, prules);
            } else if (parser_type == "clr"_u8) {
                parser_generator = std::make_shared<parser::generator::clr_generator>(alphabet, prules);
            } else {
                err.push_back("Invalid parser type: "_u8 + parser_type);
            }
        }

        void generate_lexer(const std::string& ltemplate) {
            using namespace util::literals;

            std::vector<std::size_t> ctx_start_states;
            std::vector<lexer::rules::action> actions;
            std::vector<lexer::automata::dfa::dfa> automations;

            bool no_utf8 = get_value(lconfig.config["generation.noutf8"_u8]);
            bool has_any_start_transitions = false;
            std::size_t current_states = 0;

            if (alphabet.terminals.size() == 0) {
                err.push_back("Cannot create lexer: no terminals are defined"_u8);
            }

            for (auto& rules : lrules.ruleset) {
                if (rules.empty()) {
                    err.push_back("Cannot define empty contexts"_u8);

                    continue;
                }

                lexer::automata::generator gen(err, no_utf8);

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
                return;
            }

            long long buffer_size = util::check<config::settings::number_value>(
                    lconfig.config["generation.buffer_size"_u8].get()
                    )->number;
            long long lexeme_size = util::check<config::settings::number_value>(
                    lconfig.config["generation.lexeme_size"_u8].get()
                    )->number;

            if (buffer_size <= 0 || lexeme_size <= 0) {
                throw std::runtime_error("Buffer or lexeme size must be greater than zero");
            }

            util::u8string token_type = util::check<config::settings::string_value>(
                    lconfig.config["generation.token_type"_u8].get()
            )->str + "<token_type>"_u8;

            util::u8string position_type = std::move(util::check<config::settings::string_value>(
                    lconfig.config["generation.position_type"_u8].get()
            )->str);

            util::u8string lexer_namespace = std::move(util::check<config::settings::string_value>(
                    lconfig.config["generation.namespace"_u8].get()
            )->str);


            inja::json data{
                    {"no_utf8", no_utf8},
                    {"code_top", std::move(lconfig.code_top)},
                    {"code_default", std::move(lconfig.code)},
                    {"code_content", std::move(lconfig.code_content)},
                    {"contexts", util::to_json(lrules.ctx)},
                    {"macros", get_value(lconfig.config["generation.macros"_u8])},
                    {"emit_stream", !no_utf8 && get_value(lconfig.config["generation.emit_stream"_u8])},
                    {"token_default", token_type_default},
                    {"position_default", position_type_default},
                    {"token_type", std::move(token_type)},
                    {"position_type", std::move(position_type)},
                    {"use_enum_class", get_value(lconfig.config["generation.enum_class"_u8])},
                    {"ctx_start_states", std::move(ctx_start_states)},
                    {"has_any_start_transitions", has_any_start_transitions},
                    {"symbols", util::to_json(alphabet.terminals, [](const lexer::settings::lexer_symbol& symbol) {
                        return util::u8string_to_bytes(symbol.name);
                    })},
                    {"track_lines", get_value(lconfig.config["generation.track_lines"_u8])},
                    {"buffer_size", buffer_size},
                    {"lexeme_size", lexeme_size},
                    {"actions", std::move(actions)},
                    {"no_return", get_value(lconfig.config["generation.noreturn"_u8])},
                    {"on_eof", std::move(lrules.on_eof)},
                    {"automations", automations},
                    {"custom_error", get_value(lconfig.config["generation.custom_error"_u8])},
                    {"lexer_namespace", std::move(lexer_namespace)},
                    {"no_default_constructor", get_value(lconfig.config["generation.no_default_constructor"_u8])}
            };

            inja::Template tmpl = env.parse_file(ltemplate);
            env.render_to(output, tmpl, data);
        }

        void generate_parser(const std::string& ptemplate) {
            using namespace util::literals;

            if (alphabet.non_terminals.size() == 1) {
                err.push_back("Cannot create parser: no symbols are defined"_u8);
                return;
            }

            for (std::size_t i = 0; i < alphabet.non_terminals.size(); ++i) {
                if (prules.ruleset[i].empty()) {
                    err.push_back("No productions are defined for symbol "_u8 + alphabet.non_terminals[i].name);
                    return;
                }
            }

            parser::generator::parsing_table table = parser_generator->generate_table();

            std::vector<bool> err_states(table.size()), recover_states(table.size());

            bool has_error_productions = false;

            for (std::size_t i = 0; i < table.size(); ++i) {
                auto it = table[i].find({parser::rules::symbol_type::TERMINAL, 0});

                if (it != table[i].end()) {
                    err_states[i] = true;
                    has_error_productions = true;
                    recover_states[it->second.arg1] = true;
                } else {
                    err_states[i] = false;
                }
            }

            if (!err.empty()) {
                return;
            }

            util::u8string symbol_type = std::move(util::check<config::settings::string_value>(
                    pconfig.config["generation.symbol_type"_u8].get()
            )->str);

            util::u8string parser_namespace = std::move(util::check<config::settings::string_value>(
                    pconfig.config["generation.namespace"_u8].get()
            )->str);

            inja::json data{
                    {"code_top", std::move(pconfig.code_top)},
                    {"code_default", std::move(pconfig.code)},
                    {"code_content", std::move(pconfig.code_content)},
                    {"table", parser::generator::transform_table(table, (std::ptrdiff_t) alphabet.terminals.size())},
                    {"terminals", alphabet.terminals.size()},
                    {"symbol_type", std::move(symbol_type)},
                    {"lengths", get_parser_lengths()},
                    {"types", get_parser_types()},
                    {"rules", get_parser_rules()},
                    {"start_rule", prules.start + alphabet.terminals.size()},
                    {"actions", get_parser_actions()},
                    {"custom_error", get_value(pconfig.config["generation.custom_error"_u8])},
                    {"use_token_to_str", get_value(pconfig.config["generation.use_token_to_str"_u8])},
                    {"default_token_to_str", get_value(pconfig.config["generation.default_token_to_str"_u8])},
                    {"err_states", std::move(err_states)},
                    {"has_error_productions", has_error_productions},
                    {"tokens", util::to_json(alphabet.terminals, [](const lexer::settings::lexer_symbol& symbol) {
                        return util::u8string_to_bytes(symbol.name);
                    })},
                    {"track_lines", get_value(lconfig.config["generation.track_lines"_u8])},
                    {"recover_states", std::move(recover_states)},
                    {"parser_namespace", std::move(parser_namespace)},
                    {"no_default_constructor", get_value(pconfig.config["generation.no_default_constructor"_u8])},
                    {"lexer_relative_namespace", std::move(lexer_relative_namespace)}
            };

            inja::Template tmpl = env.parse_file(ptemplate);
            env.render_to(output, tmpl, data);
        }

    private:
        static bool get_value(const std::shared_ptr<config::settings::value>& ptr) {
            return util::check<config::settings::bool_value>(ptr.get())->val;
        }

        static std::string create_range(const std::string& var, std::ptrdiff_t start, std::ptrdiff_t end) {
            if (start == end) {
                return var + " == " + std::to_string(start);
            }

            return var + " >= " + std::to_string(start) + " && " + var + " <= " + std::to_string(end);
        }

        std::vector<std::vector<std::vector<std::ptrdiff_t>>> get_parser_rules() {
            std::vector<std::vector<std::vector<std::ptrdiff_t>>> json_rules{prules.ruleset.size()};

            for (std::size_t i = 0; i < prules.ruleset.size(); ++i) {
                for (const auto& production : prules.ruleset[i]) {
                    std::vector<std::ptrdiff_t> prod;
                    const auto& symbols = production.symbols;

                    for (const auto& symbol : symbols) {
                        using t = parser::rules::symbol_type;

                        if (symbol.type == t::NON_TERMINAL) {
                            bool is_midrule = alphabet.non_terminals[symbol.index].is_midrule;
                            auto& midrule_symbols = prules.ruleset[symbol.index][0].symbols;

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

        std::vector<util::u8string> get_parser_types() {
            std::vector<util::u8string> types;

            for (std::size_t i = 0; i < alphabet.terminals.size(); ++i) {
                types.push_back(std::move(alphabet.terminals[i].type));
            }

            for (std::size_t i = 0; i < alphabet.non_terminals.size(); ++i) {
                types.push_back(std::move(alphabet.non_terminals[i].type));
            }

            return types;
        }

        std::vector<std::vector<std::size_t>> get_parser_lengths() {
            std::vector<std::vector<std::size_t>> lengths{prules.ruleset.size()};

            for (std::size_t i = 0; i < prules.ruleset.size(); ++i) {
                for (const auto& prod : prules.ruleset[i]) {
                    lengths[i].push_back(prod.symbols.size());
                }
            }

            return lengths;
        }

        std::vector<std::vector<util::u8string>> get_parser_actions() {
            std::vector<std::vector<util::u8string>> actions{prules.ruleset.size()};

            for (std::size_t i = 0; i < prules.ruleset.size(); ++i) {
                for (auto& prod : prules.ruleset[i]) {
                    actions[i].push_back(std::move(prod.action));
                }
            }

            return actions;
        }
    };

}

#endif //ALIEN_GENERATOR_H