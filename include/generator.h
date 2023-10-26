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
#include "renderer/base_renderer.h"
#include "config/generator_config.h"
#include "fwd/lexer_fwd.h"
#include "fwd/parser_fwd.h"
#include "input/input.h"
#include "util/to_json.h"
#include "util/typeutils.h"
#include "util/u8string.h"
#include "lexer/lexer_generator.h"
#include "parser/parser_generator.h"
#include "renderer/cpp_renderer.h"
#include "languages/base_language.h"
#include "languages/cpp_language.h"

namespace alien {

    class generator {
        config::generator_config config;

        std::list<util::u8string> err;

        inja::Environment env;

        std::map<std::string, std::unique_ptr<languages::base_language>> languages;

    public:
        explicit generator(config::generator_config&& configuration)
            : config(std::move(configuration)) {
            setup_inja_env();
            setup_default_languages();
        }

        void add_language(const std::string& name, std::unique_ptr<languages::base_language>&& language);

        std::unique_ptr<languages::base_language>& find_language(const std::string& name);

        void generate();

        const std::list<util::u8string>& get_errors() const;

    private:
        void setup_inja_env();

        void setup_default_languages();

    private:
        static std::string create_range(std::string_view var, std::ptrdiff_t start, std::ptrdiff_t end);
    };

}

#endif //ALIEN_GENERATOR_H