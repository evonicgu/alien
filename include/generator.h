#ifndef ALIEN_GENERATOR_H
#define ALIEN_GENERATOR_H

#include <cstddef>
#include <fstream>
#include <list>
#include <memory>
#include <vector>
#include <optional>

#include "inja/inja.hpp"

#include "alphabet.h"
#include "config/generator_config.h"
#include "fwd/lexer_fwd.h"
#include "fwd/parser_fwd.h"
#include "input/input.h"
#include "util/to_json.h"
#include "util/typeutils.h"
#include "util/u8string.h"
#include "lexer/lexer_generator.h"
#include "parser/parser_generator.h"

namespace alien {

    class generator {
        config::generator_config config;

        config::generator_streams streams;
        std::list<util::u8string> err;

        lexer::lexer_generator lexer_gen;
        parser::parser_generator parser_gen;

        alphabet::alphabet alphabet;
        inja::Environment env;

    public:
        explicit generator(config::generator_config&& configuration)
            : config(std::move(configuration)),
              streams(config),
              lexer_gen(config, streams, alphabet, err),
              parser_gen(config,streams, alphabet, err) {
            setup_inja_env();
        }

        void generate();

        const std::list<util::u8string>& get_errors() const;

    private:
        void setup_inja_env();

        util::u8string get_lexer_relative_namespace(const util::u8string& lexer_namespace,
                                                     const util::u8string& parser_namespace);

    private:
        static std::string create_range(std::string_view var, std::ptrdiff_t start, std::ptrdiff_t end);
    };

}

#endif //ALIEN_GENERATOR_H