#ifndef ALIEN_LEXER_GENERATOR_H
#define ALIEN_LEXER_GENERATOR_H

#include <fstream>
#include <memory>
#include <string>
#include "automata/algorithm.h"
#include "config/rules/rules_parser.h"
#include "config/settings/settings_parser.h"
#include "input/input.h"
#include "regex/parser.h"
#include "lexer_stubs.h"

namespace alien::lexer {

    using namespace alien::config::settings;
    using namespace config::settings;
    using namespace config::rules;

    class generator {
        std::ifstream stream;
        std::ofstream output;

        settings configuration = {
                {
                        {"generation.token_type", std::make_shared<string_value>("generalized_token")},
                        {"generation.use_macros", std::make_shared<bool_value>(false)},
                        {"generation.use_enum_class", std::make_shared<bool_value>(true)},
                        {"generation.error_function", std::make_shared<string_value>("error")},
                        {"general.enable_trailing_return", std::make_shared<bool_value>(true)},
                },
        };

        rules ruleset;

        std::string start_code, end_code;

    public:
        generator(const std::string& input_file, const std::string& output_file, bool gen_header) {
            stream = std::ifstream(input_file);

            output = std::ofstream(output_file + (gen_header ? ".h" : ".cpp"));
        }

        settings generate() {
            input::stream_input input(stream);

            start_code = scan_start_code(input);

            parse_settings(input);

            parse_rules(input);

            end_code = scan_end_code(input);

            emit();

            return configuration;
        }

    private:
        static std::string scan_start_code(input::stream_input& input) {
            char c = input.get();

            std::string code;

            while (c != -2) {
                if (c == '%' && input.peek() == '%') {
                    input.get();
                    break;
                }

                code += c;

                c = input.get();
            }

            return code;
        }

        static std::string scan_end_code(input::stream_input& input) {
            char c = input.get();

            std::string code;

            while (c != -2) {
                code += c;

                c = input.get();
            }

            return code;
        }

        void parse_settings(input::stream_input& input) {
            config::settings::lexer::lexer l(input);
            config::settings::parser::parser p(configuration, l);

            p.parse();
        }

        void parse_rules(input::stream_input& input) {
            auto* val = check<bool_value>("general.enable_trailing_return");

            config::rules::lexer::lexer l(val->val, input);
            config::rules::parser::parser p(ruleset, l);

            p.parse();

            if (ruleset.eof_action.code.empty()) {
                ruleset.eof_action.code = default_eof_action;
            }
        }

        void emit() {
            emit_required_code();

            emit_possible_macros();

            emit_possible_token_class();

            emit_token_enum();

            emit_possible_error_function();

            output << start_code;

            emit_start_lexer();

            emit_dfa();

            emit_lexer_mid();

            emit_lexer_end();

            output << end_code;

            std::cout << "done";
        }

        void emit_required_code() {
            output << required_code;
        };

        void emit_possible_token_class() {
            auto* val = check<string_value>("generation.token_type");

            if (val->str == "generalized_token") {
                output << default_token;
            }
        }

        void emit_possible_macros() {
            auto* val = check<bool_value>("generation.use_macros");
            auto* token_type = check<string_value>("generation.token_type");

            if (val->val) {
                for (const auto& token : configuration.tokens) {
                    const std::string& name = token.first;

                    output << "#define _" + name << " return new " + token_type->str + "<token_type>(" + name + ")\n";
                }

                output << '\n';
            }
        }

        void emit_start_lexer() {
            output << lexer_start;
        }

        void emit_token_enum() {
            auto* val = check<bool_value>("generation.use_enum_class");

            output << "enum" << (val->val ? " class " : " ") << "token_type {\n";

            for (const auto& token : configuration.tokens) {
                const std::string& name = token.first;

                output << "    " << name << ",\n";
            }

            output << "};\n\n";
        }

        void emit_possible_error_function() {
            auto* error_func = check<string_value>("generation.error_function");

            if (error_func->str == "error") {
                output << default_error_function;
            }
        }

        void emit_dfa() {
            output << "    std::map<std::string, unsigned int> context_mapping = {";

            for (const auto& context : ruleset.context_mapping) {
                output << "{\"" << context.first << "\", " << context.second << "}, ";
            }

            output << "};\n\n    std::array<std::vector<state>, " << ruleset.context_mapping.size() << "> dfa = {{\n";
            std::vector<unsigned int> start_states;

            for (auto& context : ruleset.context_mapping) {
                automata::dfa::dfa automata = build_dfa(context.second);
                start_states.push_back(automata.start_state);

                output << "        {{\n";

                for (const auto& state : automata.states) {
                    output << "            {\n";
                    output << "                    {";

                    for (unsigned int transition : state.out_transitions) {
                        output << '{' << (int) automata.labels[transition] << ", " << automata.heads[transition] << "}, ";
                    }

                    output << "},\n                    " << std::boolalpha << state.accepting << ",\n";
                    output << "                    " << state.rule_number << "\n";

                    output << "            },\n";
                }

                output << "        }},\n";
            }

            output << "    }};\n\n";
            output << "    std::array<unsigned int, " << ruleset.context_mapping.size() << "> start_states = {";

            for (unsigned int start_state : start_states) {
                output << start_state << ", ";
            }

            output << "};\n\n" << lexer_constructor_start << start_states[0] << lexer_constructor_end <<"\n\n";
        }

        automata::dfa::dfa build_dfa(unsigned int context) {
            alien::automata::nfa::state_ptr start_nfa_state = std::make_shared<automata::nfa::state>();

            start_nfa_state->accepting = false;

            std::set<char> A;

            for (const auto& rule : ruleset.ruleset[context]) {
                input::string_input i(rule.regex);

                regex::parser::parser p{regex::lexer::lexer(i)};

                p.parse();

                alien::lexer::regex::parser::ast::node_ptr ast = p.get_ast();
                auto result = automata::algorithm::nfa_from_tree(ast, rule.rule_number);
                std::set<char> m_alphabet = std::move(A);
                A = {};

                std::merge(m_alphabet.begin(), m_alphabet.end(),
                           result.second.begin(), result.second.end(),
                           std::inserter(A, A.begin()));

                start_nfa_state->transitions[-1].insert(result.first);
            }

            return automata::algorithm::minimize(automata::algorithm::convert_nfa2dfa(start_nfa_state, A), A);
        }

        void emit_lexer_mid() {
            auto* token_type = check<string_value>("generation.token_type");

            output << "    " << token_type->str << "<token_type>* lex() {\n";

            output << lex_method_start << ruleset.eof_action.code << lex_method_after_eof << "                ";
            output << check<string_value>("generation.error_function")->str << "(line, column);\n";
            output << lex_method_mid;

            for (unsigned int context = 0; context < ruleset.context_mapping.size(); ++context) {
                for (auto &rule: ruleset.ruleset[context]) {
                    output << "                case " << rule.rule_number << ": {\n";
                    auto &action = rule.act;

                    if (!action.trailing_return.empty()) {
                        action.code += "return new " + token_type->str + "<token_type>(";
                        action.code += "token_type::" + action.trailing_return + ");";
                    }

                    output << action.code << "\nbreak;\n                }\n";
                }
            }

            output << lex_method_end;
        }

        void emit_lexer_end() {
            output << lexer_end;
        }

        template<typename T>
        T* check(const std::string&& accessor) {
            auto* casted = dynamic_cast<T*>(configuration.config.at(accessor).get());

            if (casted == nullptr) {
                throw std::runtime_error("Setting type changed");
            }

            return casted;
        }
    };

}

#endif //ALIEN_LEXER_GENERATOR_H