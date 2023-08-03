#ifndef ALIEN_LALR_H_VEC_OLD
#define ALIEN_LALR_H_VEC_OLD

#include <set>
#include <tuple>

#include "clr.h"
#include "base_table_generator.h"
#include "parser/config/rules/rules.h"
#include "slr.h"
#include "util/hash_vecset.h"

namespace alien::parser::generator {

    using item_iterator = std::vector<slr::item>::const_iterator;

    using comparator = util::comparators::iterator_elem_addr_less<item_iterator>;

    using lookahead_table = std::map<item_iterator, std::unordered_set<std::ptrdiff_t>, comparator>;
    using propagation_table = std::map<item_iterator, std::set<item_iterator, comparator>, comparator>;

    class lalr_generator : public base_table_generator {
        lookahead_table lookahead;
        propagation_table propagation;

        slr_helper simple_helper;
        clr_helper canonical_helper;

    public:
        lalr_generator(alphabet::alphabet& alphabet, rules::rules& rules)
            : base_table_generator(alphabet, rules),
              simple_helper(first, rules, alphabet),
              canonical_helper(first, rules) {}

        parsing_table generate_table() override;

    private:
        void generate_lookahead(const util::vecset<std::vector<slr::item>>& slr_items);

        util::vecset<std::vector<clr::item>> build_lalr_items(const util::vecset<std::vector<slr::item>>& slr_items);
    };

}

#endif //ALIEN_LALR_H_VEC_OLD