#ifndef ALIEN_LEXER_CONFIG_RULES_PARSER_H
#define ALIEN_LEXER_CONFIG_RULES_PARSER_H

#include "generalized/generalized_parser.h"
#include "rules.h"
#include "rules_lexer.h"
#include "util/u8string.h"

namespace alien::lexer::config::rules::parser {

    using base_parser = generalized::generalized_parser<lexer::token_type, lexer::lexer>;

    using namespace util::literals;

    class parser : base_parser {
        rules& ruleset;

        unsigned int current_context = 0;
        int rule_number = 0;

        void init() {
            ruleset.context_mapping.insert({"initial"_u8, 0});
            ruleset.ruleset.resize(1);
        }
    public:
        parser(rules& ruleset, const lex& l) : ruleset(ruleset), base_parser(l) {
            init();
        }

        parser(rules& ruleset, lex&& l) : ruleset(ruleset), base_parser(std::move(l)) {
            init();
        }

        void parse() override {
            switch (lookahead->type) {
                case type::T_REGULAR_EXPRESSION:
                    rule();

                    if (lookahead->type != type::T_END) {
                        parse();
                    }

                    break;
                default:
                    throw syntax_exception("Rule must start with a regular expression"_u8);
            }
        }

    private:
        void rule() {
            auto* token = check<lexer::regex_token>("Expected token to be a regex token instance"_u8);
            action* action_ptr;
            bool changed_context = false;

            if (token->regex == "<<<EOF>>>"_u8) {
                action_ptr = &ruleset.eof_action;
            } else if (is_valid_context(token->regex)) {
                util::u8string context = token->regex.substr(1, token->regex.size() - 2);

                if (ruleset.context_mapping.find(context) != ruleset.context_mapping.end()) {
                    current_context = 0;
                } else {
                    ruleset.context_mapping.insert({std::move(context), ruleset.context_mapping.size()});
                    current_context = ruleset.context_mapping.size() - 1;
                    ruleset.ruleset.emplace_back();
                }

                changed_context = true;
            } else {
                ruleset.ruleset[current_context].push_back({std::move(token->regex), {}, rule_number++});
                action_ptr = &ruleset.ruleset[current_context].back().act;
            }

            match(type::T_REGULAR_EXPRESSION);

            match(type::T_COLON);

            if (changed_context) {
                return;
            }

            auto* action = check<lexer::action_token>("Expected token to be action token instance"_u8);

            action_ptr->code = std::move(action->code);

            if (action->tr.returned) {
                action_ptr->trailing_return = std::move(action->tr.name);
            }

            match(type::T_CODE_BLOCK);

            match(type::T_SEMICOLON);
        }

        static bool is_valid_context(const util::u8string& context) {
            if (context[0] != '<' || context.back() != '>') {
                return false;
            }

            for (unsigned int i = 1; i < context.size() - 1; ++i) {
                if (!isalpha(context[i])) {
                    return false;
                }
            }

            return true;
        }
    };

}

#endif //ALIEN_LEXER_CONFIG_RULES_PARSER_H