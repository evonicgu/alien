#ifndef ALIEN_LEXER_SETTINGS_H
#define ALIEN_LEXER_SETTINGS_H

#include <map>
#include <memory>
#include <string>
#include <set>

namespace alien::lexer::config::settings {

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
        std::string str;

        explicit string_value(const std::string& str) : str(str), value(value_type::STRING) {}

        explicit string_value(std::string& str) : str(std::move(str)), value(value_type::STRING) {}
    };

    struct number_value : public value {
        int number;

        explicit number_value(int number) : number(number), value(value_type::NUMBER) {}
    };

    struct bool_value : public value {
        bool val;

        explicit bool_value(bool val) : val(val), value(value_type::BOOL) {}
    };

    struct settings {
        std::map<std::string, std::shared_ptr<value>> config;

        std::set<std::string> tokens;
    };

}

#endif //ALIEN_LEXER_SETTINGS_H