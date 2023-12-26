#ifndef ALIEN_PARSER_GENERATOR_H
#define ALIEN_PARSER_GENERATOR_H

#include <list>
#include <vector>
#include <memory>

#include "nlohmann/json.hpp"

#include "util/u8string.h"
#include "alphabet.h"
#include "input/input.h"
#include "parser/generator/base_table_generator.h"
#include "parser/config/rules/rules.h"
#include "parser/config/settings/settings.h"
#include "languages/base_language.h"

namespace alien::parser {

    class parser_generator {
        rules::rules parser_rules;

        std::list<util::u8string>& err;
        alphabet::alphabet& alphabet;

        input::stream_input& input_stream;

        std::unique_ptr<generator::base_table_generator> table_generator;

        const std::unique_ptr<languages::base_language>& language;

    public:
        parser_generator(input::stream_input& input_stream,
                         const std::unique_ptr<languages::base_language>& language,
                         alphabet::alphabet& alphabet,
                         std::list<util::u8string>& err)
                : input_stream(input_stream),
                  language(language),
                  alphabet(alphabet),
                  err(err) {}

        settings::settings_t parse_parser_config();

        std::optional<nlohmann::json> generate_parser();

    private:
        template<typename Generator>
        std::unique_ptr<generator::base_table_generator> make_gen() {
            return std::make_unique<Generator>(alphabet, parser_rules);
        }

    private:
        std::vector<std::vector<std::vector<std::ptrdiff_t>>> get_parser_rules();

        std::vector<std::vector<std::size_t>> get_parser_lengths();

        std::vector<std::vector<std::string>> get_parser_actions();
    };

}

#endif //ALIEN_PARSER_GENERATOR_H