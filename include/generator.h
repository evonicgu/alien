#ifndef ALIEN_GENERATOR_H
#define ALIEN_GENERATOR_H

#include <string>
#include <list>
#include <memory>

#include "inja/inja.hpp"

#include "config/generator_config.h"
#include "util/u8string.h"
#include "languages/base_language.h"

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