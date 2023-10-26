#include "parser/config/rules/parser.h"

namespace alien::parser::rules {

    void parser::parse() {
        using namespace util::literals;

        while (lookahead->type != type::T_END) {
            if (lookahead->type == type::T_IDENTIFIER) {
                rule();
            } else {
                throw std::runtime_error("Expected a rule declaration at " + (std::string) lookahead->start);
            }
        }
    }

    void parser::rule() {
        using namespace util::literals;

        auto* token = check<identifier_token>();

        auto it = check_non_terminal(std::move(token->name));

        if (it == alphabet.non_terminals.vend()) {
            while (lookahead->type != token_type::T_SEMICOLON) {
                match(lookahead->type);
            }

            match(token_type::T_SEMICOLON);
            return;
        }

        std::ptrdiff_t index = it - alphabet.non_terminals.vbegin();

        if (!selected_first) {
            select_first(index);
        }

        match(type::T_IDENTIFIER);
        match(type::T_COLON);

        parse_productions(index);
        match(type::T_SEMICOLON);
    }

    void parser::parse_productions(std::size_t index) {
        prod(index);

        while (lookahead->type == type::T_OR) {
            match(type::T_OR);
            prod(index);
        }
    }

    bool parser::check_lookahead(parser::type t) {
        return t != type::T_OR && t != type::T_SEMICOLON && t != type::T_ACTION;
    }

    void parser::prod(std::size_t index) {
        using namespace util::literals;

        production& current_prod = ruleset.ruleset[index].emplace_back();

        while (check_lookahead(lookahead->type)) {
            grammar_symbol symbol{};

            switch (lookahead->type) {
                case type::T_IDENTIFIER: {
                    auto* token = check<identifier_token>();

                    auto it = check_non_terminal(std::move(token->name));
                    match(type::T_IDENTIFIER);

                    if (it == alphabet.non_terminals.vend()) {
                        continue;
                    }

                    symbol.type = symbol_type::NON_TERMINAL;
                    symbol.index = it - alphabet.non_terminals.vbegin();

                    break;
                }
                case type::T_TERMINAL: {
                    auto* token = check<terminal_token>();

                    auto it = check_terminal(std::move(token->name));
                    match(type::T_TERMINAL);

                    if (it == alphabet.terminals.vend()) {
                        continue;
                    }

                    symbol.type = symbol_type::TERMINAL;
                    symbol.index = it - alphabet.terminals.vbegin();

                    if (!current_prod.explicit_precedence) {
                        current_prod.prec = it->prec;
                        current_prod.assoc = it->assoc;
                    }

                    break;
                }
                case type::T_ERROR: {
                    match(type::T_ERROR);

                    symbol.type = symbol_type::TERMINAL;
                    symbol.index = 0;

                    break;
                }
                case type::T_PREC: {
                    match(type::T_PREC);

                    if (lookahead->type != type::T_IDENTIFIER) {
                        err.push_back(
                                "Expected identifier after '%prec' at "_u8 + (util::u8string) lookahead->start
                        );
                        continue;
                    }

                    auto* token = check<identifier_token>();

                    auto it = check_terminal(std::move(token->name));

                    match(type::T_IDENTIFIER);

                    if (current_prod.explicit_precedence) {
                        throw std::runtime_error("Explicit precedence used more than once");
                    }

                    current_prod.explicit_precedence = true;
                    current_prod.prec = it->prec;
                    current_prod.assoc = it->assoc;

                    continue;
                }
                case type::T_MIDRULE_ACTION: {
                    auto* token = check<midrule_action_token>();

                    util::u8string name = util::ascii_to_u8string("@er$");
                    name.push_back(auxiliary_rules++);

                    symbol.type = symbol_type::NON_TERMINAL;
                    symbol.index = alphabet.non_terminals.size();

                    alphabet.non_terminals.push_back({std::move(name), std::move(token->type), false, true});
                    check_action(token->code, current_prod, alphabet.non_terminals.size() - 1);

                    ruleset.ruleset.emplace_back().emplace_back();
                    ruleset.ruleset.back()[0].action = std::move(token->code);
                    ruleset.ruleset.back()[0].length = current_prod.symbols.size();

                    match(type::T_MIDRULE_ACTION);
                    break;
                }
                default:
                    throw std::runtime_error(
                            "Expected identifier, terminal or midrule action at" + (std::string) lookahead->start
                    );
            }

            current_prod.symbols.push_back(symbol);
        }

        current_prod.length = current_prod.symbols.size();

        if (lookahead->type == type::T_ACTION) {
            auto* token = check<action_token>();

            check_action(token->code, current_prod, index);

            current_prod.has_action = true;
            current_prod.action = std::move(token->code);

            match(type::T_ACTION);
        }
    }

