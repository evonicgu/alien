#include "lexer/regex/quantifier.h"

namespace alien::lexer::regex {

    ast::node_ptr quantifier::range_quantifier::traverse(const ast::node_ptr& tree) {
        ast::node_ptr modified = nullptr;

        if (start == 0 && end == 0) {
            return std::make_shared<ast::leaf>(-1);
        }

        for (std::size_t i = 0; i < start; ++i) {
            if (modified == nullptr) {
                modified = tree;
            } else {
                modified = std::make_shared<ast::concat_node>(modified, tree);
            }
        }

        for (std::size_t i = 0; i < end - start; ++i) {
            ast::node_ptr tmp = std::make_shared<ast::or_node>(
                    std::make_shared<ast::leaf>(-1),
                    tree
            );

            if (modified == nullptr) {
                modified = tmp;
            } else {
                modified = std::make_shared<ast::concat_node>(modified, tmp);
            }
        }

        return modified;
    }

    ast::node_ptr quantifier::question_quantifier::traverse(const ast::node_ptr& tree) {
        return std::make_shared<ast::or_node>(tree, std::make_shared<ast::leaf>(-1));
    }

    ast::node_ptr quantifier::plus_quantifier::traverse(const ast::node_ptr& tree) {
        return std::make_shared<ast::concat_node>(tree, std::make_shared<ast::star_node>(tree));
    }

    ast::node_ptr quantifier::star_quantifier::traverse(const ast::node_ptr& tree) {
        return std::make_shared<ast::star_node>(tree);
    }
}