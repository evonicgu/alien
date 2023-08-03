#include "lexer/config/settings/settings.h"

namespace alien::lexer::settings {

    bool lexer_symbol::operator<(const lexer_symbol& other) const {
        return name < other.name;
    }

    void settings_parser::setup_settings() {
        using namespace util::literals;

        std::pair<util::u8string, std::unique_ptr<config::settings::value>> values[] = {
                {"generation.token_type"_u8, std::make_unique<config::settings::string_value>("default"_u8)},
                {"generation.position_type"_u8, std::make_unique<config::settings::string_value>("default"_u8)},
                {"generation.macros"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.enum_class"_u8, std::make_unique<config::settings::bool_value>(true)},
                {"generation.custom_error"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.noutf8"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.track_lines"_u8, std::make_unique<config::settings::bool_value>(true)},
                {"generation.buffer_size"_u8, std::make_unique<config::settings::number_value>(131072)},
                {"generation.lexeme_size"_u8, std::make_unique<config::settings::number_value>(32768)},
                {"generation.emit_stream"_u8, std::make_unique<config::settings::bool_value>(true)},
                {"generation.namespace"_u8, std::make_unique<config::settings::string_value>("lexer"_u8)},
                {"generation.no_default_constructor"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.monomorphic"_u8, std::make_unique<config::settings::bool_value>(false)},
                {"generation.stream_type"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"general.guard_prefix"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"token.namespace"_u8, std::make_unique<config::settings::string_value>(""_u8)},
                {"generation.path_to_header"_u8, std::make_unique<config::settings::string_value>(""_u8)}
        };

        for (auto& value : values) {
            configuration.config.insert(std::move(value));
        }
    }

    void settings_parser::add_types() {
        using namespace util::literals;

        util::u8string& token_type = util::check<config::settings::string_value>(
                configuration.config["generation.token_type"_u8].get()
        )->str;

        util::u8string& symbol_namespace = util::check<config::settings::string_value>(
                configuration.config["token.namespace"_u8].get()
        )->str;

        util::u8string& position_type = util::check<config::settings::string_value>(
                configuration.config["generation.position_type"_u8].get()
        )->str;

        if (token_type == "default"_u8) {
            token_default = true;
            token_type = "token"_u8;
        } else if (!symbol_namespace.empty()) {
            token_type = symbol_namespace + "::"_u8 + token_type;
        }

        if (position_type == "default"_u8) {
            position_default = true;
            position_type = "position"_u8;
        } else if (!symbol_namespace.empty()) {
            position_type = symbol_namespace + "::"_u8 + position_type;
        }

        for (std::size_t i = 0; i < configuration.symbols.size(); ++i) {
            auto& symbol = configuration.symbols[i];

            if (symbol.type.empty()) {
                symbol.type = default_token_typename;
                symbol.default_type = true;
            } else if (!symbol_namespace.empty()) {
                symbol.type = symbol_namespace + "::"_u8 + configuration.symbols[i].type;
            }
        }
    }

    void settings_parser::specifier() {
        using namespace util::literals;

        auto* spec = check<config::settings::specifier_token>();
        util::pos spec_pos = spec->start;

        std::ptrdiff_t prec = prec_level++, assoc = -1;

        if (spec->name == "left"_u8) {
            assoc = 0;
        } else if (spec->name == "right"_u8) {
            assoc = 1;
        } else if (spec->name == "nonassoc"_u8) {
            assoc = -2;
        } else if (spec->name != "prec"_u8) {
            err.push_back("Unknown specifier '"_u8 + spec->name + "' at "_u8 + (util::u8string) spec_pos);
            match(type::T_SPECIFIER);

            // discard following identifiers
            while (lookahead->type == type::T_IDENTIFIER) {
                match(lookahead->type);
            }

            return;
        }

        match(type::T_SPECIFIER);

        if (lookahead->type != type::T_IDENTIFIER) {
            err.push_back("No symbols assigned to specifier at "_u8 + (util::u8string) spec_pos);
            return;
        }

        while (lookahead->type == type::T_IDENTIFIER) {
            lexer_symbol symbol{
                    .name = std::move(check<config::settings::identifier_token>()->name)
            };

            util::u8string s_pos = (util::u8string) lookahead->start;
            auto it = configuration.symbols.find(symbol);

            if (it == configuration.symbols.vend()) {
                err.push_back("Unknown symbol '"_u8 + symbol.name + "' at "_u8 + s_pos);
                match(type::T_IDENTIFIER);
                continue;
            }

            if (it->prec != -1) {
                err.push_back(
                        "Precedence redefinition for symbol '"_u8 + symbol.name + "' at "_u8 + s_pos
                );
            }

            it->prec = prec;
            it->assoc = assoc;
            match(type::T_IDENTIFIER);
        }
    }

    void settings_parser::error(config::settings::token_type expected, config::settings::token_type got) {
        using namespace util::literals;

        util::pos pos = lookahead->start;

        switch (expected) {
            case config::settings::token_type::T_EQUALS:
                err.push_back("Expected 'equals' sign after setting name at "_u8 + (util::u8string) pos);
                break;
            case config::settings::token_type::T_CLOSE_BRACE:
                err.push_back("Expected '}' after definition block at "_u8 + (util::u8string) pos);
                break;
            default:
                throw std::runtime_error("Unexpected token at " + (std::string) lookahead->start);
        }
    }

}