#ifndef ALIEN_GENERALIZED_GENERATOR_H
#define ALIEN_GENERALIZED_GENERATOR_H

#include <fstream>
#include <string>
#include "config/settings/settings_parser.h"
#include "input/input.h"
#include "util/u8string.h"

namespace alien::generalized {

    template<typename T>
    class generalized_generator {
    protected:
        std::ifstream stream;
        std::ofstream output;

        config::settings::settings configuration;

        T ruleset;

        util::u8string start_code, end_code;

    public:
        generalized_generator(const std::string& input_file, const std::string& output_file, bool gen_header) {
            stream.open(input_file);
            output.open(output_file + (gen_header ? ".h" : ".cpp"));
        }

        void generate() {
            input::stream_input input(stream);

            start_code = scan_start_code(input);

            init_settings();
            parse_settings(input);

            parse_rules(input);

            end_code = scan_end_code(input);

            emit();
        }

    protected:
        util::u8string scan_start_code(input::stream_input& input) {
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

        util::u8string scan_end_code(input::stream_input& input) {
            util::u8char c = input.get();

            util::u8string code;

            while (c != -2) {
                code += c;

                c = input.get();
            }

            return code;
        }

        void parse_settings(input::stream_input& input) {
            config::settings::lexer l(input);
            config::settings::parser p(configuration, l);

            p.parse();
        }

        void emit() {
            emit_pre_start_code();

            output << util::u8string_to_bytes(start_code);

            emit_pre_end_code();

            output << util::u8string_to_bytes(end_code);
        }

        virtual ~generalized_generator() = default;

    protected:
        template<typename V>
        V* check(const util::u8string&& accessor) {
            auto* casted = dynamic_cast<V*>(configuration.config.at(accessor).get());

            if (casted == nullptr) {
                throw std::runtime_error("Setting type changed");
            }

            return casted;
        }

    protected:
        virtual void init_settings() = 0;

        virtual void parse_rules(input::stream_input&) = 0;

        virtual void emit_pre_start_code() = 0;

        virtual void emit_pre_end_code() = 0;
    };
}

#endif //ALIEN_GENERALIZED_GENERATOR_H