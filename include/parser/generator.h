#ifndef ALIEN_PARSER_GENERATOR_H
#define ALIEN_PARSER_GENERATOR_H

#include <fstream>
#include <stdexcept>
#include <string>
#include "algorithm/algofwd.h"
#include "config/settings/settings.h"
#include "config/rules/rules_parser.h"

namespace alien::parser {

    using base_generator = generalized::generalized_generator<config::rules::rules>;

    using namespace alien::config::settings;
    using namespace alien::parser::config::rules;
    using namespace util::literals;

    class generator : public base_generator {
        settings lexer_settings;

        const std::string &type;

        alphabet symbols;
        algorithm::parsing_table table;

        void init_gen() {
            if (type == "slr") {
                table = algorithm::slr::build_table(ruleset, symbols);
            } else if (type == "clr") {
                table = algorithm::clr::build_table(ruleset, symbols);
            } else if (type == "lalr") {
                table = algorithm::lalr::build_table(ruleset, symbols);
            } else {
                throw std::invalid_argument("Invalid parser type: " + type);
            }
        }

    public:
        generator(const std::string &input_file,
                  const std::string &output_file,
                  bool gen_header,
                  const std::string &type,
                  const settings& settings) : lexer_settings(settings), type(type),
                                              base_generator(input_file, output_file, gen_header) {}

        generator(const std::string &input_file,
                  const std::string &output_file,
                  bool gen_header,
                  const std::string &type,
                  settings &&settings) : lexer_settings(std::move(settings)), type(type),
                                         base_generator(input_file, output_file, gen_header) {}

    private:
        void init_settings() override {
            configuration = {
                {
                    {"general.start"_u8, std::make_shared<string_value>(""_u8)}
                }
            };
        }

        void emit_pre_start_code() override {
            //TODO: add code generation
        }

        void emit_pre_end_code() override {
            //TODO: add code generation
        }

        void parse_rules(input::stream_input& input) override {
            config::rules::lexer::lexer l{input};

            config::rules::parser::parser p{ruleset, std::move(l), std::move(lexer_settings), std::move(configuration)};

            p.parse();

            symbols = std::move(p.get_alphabet());

            init_gen();
        }

    };

}

#endif //ALIEN_PARSER_GENERATOR_H