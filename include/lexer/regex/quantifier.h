#ifndef ALIEN_REGEX_QUANTIFIER_H
#define ALIEN_REGEX_QUANTIFIER_H

#include <optional>

#include "ast.h"

namespace alien::lexer::regex::quantifier {

    struct quantifier {
        virtual ast::node_ptr traverse(const ast::node_ptr& tree) = 0;

        virtual ~quantifier() = default;
    };

    struct star_quantifier : public quantifier {
        ast::node_ptr traverse(const ast::node_ptr& tree) override;
    };

    struct plus_quantifier : public quantifier {
        ast::node_ptr traverse(const ast::node_ptr& tree) override;
    };

    struct question_quantifier : public quantifier {
        ast::node_ptr traverse(const ast::node_ptr& tree) override;
    };

    struct range_quantifier : public quantifier {
        std::optional<std::size_t> start_opt, end_opt;

        range_quantifier(std::optional<std::size_t> start, std::optional<std::size_t> end)
            : start_opt(start),
              end_opt(end) {}

        ast::node_ptr traverse(const ast::node_ptr& tree) override;
    };

}

#endif //ALIEN_REGEX_QUANTIFIER_H