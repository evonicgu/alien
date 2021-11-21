#ifndef ALIEN_LEXER_GENERATOR_H
#define ALIEN_LEXER_GENERATOR_H

#include <memory>
#include "automata/algorithm.h"
#include "config/rules/rules_parser.h"
#include "generalized/generalized_generator.h"
#include "regex/parser.h"
#include "lexer_stubs.h"

namespace alien::lexer {

    using base_generator = generalized::generalized_generator<config::rules::rules>;

    using namespace alien::config::settings;
    using namespace util::literals;

    class generator : public base_generator {
    public:
        generator(const std::string& input_file,
                  const std::string& output_file,
                  bool gen_header) : base_generator(input_file, output_file, gen_header) {}

        settings&& get_settings() {
            return std::move(configuration);
        }

    private:
        void init_settings() override {
            configuration = {
                {
                    {"generation.token_type"_u8, std::make_shared<string_value>("generalized_token"_u8)},
                    {"generation.use_macros"_u8, std::make_shared<bool_value>(false)},
                    {"generation.use_enum_class"_u8, std::make_shared<bool_value>(true)},
                    {"generation.error_function"_u8, std::make_shared<string_value>("error"_u8)},
                    {"general.enable_trailing_return"_u8, std::make_shared<bool_value>(true)},
                },
            };
        }

        void parse_rules(input::stream_input& input) override {
            auto* val = check<bool_value>("general.enable_trailing_return"_u8);

            config::rules::lexer::lexer l(val->val, input);
            config::rules::parser::parser p(ruleset, std::move(l));

            p.parse();

            if (ruleset.eof_action.code.empty()) {
                ruleset.eof_action.code = util::ascii_to_u8string(default_eof_action);
            }
        }

        void emit_start_code() override {
            for (auto& code : configuration.code_top) {
                output << "\n\n" << util::u8string_to_bytes(code) << "\n\n";
            }

            output << required_code;

            emit_macros();

            emit_token_class();

            emit_token_enum();

            emit_error_function();

            for (auto& code : configuration.code) {
                output << "\n\n" << util::u8string_to_bytes(code) << "\n\n";
            }
        }

        void emit_pre_end_code() override {
            output << lexer_start;

            emit_dfa();

            emit_lexer_mid();

            output << lexer_end;
        }

        void emit_token_class() {
            auto* val = check<string_value>("generation.token_type"_u8);

            if (val->str == "generalized_token"_u8) {
                output << default_token;
            }
        }

        void emit_macros() {
            auto* val = check<bool_value>("generation.use_macros"_u8);
            auto* token_type = check<string_value>("generation.token_type"_u8);

            if (val->val) {
                for (unsigned int i = 0; i < configuration.symbols.size(); ++i) {
                    const auto& token = configuration.symbols[i];

                    const util::u8string& name = token.name;

                    util::u8string macro = "#define _"_u8;
                    macro += name + " return new "_u8;
                    macro += token_type->str + "<token_type>("_u8;
                    macro += name + ")\n"_u8;

                    output << util::u8string_to_bytes(macro);
                }

                output << '\n';
            }
        }

        void emit_token_enum() {
            auto* val = check<bool_value>("generation.use_enum_class"_u8);

            output << "enum" << (val->val ? " class " : " ") << "token_type {\n";

            for (unsigned int i = 0; i < configuration.symbols.size(); ++i) {
                const auto& token = configuration.symbols[i];

                const util::u8string& name = token.name;

                output << "    " << util::u8string_to_bytes(name) << ",\n";
            }

            output << "};\n\n";
        }

        void emit_error_function() {
            auto* error_func = check<string_value>("generation.error_function"_u8);

            if (error_func->str == "error"_u8) {
                output << default_error_function;
            }
        }

        void emit_dfa() {
            output << "    std::map<std::string, unsigned int> context_mapping = {";

            for (const auto& context : ruleset.context_mapping) {
                output << "{\"" << util::u8string_to_bytes(context.first) << "\", " << context.second << "}, ";
            }

            output << "};\n\n    std::vector<state> dfa[" << ruleset.context_mapping.size() << "] = {\n";
            std::vector<unsigned int> start_states;

            for (auto& context : ruleset.context_mapping) {
                automata::dfa::dfa automata = build_dfa(context.second);
                start_states.push_back(automata.start_state);

                output << "        {\n";

                for (const auto& state : automata.states) {
                    output << "            {\n";
                    output << "                {";

                    for (unsigned int transition : state.out_transitions) {
                        output << '{' << (int) automata.transitions[transition].label << ", ";
                        output << automata.transitions[transition].head << "}, ";
                    }

                    output << "},\n                " << std::boolalpha << state.accepting << ",\n";
                    output << "                " << state.rule_number << "\n";

                    output << "            },\n";
                }

                output << "        },\n";
            }

            output << "    };\n\n";
            output << "    unsigned int start_states[" << ruleset.context_mapping.size() << "] = {";

            for (unsigned int start_state : start_states) {
                output << start_state << ", ";
            }

            output << "};\n\n";

            for (auto& code : configuration.code_content) {
                output << "\n\n" << util::u8string_to_bytes(code) << "\n\n";
            }

            output << lexer_constructor_start << start_states[0] << lexer_constructor_end;
        }

        automata::dfa::dfa build_dfa(unsigned int context) {
            auto* start_nfa_state = new automata::nfa::state{{}, false, -1};
            std::vector<automata::nfa::state*> nfa_states{start_nfa_state};

            start_nfa_state->accepting = false;

            std::set<util::u8char> A;
            unsigned int rules = ruleset.ruleset[context].size();

            for (auto& rule : ruleset.ruleset[context]) {
                input::string_input i(std::move(rule.regex));

                regex::parser::parser p{regex::lexer::lexer(i)};

                p.parse();

                alien::lexer::regex::parser::ast::node_ptr ast = p.get_ast();

                auto result = automata::algorithm::nfa_from_tree(ast, rule.rule_number, nfa_states);

                A.merge(result.second);
                start_nfa_state->transitions[-1].insert(result.first);
            }

            auto dfa = automata::algorithm::minimize(
                    automata::algorithm::convert_nfa2dfa(start_nfa_state, A, rules),
                    A);

            for (auto* nfa_state : nfa_states) {
                delete nfa_state;
            }

            return dfa;
        }

        void emit_lexer_mid() {
            auto* token_type = check<string_value>("generation.token_type"_u8);

            output << "    " << util::u8string_to_bytes(token_type->str) << "<token_type>* lex() {\n";
            output << lex_method_start << util::u8string_to_bytes(ruleset.eof_action.code) << lex_method_after_eof;
            output << "                ";
            output << util::u8string_to_bytes(check<string_value>("generation.error_function"_u8)->str);
            output << "(line, column);\n" << lex_method_mid;

            for (unsigned int context = 0; context < ruleset.context_mapping.size(); ++context) {
                for (auto &rule : ruleset.ruleset[context]) {
                    output << "                case " << rule.rule_number << ": {\n";
                    auto &action = rule.act;

                    if (!action.trailing_return.empty()) {
                        action.code += "\nreturn new "_u8 + token_type->str + "<token_type>("_u8;
                        action.code += "token_type::"_u8 + action.trailing_return;
                        action.code += ", {sline, scolumn}, {line, column});"_u8;
                    }

                    output << util::u8string_to_bytes(action.code) << "\nbreak;\n                }\n";
                }
            }

            output << lex_method_end;
        }
    };

}

#endif //ALIEN_LEXER_GENERATOR_H