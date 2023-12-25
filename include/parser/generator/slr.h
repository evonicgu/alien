#ifndef ALIEN_SLR_H_VEC_OLD
#define ALIEN_SLR_H_VEC_OLD

#include <map>
#include <set>
#include <tuple>
#include <iostream>

#include "base_table_generator.h"
#include "parser/config/rules/rules.h"
#include "util/vecset.h"

namespace alien::parser::generator {

    namespace slr {

        using item = std::tuple<std::size_t, std::size_t, std::size_t>;

    }

    class slr_helper : public base_helper {
        std::map<std::vector<slr::item>, std::vector<slr::item>> slr_closure_cache;
        const alphabet::alphabet& alphabet;
        std::vector<bool> non_terminal_productions;

    public:
        slr_helper(symbol_props& first,
                   const rules::rules& rules,
                   const alphabet::alphabet& alphabet)
            : base_helper(first, rules),
              alphabet(alphabet) {
            non_terminal_productions.resize(alphabet.non_terminals.size());
        }

        util::vecset<std::vector<slr::item>> generate_slr_items();

        std::vector<slr::item> slr_closure(const std::vector<slr::item>& items);

        std::vector<slr::item> slr_move(const std::vector<slr::item>& items, const rules::grammar_symbol& symbol) const;
    };

    class slr_generator : public base_table_generator {
        slr_helper helper;

    public:
        slr_generator(alphabet::alphabet& alphabet, rules::rules& rules)
            : base_table_generator(alphabet, rules),
              helper(first, rules, alphabet) {}

        parsing_table generate_table() override;

        void build_follow();
    };

}

#endif //ALIEN_SLR_H_VEC_OLD