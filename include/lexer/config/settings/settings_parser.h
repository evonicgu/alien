#ifndef ALIEN_LEXER_CONFIG_SETTINGS_PARSER_H
#define ALIEN_LEXER_CONFIG_SETTINGS_PARSER_H

#include <memory>
#include <string>
#include "generalized/generalized_parser.h"
#include "settings.h"
#include "config/settings/settings_lexer.h"
#include "config/settings/settings_token.h"

namespace alien::lexer::config::settings::parser {

    using namespace alien::config::settings;

    static constexpr const char value_type_exception_str[] = "Wrong value type. ";
    static constexpr const char unknown_setting_exception_str[] = "Unknown setting specified. ";

    using base_parser = generalized::generalized_parser<lexer::token_type, lexer::lexer>;

    class parser : base_parser {
        settings& values;

    public:
        explicit parser(settings& values, const lex& l) : values(values), base_parser(l) {}

        explicit parser(settings& values, lex&& l) : values(values), base_parser(std::move(l)) {}

        void parse() override {
            switch (lookahead->type) {
                case type::T_HASHTAG:
                case type::T_OPEN_BRACE:
                    setting_or_definition();

                    if (lookahead->type != type::T_END) {
                        parse();
                    }

                    break;
                default:
                    throw syntax_exception("Expected token to be a setting or a definition");
            }
        }

        using value_type_exception = generalized::generalized_exception<value_type_exception_str>;
        using unknown_setting_exception = generalized::generalized_exception<unknown_setting_exception_str>;

    private:
        void setting_or_definition() {
            if (lookahead->type == type::T_HASHTAG) {
                setting();
                return;
            }

            definition();
        }

        void setting() {
            match(type::T_HASHTAG);

            std::string setting_name = dot_identifiers();

            if (values.config.find(setting_name) == values.config.end()) {
                throw unknown_setting_exception("Cannot find setting: " + setting_name);
            }

            match(type::T_EQUALS);

            set_setting(std::move(setting_name));
        }

        std::string dot_identifiers() {
            auto* token = check<lexer::identifier_token>("Expected token to be an identifier token instance");

            std::string str = std::move(token->name);
            match(type::T_IDENTIFIER);

            if (lookahead->type == type::T_DOT) {
                match(type::T_DOT);
                str += '.' + dot_identifiers();
            }

            return str;
        }

        void set_setting(std::string&& name) {
            auto* uncasted_val = values.config[name].get();

            switch (lookahead->type) {
                case type::T_STR: {
                    auto* str = check<lexer::str_token>("Expected token to be a string token instance");
                    auto* val = check<string_value, value_type_exception>(uncasted_val, "Wrong value type");

                    val->str = std::move(str->str);
                    match(type::T_STR);

                    break;
                }
                case type::T_IDENTIFIER: {
                    auto* id = check<lexer::identifier_token>("Expected token to be an identifier token instance");
                    auto* val = check<string_value, value_type_exception>(uncasted_val, "Wrong value type");

                    val->str = std::move(id->name);
                    match(type::T_IDENTIFIER);

                    break;
                }
                case type::T_NUMBER: {
                    auto* number = check<lexer::number_token>("Expected token to be a number token instance");
                    auto* val = check<number_value, value_type_exception>(uncasted_val, "Wrong value type");

                    val->number = number->number;
                    match(type::T_NUMBER);

                    break;
                }
                case type::T_BOOL: {
                    auto* bool_val = check<lexer::bool_token>("Expected token to be a bool token instance");
                    auto* val = check<bool_value, value_type_exception>(uncasted_val, "Wrong value type");

                    val->val = bool_val->value;
                    match(type::T_BOOL);

                    break;
                }
                default:
                    throw syntax_exception("Unknown value type");
            }
        }

        void definition() {
            match(type::T_OPEN_BRACE);

            if (lookahead->type == type::T_IDENTIFIER) {
                identifiers();
            }

            match(type::T_CLOSE_BRACE);
        }

        void identifiers() {
            auto* id = check<lexer::identifier_token>("Expected token to be an identifier token instance");

            values.tokens.insert(id->name);
            match(type::T_IDENTIFIER);

            if (lookahead->type == type::T_IDENTIFIER) {
                identifiers();
            }
        }
    };

}

#endif //ALIEN_LEXER_CONFIG_SETTINGS_PARSER_H