#ifndef ALIEN_GENERATOR_CONFIG_H
#define ALIEN_GENERATOR_CONFIG_H

#include <filesystem>
#include <string>
#include <optional>
#include <fstream>

#include "input/input.h"

namespace alien::config {

    struct generator_config {
        // IO
        std::string input;
        std::optional<std::string> output_directory;
        std::string header_output_directory;

        // Settings
        bool verbose = false;
        bool quiet = false;
        bool header_only = false;

        // Generation templates
        std::string token_template;
        std::string lexer_template, lexer_header_template;
        std::string parser_template, parser_header_template;
    };

    struct generator_streams {
        input::stream_input in;
        std::optional<std::ofstream> parser_source_out;
        std::ofstream parser_header_out, token_out;

        explicit generator_streams(const generator_config& config)
                : in(config.input) {
            if (!config.header_only) {
                std::filesystem::path source_gen_dir = config.output_directory.value();
                ensure_directory_exists(source_gen_dir);

                std::filesystem::path source_gen_file = source_gen_dir / "parser.gen.cpp";

                parser_source_out = std::ofstream(source_gen_file);

                check_out_stream(parser_source_out.value());
            }

            std::filesystem::path header_gen_dir = config.header_output_directory;
            ensure_directory_exists(header_gen_dir);

            std::filesystem::path header_gen_file = header_gen_dir / "parser.gen.h";
            std::filesystem::path token_gen_file = header_gen_dir / "token.gen.h";

            parser_header_out = std::ofstream(header_gen_file);
            check_out_stream(parser_header_out);

            token_out = std::ofstream(token_gen_file);
            check_out_stream(token_out);
        }

    private:
        static void check_out_stream(std::ofstream& out_stream);

        static void ensure_directory_exists(const std::filesystem::path& directory);
    };

}

#endif //ALIEN_GENERATOR_CONFIG_H
