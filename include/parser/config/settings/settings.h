#ifndef ALIEN_PARSER_SETTING_H
#define ALIEN_PARSER_SETTING_H

#include <list>

#include "config/settings/parser.h"
#include "config/settings/settings.h"
#include "util/u8string.h"

namespace alien::parser::settings {

    struct parser_symbol {
        util::u8string name, type;

        bool is_first = false, is_midrule = false;

        bool operator<(const parser_symbol& other) const {
            return name < other.name;
        }

        friend bool operator<(const parser_symbol& lhs, const util::u8string& rhs) {
            return lhs.name < rhs;
        }

        friend bool operator<(const util::u8string& lhs, const parser_symbol& rhs) {
            return lhs < rhs.name;
        }
    };

    class settings_parser : public config::settings::parser<parser_symbol> {
    public:
        settings_parser(lexer& l, std::list<util::u8string>& err)
                : config::settings::parser<parser_symbol>(l, err) {
            using namespace util::literals;

            configuration = {
                    {
                        {"generation.type"_u8, std::make_shared<config::settings::string_value>("lalr"_u8)},
                        {"general.start"_u8, std::make_shared<config::settings::string_value>(""_u8)},
                        {"generation.fixed_stack"_u8, std::make_shared<config::settings::bool_value>(false)},
                        {"generation.stack_size"_u8, std::make_shared<config::settings::number_value>(4096)},
                        {"symbol.namespace"_u8, std::make_shared<config::settings::string_value>(""_u8)}
                    }
            };
        }

        void specifier() override {
            using namespace util::literals;

            util::pos pos = lookahead->start;
            err.push_back("Unexpected specifier at "_u8 + (util::u8string) pos + "No specifiers are allowed"_u8);
        }

        void add_types() override {
            using namespace util::literals;

            auto& symbol_namespace = util::check<config::settings::string_value>(
                    configuration.config["symbol.namespace"_u8].get()
            )->str;

            for (std::size_t i = 0; i < configuration.symbols.size(); ++i) {
                auto& symbol = configuration.symbols[i];

                if (symbol.type.empty()) {
                    symbol.type = "void"_u8;
                } else if (!symbol_namespace.empty()) {
                    symbol.type = symbol_namespace + "::"_u8 + symbol.type;
                }
            }
        }

        void error(type expected, type got) override {
            using namespace util::literals;

            util::pos pos = lookahead->start;

            switch (expected) {
                case config::settings::token_type::T_EQUALS:
                    err.push_back("Expected '=' after setting name at "_u8 + (util::u8string) pos);
                    break;
                case config::settings::token_type::T_CLOSE_BRACE:
                    err.push_back("Expected '}' after definition block at "_u8 + (util::u8string) pos);
                    break;
                default:
                    throw std::runtime_error("Unexpected token at " + (std::string) lookahead->start);
            }
        }
    };

}

#endif //ALIEN_PARSER_SETTING_H