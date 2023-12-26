#ifndef ALIEN_LEXER_RULES_PARSER_H
#define ALIEN_LEXER_RULES_PARSER_H

#include <list>

#include "alphabet.h"
#include "rules.h"
#include "token.h"
#include "util/parser.h"
#include "util/u8string.h"

namespace alien::lexer::rules {

    const util::u8string ctx_initial = util::ascii_to_u8string("initial");

    class parser : public util::parser<token_type> {
        std::size_t current_context = 0;
        std::ptrdiff_t rule_number = 0;
        rules ruleset;
        alphabet::alphabet& alphabet;

        bool next_rule_no_utf8 = false;

    public:
        parser(lexer_t& l, std::list<util::u8string>& err, alphabet::alphabet& alphabet)
            : alphabet(alphabet),
              util::parser<token_type>(l, err) {
            using namespace util::literals;
            ruleset.ctx[ctx_initial] = 0;
            ruleset.ruleset.resize(1);
        }

        void parse() override;

        rules&& get_rules() {
            return std::move(ruleset);
        }

    private:
        void rule();

        void set_action(action& act);

        void switch_context();

        void error(token_type expected, token_type got) override;
    };

}

#endif //ALIEN_LEXER_RULES_PARSER_H