    std::vector<alien::lexer::settings::lexer_symbol>::iterator parser::check_terminal(util::u8string&& name) {
        using namespace util::literals;

        alien::lexer::settings::lexer_symbol symbol{
                std::move(name)
        };

        auto it = alphabet.terminals.find(symbol);

        if (it == alphabet.terminals.vend()) {
            util::pos pos = lookahead->start;

            err.push_back("Unknown terminal '"_u8 + symbol.name + "' at "_u8 + (util::u8string) pos);
        }

        return it;
    }

    std::vector<settings::parser_symbol>::iterator parser::check_non_terminal(util::u8string&& name) {
        using namespace util::literals;

        settings::parser_symbol symbol{
                std::move(name)
        };

        auto it = alphabet.non_terminals.find(symbol);

        if (it == alphabet.non_terminals.vend()) {
            util::pos pos = lookahead->start;

            err.push_back("Unknown non-terminal '"_u8 + symbol.name + "' at "_u8 + (util::u8string) pos);
        }

        return it;
    }

    bool parser::check_action(util::u8string& code, production& prod, std::size_t rule) {
        using namespace util::literals;

        bool in_string = false, in_char = false, in_comment = false, in_multiline_comment = false;
        util::u8char prev = 0, c = code[0];

        util::u8string cleaned;
        cleaned.reserve(code.size() + 1);

        for (std::size_t i = 0; i < code.size(); prev = c, c = code[++i]) {
            if (in_string) {
                if (c == '"') {
                    in_string = false;
                }
                continue;
            }

            if (in_char) {
                if (c == '\'') {
                    in_char = false;
                }
                continue;
            }

            if (in_comment) {
                if (c == '\n') {
                    in_comment = false;
                }
                continue;
            }

            if (in_multiline_comment) {
                if (prev == '*' && c == '/') {
                    in_multiline_comment = false;
                }
                continue;
            }

            if (c == '"') {
                in_string = true;
                continue;
            }

            if (c == '\'') {
                in_char = true;
                continue;
            }

            if (prev == '/' && c == '/') {
                in_comment = true;
                continue;
            }

            if (prev == '/' && c == '*') {
                in_multiline_comment = true;
                continue;
            }

            cleaned.push_back(c);
        }

        cleaned.push_back(0);

        bool state = false;
        std::size_t number = 0;

        for (std::size_t i = 0; i < cleaned.size(); ++i) {
            if (state) {
                if (isdigit(cleaned[i])) {
                    number = number * 10 + cleaned[i] - '0';
                    continue;
                }

                state = false;

                if (number >= prod.symbols.size()) {
                    err.push_back("Symbol index out of range in action at "_u8 + (util::u8string) lookahead->start);
                    continue;
                }

                if (prod.symbols[number].type == symbol_type::TERMINAL) {
                    continue;
                }

                if (alphabet.non_terminals[prod.symbols[number].index].type == settings::void_type) {
                    err.push_back("Illegal symbol used in action at "_u8 + (util::u8string) lookahead->start);
                }
            } else if (cleaned[i] == '$' && isdigit(cleaned[i + 1])) {
                number = 0;
                state = true;
            } else if (cleaned[i] == '$' && cleaned[i + 1] == '$') {
                if (alphabet.non_terminals[rule].type == settings::void_type) {
                    err.push_back("Cannot use $$ if rule type is void"_u8);
                }

                ++i;
            }
        }
        return true;
    }

    void parser::error(token_type expected, token_type got) {
        using namespace util::literals;

        switch (expected) {
            case type::T_COLON:
                err.push_back("Expected a colon after rule name "_u8 + (util::u8string) lookahead->start);
                break;
            case type::T_SEMICOLON:
                err.push_back(
                        "Expected a semicolon after rule declaration at"_u8 + (util::u8string) lookahead->start
                );
                break;
            default:
                throw std::runtime_error("Unexpected token at " + (std::string) lookahead->start);
        }
    }

    void parser::select_first(std::ptrdiff_t index) {
        ruleset.ruleset[0].emplace_back();
        ruleset.ruleset[0][0].symbols.push_back({symbol_type::NON_TERMINAL, index});
        ruleset.start = index;
        selected_first = true;
    }

}