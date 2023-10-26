#include <iostream>
#include <optional>

#include "cxxopts.hpp"

#include "generator.h"
#include "util/u8string.h"

template<typename T>
auto create_value(std::optional<T>& t) {
    return cxxopts::value(t);
}

int main(int argc, char** argv) {
    using namespace alien::util::literals;

    bool verbose = false, quiet = false, time = false;

    try {
        alien::config::generator_config config;

        cxxopts::Options options("Alien", "Alien - front-end compiler library");

        options.show_positional_help().add_options()
                ("i,input", "Input file location", cxxopts::value(config.input))
                ("output", "Source file output directory", create_value(config.output_directory))
                ("header_output", "Header file output directory", create_value(config.header_output_directory))
                ("v,verbose", "Verbose error output", cxxopts::value(verbose))
                ("h,help", "Prints the help message")
                ("l,lang", "Generated parser language", cxxopts::value(config.lang)->default_value("c++"))
                ("t,time", "Print elapsed time after generation", cxxopts::value(time))
                ("q,quiet", "Quiet mode (no warnings, not recommended)", cxxopts::value(quiet))
                ("lexer_source_template", "Lexer source template file", create_value(config.lexer_template))
//                        ->default_value("resources/templates/cpp/sources/lexer.template.txt"))
                ("lexer_header_template", "Lexer header template file", create_value(config.lexer_header_template))
//                        ->default_value("resources/templates/cpp/headers/lexer.template.txt"))
                ("tokens_template", "Tokens template file", create_value(config.token_template))
//                        ->default_value("resources/templates/cpp/sources/token.template.txt"))
                ("parser_source_template", "Parser source template file", create_value(config.parser_template))
//                        ->default_value("resources/templates/cpp/sources/parser.template.txt"))
                ("parser_header_template", "parser header template file", create_value(config.parser_header_template));
//                        ->default_value("resources/templates/cpp/headers/parser.template.txt"));

        options.parse_positional({"input"});

        auto result = options.parse(argc, argv);

        if (result.count("help") > 0) {
            std::cout << options.help() << '\n';
            return 0;
        }

        if (result.count("input") == 0) {
            std::cerr << "No input file\n";
            return 1;
        }

        auto start = std::chrono::high_resolution_clock::now();

        alien::generator gen(std::move(config));

        gen.generate();

        auto end = std::chrono::high_resolution_clock::now();

        if (time) {
            std::cout << "Time spent generating frontend: ";
            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
        }

        const auto& err = gen.get_errors();

        if (!err.empty()) {
            if (!quiet) {
                for (const auto& str : err) {
                    std::cerr << alien::util::u8string_to_bytes(str) << '\n';
                }
            }

            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Configuration error:\n" << e.what() << '\n';
        return 1;
    } catch (const cxxopts::OptionException& e) {
        std::cerr << "Option parsing error:\n" << e.what() << '\n';
        return 1;
    } catch (...) {
        if (verbose) {
            std::rethrow_exception(std::current_exception());
        }

        std::cerr << "Internal error\n";
        return 1;
    }
}