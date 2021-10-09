#ifndef ALIEN_REGEX_AST_H
#define ALIEN_REGEX_AST_H

#include <memory>

namespace alien::lexer::regex::parser::ast {


    struct node {
        enum class node_type {
            CONCAT,
            STAR,
            OR,
            LEAF
        } type;

        virtual ~node() = default;
    };

    using node_ptr = std::shared_ptr<node>;

    struct op_node : public node {
        node_ptr first, second;

        op_node(const node_ptr& first, const node_ptr& second) : first(first), second(second) {}
    };

    struct concat_node : public op_node {
        concat_node(const node_ptr& first, const node_ptr& second) : op_node(first, second) {
            type = node_type::CONCAT;
        }
    };

    struct or_node : public op_node {
        or_node(const node_ptr& first, const node_ptr& second) : op_node(first, second) {
            type = node_type::OR;
        }
    };

    struct star_node : public node {
        node_ptr first;

        explicit star_node(const node_ptr& first) : first(first) {
            type = node_type::STAR;
        }
    };

    struct leaf : public node {
        char symbol;

        explicit leaf(char symbol) : symbol(symbol) {
            type = node_type::LEAF;
        }
    };
}

#endif //ALIEN_REGEX_AST_H