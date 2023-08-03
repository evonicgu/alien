#ifndef ALIEN_LEXER_RULES_TOKEN_H
#define ALIEN_LEXER_RULES_TOKEN_H

#include "util/token.h"
#include "util/u8string.h"

namespace alien::lexer::rules {

    enum class token_type {
        T_SEMICOLON,
        T_COLON,
        T_NO_UTF8,
        T_REGEX,
        T_EOF_RULE,
        T_CONTEXT,
        T_ACTION,
        T_END
    };

    using base = util::token<token_type>;

    struct regex_token : public base {
        util::u8string regex;

        explicit regex_token(util::u8string&& regex, util::pos start, util::pos end)
            : regex(std::move(regex)),
              base(token_type::T_REGEX, start, end) {}
    };

    struct context_token : public base {
        util::u8string context;

        explicit context_token(util::u8string&& context, util::pos start, util::pos end)
            : context(std::move(context)),
              base(token_type::T_CONTEXT, start, end) {}
    };

    struct action_token : public base {
        util::u8string code, symbol;

        explicit action_token(util::u8string&& code, util::pos start, util::pos end)
            : code(std::move(code)),
              base(token_type::T_ACTION, start, end) {}

        explicit action_token(util::u8string&& code, util::u8string&& symbol, util::pos start, util::pos end)
            : code(std::move(code)),
              symbol(std::move(symbol)),
              base(token_type::T_ACTION, start, end) {}
    };

}

#endif //ALIEN_LEXER_RULES_TOKEN_H