#ifndef ALIEN_REGEX_QUANTIFIER_H
#define ALIEN_REGEX_QUANTIFIER_H

#include <memory>

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
        std::size_t start, end;

        range_quantifier(std::size_t start, std::size_t end)
            : start(start),
              end(end) {}

        ast::node_ptr traverse(const ast::node_ptr& tree) override;
    };

}

#endif //ALIEN_REGEX_QUANTIFIER_H