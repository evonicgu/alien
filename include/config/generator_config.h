#ifndef ALIEN_GENERATOR_CONFIG_H
#define ALIEN_GENERATOR_CONFIG_H

#include <string>
#include <optional>

namespace alien::config {

    struct generator_config {
        // IO
        std::string input;
        std::optional<std::string> output_directory;
        std::optional<std::string> header_output_directory;

        std::string lang;

        // Generation templates
        std::optional<std::string> token_template;
        std::optional<std::string> lexer_template, lexer_header_template;
        std::optional<std::string> parser_template, parser_header_template;
    };

}

#endif //ALIEN_GENERATOR_CONFIG_H
