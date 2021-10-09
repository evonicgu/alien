#ifndef ALIEN_QUANTIFIER_H
#define ALIEN_QUANTIFIER_H

#include <memory>
#include "ast.h"

namespace alien::lexer::regex::quantifier {

    using namespace parser::ast;

    struct quantifier {
        virtual node_ptr traverse(const node_ptr& tree) = 0;

        virtual ~quantifier() = default;
    };

    struct star_quantifier : public quantifier {
        node_ptr traverse(const node_ptr& tree) override {
            return std::make_shared<star_node>(tree);
        }
    };

    struct plus_quantifier : public quantifier {
        node_ptr traverse(const node_ptr& tree) override {
            return std::make_shared<concat_node>(tree, std::make_shared<star_node>(tree));
        }
    };

    struct question_quantifier : public quantifier {
        node_ptr traverse(const node_ptr& tree) override {
            return std::make_shared<or_node>(tree, std::make_shared<leaf>(-1));
        }
    };

    struct range_quantifier : public quantifier {
        unsigned int start, end;

        range_quantifier(unsigned int start, unsigned int end) : start(start), end(end) {}

        node_ptr traverse(const node_ptr& tree) override {
            node_ptr modified = tree;

            for (int i = 0; i < start - 1; ++i) {
                modified = std::make_shared<concat_node>(modified, tree);
            }

            for (int i = 0; i < end - start; ++i) {
                modified = std::make_shared<concat_node>(modified,
                                                         std::make_shared<or_node>(std::make_shared<leaf>(-1), tree));
            }

            return modified;
        }
    };

}

#endif //ALIEN_QUANTIFIER_H