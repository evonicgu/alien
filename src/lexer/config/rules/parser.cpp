#include "lexer/config/rules/parser.h"

namespace alien::lexer::rules {

    void parser::parse() {
        using namespace util::literals;

        while (lookahead->type != type::T_END) {
            switch (lookahead->type) {
                case type::T_REGEX:
                    rule();
                    break;
                case type::T_EOF_RULE:
                    match(type::T_EOF_RULE);
                    set_action(ruleset.on_eof);
                    break;
                case type::T_CONTEXT:
                    switch_context();
                    break;
                case type::T_NO_UTF8:
                    next_rule_no_utf8 = true;
                    match(type::T_NO_UTF8);
                    break;
                default: {
                    util::pos err_pos = lookahead->start;

                    throw std::runtime_error(
                            "Expected regex or context declaration at " + (std::string) err_pos
                    );
                }
            }
        }

        if (current_context != 0) {
            err.push_back("Unmatched context declaration"_u8);
        }
    }

    void parser::rule() {
        auto* token = check<regex_token>();

        ruleset.ruleset[current_context].push_back({
                                                           next_rule_no_utf8,
                                                           std::move(token->regex),
                                                           {},
                                                           token->start,
                                                           rule_number++
                                                   });

        match(type::T_REGEX);
        set_action(ruleset.ruleset[current_context].back().act);

        next_rule_no_utf8 = false;
    }

    void parser::set_action(action& act) {
        using namespace util::literals;

        match(type::T_COLON);

        if (lookahead->type == type::T_ACTION) {
            auto* token = check<action_token>();
            act.code = std::move(token->code);

            settings::lexer_symbol symbol{std::move(token->symbol)};

            if (!symbol.name.empty()) {
                auto it = alphabet.terminals.find(symbol);

                if (it == alphabet.terminals.vend()) {
                    err.push_back(
                            "Unknown symbol '"_u8 + symbol.name + "' at "_u8 + (util::u8string) lookahead->start
                    );
                } else {
                    act.symbol = std::move(symbol.name);
                }
            }

            match(type::T_ACTION);
        } else {
            util::pos err_pos = lookahead->start;
            err.push_back("Expected an action after rule declaration at"_u8 + (util::u8string) err_pos);
        }

        match(type::T_SEMICOLON);
    }

    void parser::switch_context() {
        auto* token = check<context_token>();
        util::pos start = token->start;
        util::u8string ctx = token->context.substr(1, token->context.size() - 2);

        if (current_context != 0) {
            auto it = ruleset.ctx.find(ctx);

            // end of context declaration
            if (it == ruleset.ctx.end() || it->second != current_context) {
                throw std::runtime_error("Nested context declaration at " + (std::string) start);
            }

            current_context = 0;
        } else {
            // start of context
            auto it = ruleset.ctx.find(ctx);

            if (it == ruleset.ctx.end()) {
                ruleset.ctx[ctx] = current_context = ruleset.ctx.size();
                ruleset.ruleset.emplace_back();
            } else {
                current_context = it->second;
            }
        }

        match(type::T_CONTEXT);
        match(type::T_COLON);
    }

    void parser::error(token_type expected, token_type got) {
        using namespace util::literals;

        if (expected == type::T_SEMICOLON) {
            err.push_back("Expected a semicolon after rule declaration"_u8);
            return;
        }

        throw std::runtime_error("Unexpected token at" + (std::string) lookahead->start);
    }
}