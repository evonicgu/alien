#ifndef ALIEN_PARSER_RULES_PARSER_H
#define ALIEN_PARSER_RULES_PARSER_H

#include <list>
#include <vector>

#include "alphabet.h"
#include "rules.h"
#include "token.h"
#include "util/parser.h"
#include "lexer/config/settings/settings.h"
#include "parser/config/settings/settings.h"
#include "util/u8string.h"

namespace alien::parser::rules {

    class parser : util::parser<token_type> {
        rules ruleset;
        alphabet::alphabet& alphabet;
        bool selected_first = false;

        std::vector<bool> has_return;

        int auxiliary_rules = 0;

    public:
        parser(lexer_t& l, std::list<util::u8string>& err, alphabet::alphabet& alphabet, const util::u8string& first)
            : alphabet(alphabet),
              util::parser<token_type>(l, err) {
            using namespace util::literals;

            ruleset.ruleset.resize(alphabet.non_terminals.size());

            has_return.resize(alphabet.non_terminals.size(), true);

            auto it = alphabet.non_terminals.find(first);

            if (it != alphabet.non_terminals.vend()) {
                select_first(it - alphabet.non_terminals.vbegin());
            }

            if (!first.empty() && !selected_first) {
                err.push_back("Start symbol not found: "_u8 + first);
            }
        }

        void parse() override;

        rules&& get_rules() {
            return std::move(ruleset);
        }

    private:
        void rule();

        void parse_productions(std::size_t index);

        static bool check_lookahead(type t);

        void prod(std::size_t index);

        std::vector<alien::lexer::settings::lexer_symbol>::iterator check_terminal(util::u8string&& name);

        std::vector<settings::parser_symbol>::iterator check_non_terminal(util::u8string&& name);

        bool check_action(util::u8string& code, production& prod, std::size_t rule);

        void error(token_type expected, token_type got) override;

        void select_first(std::ptrdiff_t index);
    };

}

#endif //ALIEN_PARSER_RULES_PARSER_H