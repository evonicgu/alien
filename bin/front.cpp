#include <cxxopts.hpp>
#include "lexer/generator.h"

int main(int argc, char** argv) {
    cxxopts::Options options("Alien", "Alien - front-end compiler library");

    options.add_options()
            ("linput", "Lexer input file", cxxopts::value<std::string>())
            ("pinput", "Parser input file", cxxopts::value<std::string>()->default_value(""))
            ("loutput", "Lexer output file", cxxopts::value<std::string>())
            ("poutput", "Parser output file", cxxopts::value<std::string>()->default_value(""))
            ("headers", "Generate header file", cxxopts::value<bool>()->default_value("true"));

    auto result = options.parse(argc, argv);

    std::string lexer = result["l"].as<std::string>();
    std::string parser = result["p"].as<std::string>();

    if (!lexer.empty()) {
        alien::lexer::generator gen(lexer, result["lexer_output"].as<std::string>(),result["h"].as<bool>());

        gen.generate();
    }

}