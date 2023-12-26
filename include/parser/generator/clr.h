#ifndef ALIEN_CLR_H
#define ALIEN_CLR_H

#include <vector>
#include <tuple>

#include "alphabet.h"
#include "base_table_generator.h"
#include "parser/config/rules/rules.h"

namespace alien::parser::generator {

    namespace clr {

        using item = std::tuple<std::size_t, std::size_t, std::size_t, std::ptrdiff_t>;

    }

    class clr_helper : public base_helper {
        std::vector<bool> non_terminal_looahead_mapping;
        const alphabet::alphabet& alphabet;

    public:
        clr_helper(symbol_props& first, const rules::rules& rules, const alphabet::alphabet& alphabet)
            : base_helper(first, rules),
              alphabet(alphabet) {
            non_terminal_looahead_mapping.resize(alphabet.non_terminals.size() * (alphabet.terminals.size() + 3));
        }

        std::vector<clr::item> clr_closure(const std::vector<clr::item>& items);

        std::vector<clr::item> clr_move(const std::vector<clr::item>& items, const rules::grammar_symbol& symbol) const;
    };

    class clr_generator : public base_table_generator {
        clr_helper helper;

    public:
        clr_generator(alphabet::alphabet& alphabet, rules::rules& rules)
            : base_table_generator(alphabet, rules),
              helper(first, rules, alphabet) {}

        parsing_table generate_table() override;

        std::size_t clr_transition(util::vecset<std::vector<clr::item>>& states, std::vector<clr::item>& next);
    };

}

#endif //ALIEN_CLR_H