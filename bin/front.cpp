#include <utility>
#include <cxxopts.hpp>
#include "lexer/generator.h"
#include "parser/generator.h"

int main(int argc, char** argv) {
    cxxopts::Options options("Alien", "Alien - front-end compiler library");

    options.add_options()
            ("linput", "Lexer input file", cxxopts::value<std::string>())
            ("pinput", "Parser input file", cxxopts::value<std::string>()->default_value(""))
            ("loutput", "Lexer output file", cxxopts::value<std::string>())
            ("poutput", "Parser output file", cxxopts::value<std::string>()->default_value(""))
            ("headers", "Generate header file", cxxopts::value<bool>()->default_value("true"))
            ("ptype", "Parser type", cxxopts::value<std::string>()->default_value("lalr"));

    auto result = options.parse(argc, argv);

    std::string lexer = result["linput"].as<std::string>();
    std::string parser = result["pinput"].as<std::string>();

    alien::config::settings::settings lexer_settings;

    if (!lexer.empty()) {
        alien::lexer::generator gen(lexer, result["loutput"].as<std::string>(),result["headers"].as<bool>());

        lexer_settings = gen.generate();
    }

    if (!parser.empty() && !lexer.empty()) {
        alien::parser::generator gen(parser,
                                     result["poutput"].as<std::string>(),
                                     result["headers"].as<bool>(),
                                     result["ptype"].as<std::string>(),
                                     std::move(lexer_settings));

        gen.generate();
    }

}