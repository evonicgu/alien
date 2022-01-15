#include <iostream>
#include <list>

#include "cxxopts.hpp"

#include "generator.h"
#include "util/u8string.h"

int main(int argc, char** argv) {
    using namespace alien::util::literals;

    bool verbose = false, quiet = false;

    try {
        cxxopts::Options options("Alien", "Alien - front-end compiler library");

        options.show_positional_help().add_options()
                ("i,input", "Input file location", cxxopts::value<std::string>())
                ("o,output", "Parser output file location", cxxopts::value<std::string>()->default_value("parser.out"))
                ("v,verbose", "Verbose error output", cxxopts::value(verbose))
                ("header", "Generates header file if true", cxxopts::value<bool>()->default_value("true"))
                ("h,help", "Prints the help message")
                ("q,quiet", "Quiet mode (no warnings, not recommended)", cxxopts::value(quiet))
                ("ltemplate", "Lexer template file", cxxopts::value<std::string>()
                        ->default_value("templates/lexer.template.txt"))
                ("ttemplate", "Lexer template file", cxxopts::value<std::string>()
                        ->default_value("templates/token.template.txt"))
                ("ptemplate", "Parser template file", cxxopts::value<std::string>()
                        ->default_value("templates/parser.template.txt"))
                ("args", "Positional args", cxxopts::value<std::vector<std::string>>());

        options.parse_positional({"input", "args"});

        auto result = options.parse(argc, argv);

        if (result.count("help") > 0) {
            std::cout << options.help() << '\n';
            return 0;
        }

        if (result.count("input") == 0) {
            std::cout << "No input file";
            return 0;
        }

        auto output = result["output"].as<std::string>();
        auto generate_headers = result["header"].as<bool>();

        std::list<alien::util::u8string> err;
        std::ifstream in(result["input"].as<std::string>());
        std::ofstream parser_out(output + (generate_headers ? ".h" : ".cpp"));

        std::string out_directory = output.substr(0, output.find_last_of("/\\") + 1);
        out_directory += "definitions.gen.h";

        std::ofstream token_out(out_directory);

        alien::generator gen(in, parser_out, token_out, err);

        gen.generate(
                result["ltemplate"].as<std::string>(),
                result["ttemplate"].as<std::string>(),
                result["ptemplate"].as<std::string>());

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
        std::cerr << "Option parsing error: " << e.what() << '\n';
        return 1;
    } catch (...) {
        if (verbose) {
            std::rethrow_exception(std::current_exception());
        }

        std::cerr << "Internal error\n";
        return 1;
    }
}