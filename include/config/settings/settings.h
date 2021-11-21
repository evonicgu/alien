#ifndef ALIEN_CONFIG_SETTINGS_H
#define ALIEN_CONFIG_SETTINGS_H

#include <map>
#include <memory>
#include "util/u8string.h"

namespace alien::config::settings {

    struct value {
        enum class value_type {
            NUMBER,
            STRING,
            BOOL
        } type;

        explicit value(value_type type) : type(type) {}

        virtual ~value() = default;
    };

    struct string_value : public value {
        util::u8string str;

        explicit string_value(const util::u8string& str) : str(str), value(value_type::STRING) {}

        explicit string_value(util::u8string&& str) : str(std::move(str)), value(value_type::STRING) {}
    };

    struct number_value : public value {
        int number;

        explicit number_value(int number) : number(number), value(value_type::NUMBER) {}
    };

    struct bool_value : public value {
        bool val;

        explicit bool_value(bool val) : val(val), value(value_type::BOOL) {}
    };

    struct symbol {
        util::u8string name, code_type;

        std::map<std::string, int> specifiers;

        bool operator<(const symbol& other) const {
            return name < other.name;
        }
    };

    struct settings {
        std::map<util::u8string, std::shared_ptr<alien::config::settings::value>> config;

        util::vecset<symbol> symbols;

        std::vector<util::u8string> code_top, code, code_content;
    };

}

#endif //ALIEN_CONFIG_SETTINGS_H