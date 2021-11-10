#ifndef ALIEN_PARSER_CONFIG_RULES_PARSER_H
#define ALIEN_PARSER_CONFIG_RULES_PARSER_H

#include "config/settings/settings.h"
#include "generalized/generalized_exception.h"
#include "generalized/generalized_parser.h"
#include "rules.h"
#include "rules_lexer.h"
#include "util/u8string.h"

namespace alien::parser::config::rules::parser {

    static constexpr const char invalid_grammar_exception_str[] = "Invalid grammar. ";

    using base_parser = generalized::generalized_parser<lexer::token_type, lexer::lexer>;

    using namespace util::literals;
    using namespace alien::config::settings;

    class parser : public base_parser {
        rules& ruleset;

        settings lexer_settings, parser_settings;

        alphabet symbols;

        void init_alphabet() {
            auto* start_name = check<string_value, alien::config::settings::parser::value_type_exception>(
                    parser_settings.config["general.start"_u8].get(),
                    "Expected general.start setting to be string_value"_u8);

            bool found_start = false;

            // augmented grammar start
            grammar_symbol ag_start{start_name->str + "'"_u8, ""_u8, grammar_symbol::symbol_type::NON_TERMINAL};
            symbols.push_back(ag_start);

            for (unsigned int i = 0; i < parser_settings.symbols.size(); ++i) {
                grammar_symbol tmp;

                tmp.name = std::move(parser_settings.symbols[i].first);
                tmp.code_type = std::move(parser_settings.symbols[i].second);
                tmp.type = grammar_symbol::symbol_type::NON_TERMINAL;

                unsigned int it = symbols.push_back(tmp);

                if (tmp.name == start_name->str) {
                    found_start = true;
                    ruleset.start = it;
                }
            }

            if (!found_start) {
                throw invalid_grammar_exception("Unknown start symbol: "_u8 + start_name->str);
            }

            ruleset.ruleset.resize(symbols.size());

            ruleset.ruleset[0].push_back({{(int) ruleset.start}, {}});
            ruleset.start = 0;

            for (unsigned int i = 0; i < lexer_settings.symbols.size(); ++i) {
                grammar_symbol tmp;

                tmp.name = std::move(lexer_settings.symbols[i].first);
                tmp.code_type = std::move(lexer_settings.symbols[i].second);
                tmp.type = grammar_symbol::symbol_type::TERMINAL;

                symbols.push_back(tmp);
            }
        }

    public:
        parser(rules& ruleset, const lex& l, const settings& lexer_settings, const settings& parser_settings) :
                                                                               ruleset(ruleset),
                                                                               lexer_settings(lexer_settings),
                                                                               parser_settings(parser_settings),
                                                                               base_parser(l) {
            init_alphabet();
        }

        parser(rules& ruleset, lex&& l, settings&& lexer_settings, settings&& parser_settings) :
                                                                     ruleset(ruleset),
                                                                     lexer_settings(std::move(lexer_settings)),
                                                                     parser_settings(std::move(parser_settings)),
                                                                     base_parser(std::move(l)) {
            init_alphabet();
        }

        using invalid_grammar_exception = generalized::generalized_exception<invalid_grammar_exception_str>;

        void parse() override {
            switch (lookahead->type) {
                case type::T_IDENTIFIER:
                    definition();

                    if (lookahead->type != type::T_END) {
                        parse();
                    }

                    break;
                default:
                    throw syntax_exception("Rule must start with an identifier"_u8);
            }
        }

        alphabet&& get_alphabet() {
            return std::move(symbols);
        }

    private:
        void definition() {
            auto* token = check<lexer::identifier_token>("Expected token to be an identifier token instance"_u8);

            grammar_symbol tmp = {std::move(token->name), {}, grammar_symbol::symbol_type::NON_TERMINAL};
            auto it = symbols.find(tmp);

            if (it == symbols.vend()) {
                throw invalid_grammar_exception("Undefined grammar symbol: "_u8 + tmp.name);
            }

            match(type::T_IDENTIFIER);
            match(type::T_COLON);

            productions(ruleset.ruleset[it - symbols.vbegin()]);

            match(type::T_SEMICOLON);
        }

        void productions(nonterminal& rule) {
            prod(rule);

            if (lookahead->type == type::T_OR) {
                match(type::T_OR);
                productions(rule);
            }
        }

        void prod(nonterminal& rule) {
            rule.push_back({});
            production& rule_production = rule.back();

            while (lookahead->type != type::T_OR && lookahead->type != type::T_SEMICOLON) {
                grammar_symbol symbol;

                switch (lookahead->type) {
                    case type::T_IDENTIFIER: {
                        auto* token = check<lexer::identifier_token>();

                        symbol.type = grammar_symbol::symbol_type::NON_TERMINAL;
                        symbol.name = std::move(token->name);
                        break;
                    }
                    case type::T_TERMINAL: {
                        auto* token = check<lexer::terminal_token>();

                        symbol.type = grammar_symbol::symbol_type::TERMINAL;
                        symbol.name = std::move(token->name);
                        break;
                    }
                    case type::T_CODE_BLOCK:
                        // TODO: creation of code blocks, and probably middle-rule productions
                        break;
                    default:
                        throw syntax_exception("Invalid production body"_u8);
                }

                match(lookahead->type);

                auto it = symbols.find(symbol);

                if (it == symbols.vend()) {
                    throw invalid_grammar_exception("Undefined grammar symbol: "_u8 + symbol.name);
                }

                rule_production.first.push_back(it - symbols.vbegin());
            }
        }
    };

}

#endif //ALIEN_PARSER_CONFIG_RULES_PARSER_H