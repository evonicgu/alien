#ifndef ALIEN_CLR_H
#define ALIEN_CLR_H

#include <map>
#include <set>
#include <tuple>

#include "base_table_generator.h"
#include "parser/config/rules/rules.h"
#include "util/hash_vecset.h"
#include "boost/container_hash/hash.hpp"

namespace alien::parser::generator {

    namespace clr {

        using item = std::tuple<std::size_t, std::size_t, std::size_t, std::ptrdiff_t>;

    }

    class clr_helper : public base_helper {
        std::map<std::vector<clr::item>, std::vector<clr::item>> cache;

    public:
        clr_helper(symbol_props& first, const rules::rules& rules)
            : base_helper(first, rules) {}

        std::vector<clr::item> clr_closure(const std::vector<clr::item>& items);

        std::vector<clr::item> clr_move(const std::vector<clr::item>& items, const rules::grammar_symbol& symbol);
    };

    class clr_generator : public base_table_generator {
        clr_helper helper;

    public:
        clr_generator(alphabet::alphabet& alphabet, rules::rules& rules)
            : base_table_generator(alphabet, rules),
              helper(first, rules) {}

        parsing_table generate_table() override;

        std::size_t clr_transition(util::vecset<std::vector<clr::item>>& states, std::vector<clr::item>& next);
    };

}

#endif //ALIEN_CLR_H