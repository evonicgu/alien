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
#include "util/u8string.h"

namespace alien::lexer {

    using namespace alien::config::settings;
    using namespace util::literals;

    class generator {
        std::ifstream stream;
        std::ofstream output;

        settings configuration = {
                {
                        {"generation.token_type"_u8, std::make_shared<string_value>("generalized_token"_u8)},
                        {"generation.use_macros"_u8, std::make_shared<bool_value>(false)},
                        {"generation.use_enum_class"_u8, std::make_shared<bool_value>(true)},
                        {"generation.error_function"_u8, std::make_shared<string_value>("error"_u8)},
                        {"general.enable_trailing_return"_u8, std::make_shared<bool_value>(true)},
                },
        };

        config::rules::rules ruleset;

        util::u8string start_code, end_code;

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
        static util::u8string scan_start_code(input::stream_input& input) {
            util::u8char c = input.get();

            util::u8string code;

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

        static util::u8string scan_end_code(input::stream_input& input) {
            util::u8char c = input.get();

            util::u8string code;

            while (c != -2) {
                code += c;

                c = input.get();
            }

            return code;
        }

        void parse_settings(input::stream_input& input) {
            alien::config::settings::lexer l(input);
            alien::config::settings::parser p(configuration, l);

            p.parse();
        }

        void parse_rules(input::stream_input& input) {
            auto* val = check<bool_value>("general.enable_trailing_return"_u8);

            config::rules::lexer::lexer l(val->val, input);
            config::rules::parser::parser p(ruleset, l);

            p.parse();

            if (ruleset.eof_action.code.empty()) {
                ruleset.eof_action.code = util::ascii_to_u8string(default_eof_action);
            }
        }

        void emit() {
            emit_required_code();

            emit_macros();

            emit_token_class();

            emit_token_enum();

            emit_possible_error_function();

            output << util::u8string_to_bytes(start_code);

            emit_start_lexer();

            emit_dfa();

            emit_lexer_mid();

            emit_lexer_end();

            output << util::u8string_to_bytes(end_code);

            std::cout << "done";
        }

        void emit_required_code() {
            output << required_code;
        };

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
                for (const auto& token : configuration.symbols) {
                    const util::u8string& name = token.first;

                    util::u8string macro = "#define _"_u8;
                    macro += name + " return new "_u8;
                    macro += token_type->str + "<token_type>("_u8;
                    macro += name + ")\n"_u8;

                    output << util::u8string_to_bytes(macro);
                }

                output << '\n';
            }
        }

        void emit_start_lexer() {
            output << lexer_start;
        }

        void emit_token_enum() {
            auto* val = check<bool_value>("generation.use_enum_class"_u8);

            output << "enum" << (val->val ? " class " : " ") << "token_type {\n";

            for (const auto& token : configuration.symbols) {
                const util::u8string& name = token.first;

                output << "    " << util::u8string_to_bytes(name) << ",\n";
            }

            output << "};\n\n";
        }

        void emit_possible_error_function() {
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

            output << "};\n\n" << lexer_constructor_start << start_states[0] << lexer_constructor_end;
        }

        automata::dfa::dfa build_dfa(unsigned int context) {
            alien::automata::nfa::state_ptr start_nfa_state = std::make_shared<automata::nfa::state>();

            start_nfa_state->accepting = false;

            std::set<util::u8char> A;

            for (auto& rule : ruleset.ruleset[context]) {
                input::string_input i(std::move(rule.regex));

                regex::parser::parser p{regex::lexer::lexer(i)};

                p.parse();

                alien::lexer::regex::parser::ast::node_ptr ast = p.get_ast();
                auto result = automata::algorithm::nfa_from_tree(ast, rule.rule_number);

                std::set<util::u8char> m_alphabet;

                std::merge(A.begin(), A.end(),
                           result.second.begin(), result.second.end(),
                           std::inserter(m_alphabet, m_alphabet.begin()));

                A = std::move(m_alphabet);
                start_nfa_state->transitions[-1].insert(result.first);
            }

            return automata::algorithm::minimize(automata::algorithm::convert_nfa2dfa(start_nfa_state, A), A);
        }

        void emit_lexer_mid() {
            auto* token_type = check<string_value>("generation.token_type"_u8);

            output << "    " << util::u8string_to_bytes(token_type->str) << "<token_type>* lex() {\n";
            output << lex_method_start << util::u8string_to_bytes(ruleset.eof_action.code) << lex_method_after_eof;
            output << "                ";
            output << util::u8string_to_bytes(check<string_value>("generation.error_function"_u8)->str);
            output << "(line, column);\n" << lex_method_mid;

            for (unsigned int context = 0; context < ruleset.context_mapping.size(); ++context) {
                for (auto &rule: ruleset.ruleset[context]) {
                    output << "                case " << rule.rule_number << ": {\n";
                    auto &action = rule.act;

                    if (!action.trailing_return.empty()) {
                        action.code += "return new "_u8 + token_type->str + "<token_type>("_u8;
                        action.code += "token_type::"_u8 + action.trailing_return + ");"_u8;
                    }

                    output << util::u8string_to_bytes(action.code) << "\nbreak;\n                }\n";
                }
            }

            output << lex_method_end;
        }

        void emit_lexer_end() {
            output << lexer_end;
        }

        template<typename T>
        T* check(const util::u8string&& accessor) {
            auto* casted = dynamic_cast<T*>(configuration.config.at(accessor).get());

            if (casted == nullptr) {
                throw std::runtime_error("Setting type changed");
            }

            return casted;
        }
    };

}

#endif //ALIEN_LEXER_GENERATOR_H