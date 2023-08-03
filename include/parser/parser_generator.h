#ifndef ALIEN_PARSER_GENERATOR_H

#define ALIEN_PARSER_GENERATOR_H

#include <vector>
#include <memory>

#include "inja/inja.hpp"

#include "util/u8string.h"
#include "util/typeutils.h"
#include "config/settings/settings.h"
#include "config/generator_config.h"
#include "config/rules/lexer.h"
#include "config/rules/parser.h"
#include "generator/slr.h"
#include "generator/clr.h"
#include "generator/lalr.h"
#include "util/to_json.h"

namespace alien::parser {

    class parser_generator {
        using settings_t = config::settings::settings<settings::parser_symbol>;

        rules::rules parser_rules;

        std::list<util::u8string>& err;
        alphabet::alphabet& alphabet;

        const config::generator_config& generator_config;

        config::generator_streams& generator_streams;

        std::unique_ptr<generator::base_table_generator> table_generator;

        settings_t parser_settings;

    public:
        parser_generator(const config::generator_config& generator_config,
                        config::generator_streams& generator_streams,
                        alphabet::alphabet& alphabet,
                        std::list<util::u8string>& err)
                : generator_config(generator_config),
                  generator_streams(generator_streams),
                  alphabet(alphabet),
                  err(err) {}

        const std::unique_ptr<config::settings::value>& get_param(const util::u8string& param) const;

        void parse_parser_config();

        void generate_parser(inja::Environment& env, const util::u8string& guard_prefix, bool track_lines,
                             util::u8string&& lexer_relative_namespace, bool monomorphic);

    private:
        template<typename Generator>
        std::unique_ptr<generator::base_table_generator> make_gen() {
            return std::make_unique<Generator>(alphabet, parser_rules);
        }

        static bool get_value(const std::unique_ptr<config::settings::value>& ptr);

    private:
        std::vector<std::vector<std::vector<std::ptrdiff_t>>> get_parser_rules();

        std::vector<util::u8string> get_parser_types();

        std::vector<std::vector<std::size_t>> get_parser_lengths();

        std::vector<std::vector<util::u8string>> get_parser_actions();
    };

}

#endif //ALIEN_PARSER_GENERATOR_H