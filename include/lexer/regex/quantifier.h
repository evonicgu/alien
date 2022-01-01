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
        ast::node_ptr traverse(const ast::node_ptr& tree) override {
            return std::make_shared<ast::star_node>(tree);
        }
    };

    struct plus_quantifier : public quantifier {
        ast::node_ptr traverse(const ast::node_ptr& tree) override {
            return std::make_shared<ast::concat_node>(tree, std::make_shared<ast::star_node>(tree));
        }
    };

    struct question_quantifier : public quantifier {
        ast::node_ptr traverse(const ast::node_ptr& tree) override {
            return std::make_shared<ast::or_node>(tree, std::make_shared<ast::leaf>(-1));
        }
    };

    struct range_quantifier : public quantifier {
        std::size_t start, end;

        range_quantifier(std::size_t start, std::size_t end)
            : start(start),
              end(end) {}

        ast::node_ptr traverse(const ast::node_ptr& tree) override {
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
    };

}

#endif //ALIEN_REGEX_QUANTIFIER_H