#include "parser/config/settings/settings.h"

namespace alien::parser::settings {

    bool parser_symbol::operator<(const parser_symbol& other) const {
        return name < other.name;
    }

    bool operator<(const parser_symbol& lhs, const util::u8string& rhs) {
        return lhs.name < rhs;
    }

    bool operator<(const util::u8string& lhs, const parser_symbol& rhs) {
        return lhs < rhs.name;
    }

    void settings_parser::specifier() {
        using namespace util::literals;

        util::pos pos = lookahead->start;
        err.push_back("Unexpected specifier at "_u8 + (util::u8string) pos + "No specifiers are allowed"_u8);
    }

    void settings_parser::add_types() {
        using namespace util::literals;

        util::u8string& symbol_type = util::check<config::settings::string_value>(
                configuration.config["generation.symbol_type"_u8].get()
        )->str;

        if (symbol_type.empty()) {
            err.push_back("Symbol type, a common type for all symbols, must be defined"_u8);
        }

        util::u8string& symbol_namespace = util::check<config::settings::string_value>(
                configuration.config["symbol.namespace"_u8].get()
        )->str;

        if (!symbol_namespace.empty()) {
            symbol_type = symbol_namespace + "::"_u8 + symbol_type;
        }

        for (std::size_t i = 0; i < configuration.symbols.size(); ++i) {
            auto& symbol = configuration.symbols[i];

            if (symbol.type.empty()) {
                symbol.type = "void"_u8;
            } else if (!symbol_namespace.empty()) {
                symbol.type = symbol_namespace + "::"_u8 + symbol.type;
            }
        }
    }

    void settings_parser::error(config::settings::token_type expected, config::settings::token_type got) {
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

    void settings_parser::setup_settings() {
        using namespace util::literals;

        std::pair<util::u8string, std::unique_ptr<config::settings::value>> values[] = {
                {"generation.type"_u8, std::make_unique<config::settings::string_value>("lalr"_u8)},
                {"general.start"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"generation.symbol_type"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"symbol.namespace"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"generation.namespace"_u8, std::make_unique<config::settings::string_value>("parser"_u8)},
                {"generation.cpp.no_default_constructor"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.custom_error"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.use_token_to_str"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.default_token_to_str"_u8, std::make_unique<config::settings::bool_value>(false)}
        };

        for (auto& value : values) {
            configuration.config.insert(std::move(value));
        }
    }
}