#ifndef ALIEN_REGEX_AST_H
#define ALIEN_REGEX_AST_H

#include <memory>
#include <unordered_set>

#include "util/u8string.h"

namespace alien::lexer::regex::ast {

    struct node {
        enum class node_type {
            CONCAT,
            STAR,
            OR,
            LEAF,
            NEGATIVE_CLASS
        } type;

        virtual ~node() = default;
    };

    using node_ptr = std::shared_ptr<node>;

    struct op_node : public node {
        node_ptr first, second;

        op_node(const node_ptr& first, const node_ptr& second)
            : first(first),
              second(second) {}
    };

    struct concat_node : public op_node {
        concat_node(const node_ptr& first, const node_ptr& second)
            : op_node(first, second) {
            type = node_type::CONCAT;
        }
    };

    struct or_node : public op_node {
        or_node(const node_ptr& first, const node_ptr& second)
            : op_node(first, second) {
            type = node_type::OR;
        }
    };

    struct star_node : public node {
        node_ptr first;

        explicit star_node(const node_ptr& first)
            : first(first) {
            type = node_type::STAR;
        }
    };

    struct leaf : public node {
         util::u8char symbol;

        explicit leaf(util::u8char symbol)
            : symbol(symbol) {
            type = node_type::LEAF;
        }
    };

    struct negative_class : public node {
        std::unordered_set<util::u8char> negative_chars;

        explicit negative_class(std::unordered_set<util::u8char>&& negative_chars)
            : negative_chars(std::move(negative_chars)) {
            type = node_type::NEGATIVE_CLASS;
        }
    };
}

#endif //ALIEN_REGEX_AST_H