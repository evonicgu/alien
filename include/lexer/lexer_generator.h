#ifndef ALIEN_LEXER_GENERATOR_H
#define ALIEN_LEXER_GENERATOR_H

#include <list>
#include <memory>
#include <optional>
#include <vector>

#include "input/input.h"
#include "alphabet.h"
#include "lexer/config/settings/settings.h"
#include "util/u8string.h"
#include "lexer/config/rules/rules.h"
#include "nlohmann/json.hpp"
#include "languages/base_language.h"

namespace alien::lexer {

    class lexer_generator {
        rules::rules lexer_rules;

        std::list<util::u8string>& err;
        alphabet::alphabet& alphabet;

        std::unique_ptr<languages::base_language>& language;

        input::stream_input& input_stream;

        bool no_utf8;

    public:
        lexer_generator(input::stream_input& input_stream,
                        std::unique_ptr<languages::base_language>& language,
                        alphabet::alphabet& alphabet,
                        std::list<util::u8string>& err)
                : input_stream(input_stream),
                  language(language),
                  alphabet(alphabet),
                  err(err) {}

        settings::settings_t parse_lexer_config();

        std::optional<nlohmann::json> generate_lexer();

        std::vector<bool> get_default_token_type_info() const;
    };

}

#endif //ALIEN_LEXER_GENERATOR_H