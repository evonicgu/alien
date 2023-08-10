#include <iostream>
#include <optional>

#include "cxxopts.hpp"

#include "generator.h"
#include "util/u8string.h"

auto create_empty_value() {
    return cxxopts::value<std::optional<std::string>>()->default_value("");
}

int main(int argc, char** argv) {
    using namespace alien::util::literals;

    bool verbose = false, quiet = false, header_only = false, time = false;

    try {
        cxxopts::Options options("Alien", "Alien - front-end compiler library");

        options.show_positional_help().add_options()
                ("i,input", "Input file location", cxxopts::value<std::string>())
                ("output", "Source file output directory", create_empty_value())
                ("header_output", "Header file output directory", create_empty_value())
                ("v,verbose", "Verbose error output", cxxopts::value(verbose))
                ("header_only", "Generates header-only code", cxxopts::value(header_only))
                ("h,help", "Prints the help message")
                ("l,lang", "Generated parser language", cxxopts::value<std::string>()->default_value("c++"))
                ("t,time", "Print elapsed time after generation", cxxopts::value(time))
                ("q,quiet", "Quiet mode (no warnings, not recommended)", cxxopts::value(quiet))
                ("lexer_source_template", "Lexer source/header-only template file", create_empty_value())
//                        ->default_value("resources/templates/cpp/sources/lexer.template.txt"))
                ("lexer_header_template", "Lexer header (non header-only) template file", create_empty_value())
//                        ->default_value("resources/templates/cpp/headers/lexer.template.txt"))
                ("tokens_template", "Tokens template file", create_empty_value())
//                        ->default_value("resources/templates/cpp/sources/token.template.txt"))
                ("parser_source_template", "Parser source/header-only template file", create_empty_value())
//                        ->default_value("resources/templates/cpp/sources/parser.template.txt"))
                ("parser_header_template", "parser header (non header-only) template file", create_empty_value());
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

        alien::config::generator_config config{
                result["input"].as<std::string>(),
                result["output"].as<std::optional<std::string>>(),
                result["header_output"].as<std::optional<std::string>>(),

                alien::config::language_from_string(result["lang"].as<std::string>()),

                verbose,
                quiet,
                header_only,

                result["tokens_template"].as<std::optional<std::string>>(),
                result["lexer_source_template"].as<std::optional<std::string>>(),
                result["lexer_header_template"].as<std::optional<std::string>>(),
                result["parser_source_template"].as<std::optional<std::string>>(),
                result["parser_header_template"].as<std::optional<std::string>>()
        };

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