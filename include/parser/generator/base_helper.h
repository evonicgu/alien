#ifndef ALIEN_BASE_HELPER_H
#define ALIEN_BASE_HELPER_H

#include <optional>
#include <vector>
#include <unordered_set>

#include "parser/config/rules/rules.h"

namespace alien::parser::generator {

    using symbol_props = std::vector<std::unordered_set<std::ptrdiff_t>>;

    class base_helper {
    protected:
        symbol_props& first;
        const rules::rules& rules;

    public:
        base_helper(symbol_props& first, const rules::rules& rules)
            : first(first),
              rules(rules) {}

        const std::unordered_set<std::ptrdiff_t>& get_first(std::size_t symbol);

        std::unordered_set<std::ptrdiff_t> get_first(
            const std::vector<rules::grammar_symbol>& str,
            std::size_t start_index = 0,
            std::optional<rules::grammar_symbol> ending = std::nullopt);
    };

}

#endif //ALIEN_BASE_HELPER_H