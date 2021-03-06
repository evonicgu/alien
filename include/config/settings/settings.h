#ifndef ALIEN_SETTINGS_H
#define ALIEN_SETTINGS_H

#include <map>
#include <memory>
#include <vector>

#include "util/u8string.h"
#include "util/vecset.h"

namespace alien::config::settings {

    struct value {
        enum class value_type {
            NUMBER,
            STRING,
            BOOL
        } type;

        explicit value(value_type type)
            : type(type) {}

        virtual ~value() = default;
    };

    struct string_value : public value {
        util::u8string str;

        explicit string_value(util::u8string&& str)
            : str(std::move(str)),
              value(value_type::STRING) {}
    };

    struct number_value : public value {
        long long number;

        explicit number_value(long long number)
            : number(number),
              value(value_type::NUMBER) {}
    };

    struct bool_value : public value {
        bool val;

        explicit bool_value(bool val)
            : val(val),
              value(value_type::BOOL) {}
    };

    template<typename T>
    struct settings {
        std::map<util::u8string, std::shared_ptr<alien::config::settings::value>> config;

        util::vecset<T> symbols;

        std::vector<util::u8string> code_top, code, code_content;
    };


}

#endif //ALIEN_SETTINGS_H