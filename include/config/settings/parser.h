#ifndef ALIEN_SETTINGS_PARSER_H
#define ALIEN_SETTINGS_PARSER_H

#include <list>
#include <memory>
#include <utility>

#include "lexer.h"
#include "settings.h"
#include "token.h"
#include "util/parser.h"
#include "util/typeutils.h"
#include "util/u8string.h"
#include "parser/config/rules/lexer.h"

namespace alien::config::settings {

    const util::u8string error_t = util::ascii_to_u8string("error");

    template<typename T>
    class parser : public util::parser<token_type> {
        using vt = value::value_type;

    protected:
        settings<T> configuration;
        util::vecset<T>& store;

    public:
        explicit parser(lexer_t& l, std::list<util::u8string>& err, util::vecset<T>& store)
            : util::parser<token_type>(l, err),
              store(store) {}

        void parse() override {
            while (lookahead->type != type::T_END) {
                switch (lookahead->type) {
                    case type::T_HASHTAG:
                        setting();
                        break;
                    case type::T_OPEN_BRACE:
                        definition();
                        break;
                    case type::T_CODE:
                        code_block();
                        break;
                    case type::T_SPECIFIER:
                        specifier();
                        break;
                    default: {
                        util::pos err_pos = lookahead->start;

                        throw std::runtime_error(
                                "Expected setting, definition, code block or specifier at " + (std::string) err_pos
                        );
                    }
                }
            }
        }

        settings<T>&& get_settings() {
            add_types();

            return std::move(configuration);
        }

    protected:
        virtual void specifier() = 0;

        virtual void add_types() = 0;

    private:
        std::pair<util::u8string, util::pos> dot_identifier() {
            auto* token = check<identifier_token>();
            util::pos start = token->start;

            util::u8string str = std::move(token->name);
            match(type::T_IDENTIFIER);

            while (lookahead->type == type::T_DOT) {
                match(type::T_DOT);
                str += '.';

                if (lookahead->type != type::T_IDENTIFIER) {
                    err.push_back("Expected identifier after '.' at "_u8 + (util::u8string) lookahead->start);
                    str.pop_back();
                    continue;
                }

                token = check<identifier_token>();
                str += token->name;
                match(type::T_IDENTIFIER);
            }

            return {str, start};
        }

        void setting() {
            match(type::T_HASHTAG);

            auto [name, pos] = dot_identifier();

            auto it = configuration.config.find(name);

            if (it == configuration.config.end()) {
                err.push_back("Unknown setting name: '"_u8 + name + "' at "_u8 + (util::u8string) pos);
                match(type::T_EQUALS);
                match(lookahead->type);
                return;
            }

            match(type::T_EQUALS);

            set_value(it->first, it->second);
        }

        template<vt Tp, typename TTp, typename VTp>
        std::pair<TTp*, VTp*> check_value(bool& type_err, std::unique_ptr<value>& setting_ptr) {
            if (setting_ptr->type != Tp) {
                type_err = true;
                return {nullptr, nullptr};
            }

            return {check<TTp>(), util::check<VTp, value>(setting_ptr.get())};
        }

        void set_value(const util::u8string& name, std::unique_ptr<value>& set_value) {
            bool type_error = false;

            util::pos pos = lookahead->start;

            switch (lookahead->type) {
                case type::T_STR: {
                    auto [val, set] = check_value<vt::STRING, str_token, string_value>(type_error, set_value);

                    if (!type_error) {
                        set->str = std::move(val->str);
                    }

                    break;
                }
                case type::T_IDENTIFIER: {
                    auto [val, set] = check_value<vt::STRING, identifier_token, string_value>(type_error, set_value);

                    if (!type_error) {
                        set->str = std::move(val->name);
                    }

                    break;
                }
                case type::T_NUMBER: {
                    auto [val, set] = check_value<vt::NUMBER, number_token, number_value>(type_error, set_value);

                    if (!type_error) {
                        set->number = val->number;
                    }

                    break;
                }
                case type::T_BOOL: {
                    auto [val, set] = check_value<vt::BOOL, bool_token, bool_value>(type_error, set_value);

                    if (!type_error) {
                        set->val = val->value;
                    }

                    break;
                }
                default:
                    err.push_back(
                            "Expected a constant after assignment operator at "_u8 + (util::u8string) pos
                            );
            }

            if (type_error) {
                err.push_back(
                        "Wrong type used in assignment to "_u8 + name + " at "_u8 + (util::u8string) pos
                        );
            }

            match(lookahead->type);
        }

        void definition() {
            match(type::T_OPEN_BRACE);

            while (lookahead->type == type::T_IDENTIFIER) {
                util::pos pos = lookahead->start;

                util::u8string name = std::move(check<identifier_token>()->name);

                if (name == error_t) {
                    throw std::runtime_error("Cannot use predefined name 'error' at pos " + (std::string) pos);
                }

                T symbol;
                symbol.name = std::move(name);

                match(type::T_IDENTIFIER);

                auto it = store.find(symbol);

                if (it != store.vend()) {
                    err.push_back("Redefinition of symbol '"_u8 + it->name + "' at "_u8 + (util::u8string) pos);
                }

                if (lookahead->type == type::T_EQUALS) {
                    match(type::T_EQUALS);

                    symbol.type = std::move(check<identifier_token>()->name);
                    match(type::T_IDENTIFIER);
                }

                store.push_back(symbol);

                if (lookahead->type == type::T_COMMA) {
                    match(type::T_COMMA);
                }
            }

            match(type::T_CLOSE_BRACE);
        }

        void code_block() {
            auto* block = check<code_token>();

            configuration.code_declarations[block->loc].push_back(std::move(block->code));

            match(type::T_CODE);
        }
    };

}

#endif //ALIEN_SETTINGS_PARSER_H