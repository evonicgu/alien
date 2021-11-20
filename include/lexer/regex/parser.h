#ifndef ALIEN_REGEX_PARSER_H
#define ALIEN_REGEX_PARSER_H

#include <algorithm>
#include <set>
#include <memory>
#include "ast.h"
#include "generalized/generalized_parser.h"
#include "lexer.h"
#include "quantifier.h"
#include "ranges_classes.h"
#include "util/u8string.h"

namespace alien::lexer::regex::parser {

    using node_ptr = std::shared_ptr<ast::node>;
    using base_parser = generalized::generalized_parser<lexer::token_type, lexer::lexer>;

    using namespace util::literals;

    static constexpr const char range_exception_str[] = "Incorrect character range. ";

    class parser : public base_parser {
        unsigned int fold_level = 0;

        node_ptr ast;

    public:
        explicit parser(const lex& l) : base_parser(l) {}

        explicit parser(lex&& l) : base_parser(std::move(l)) {}

        void parse() override {
            ast = regex();
        }

        node_ptr get_ast() {
            return ast;
        }

        using range_exception = generalized::generalized_exception<range_exception_str>;

    private:
        node_ptr regex() {
            node_ptr tree = factors();

            if (lookahead->type == type::T_OR) {
                match(type::T_OR);
                tree = std::make_shared<ast::or_node>(tree, regex());
            }

            return tree;
        }

        node_ptr factors() {
            switch (lookahead->type) {
                case type::T_OR:
                case type::T_STAR:
                case type::T_PLUS:
                case type::T_BRACE_OPEN:
                case type::T_BRACE_CLOSE:
                case type::T_QUESTION_MARK:
                    throw syntax_exception("The preceding token is not quantifiable"_u8);
                case type::T_SQUARE_BRACKET_CLOSE:
                case type::T_PARENTHESIS_CLOSE:
                    throw syntax_exception("Unmatched parenthesis"_u8);
                case type::T_NEGATIVE_CLASS:
                    throw syntax_exception("Unexpected class negation character"_u8);
                case type::T_END:
                    throw syntax_exception("Unexpected end of file"_u8);
                default:
                    node_ptr first = factor();

                    if (lookahead->type == type::T_END || lookahead->type == type::T_OR) {
                        return first;
                    }

                    if (lookahead->type == type::T_PARENTHESIS_CLOSE) {
                        if (fold_level == 0) {
                            throw syntax_exception("Unmatched parenthesis"_u8);
                        }

                        --fold_level;
                        return first;
                    }

                    return std::make_shared<ast::concat_node>(first, factors());
            }
        }

        node_ptr factor() {
            node_ptr tree = base();

            auto res = quantifier();

            if (res.first) {
                tree = res.second->traverse(tree);
                delete res.second;
            }

            return tree;
        }

        node_ptr base() {
            node_ptr tree;

            switch (lookahead->type) {
                case type::T_PARENTHESIS_OPEN:
                    ++fold_level;
                    match(type::T_PARENTHESIS_OPEN);

                    if (lookahead->type == type::T_PARENTHESIS_CLOSE) {
                        throw syntax_exception("Empty group"_u8);
                    }

                    tree = regex();
                    match(type::T_PARENTHESIS_CLOSE);
                    break;
                case type::T_SQUARE_BRACKET_OPEN:
                    tree = char_class();
                    break;
                case type::T_SYMBOL:
                case type::T_NUMBER:
                case type::T_COMMA:
                case type::T_HYPHEN:
                    tree = char_normal();
                    break;
                default:
                    tree = shortcut();
                    break;
            }
            
            return tree;
        }
        
        node_ptr char_normal() {
            node_ptr tree;

            switch (lookahead->type) {
                case type::T_SYMBOL:
                    tree = std::make_shared<ast::leaf>(get_char());
                    match(type::T_SYMBOL);
                    break;
                case type::T_NUMBER:
                    tree = std::make_shared<ast::leaf>(get_digit() + '0');
                    match(type::T_NUMBER);
                    break;
                case type::T_COMMA:
                    tree = std::make_shared<ast::leaf>(',');
                    match(type::T_COMMA);
                    break;
                case type::T_HYPHEN:
                    tree = std::make_shared<ast::leaf>('-');
                    match(type::T_HYPHEN);
                    break;
            }

            return tree;
        }

        node_ptr char_class() {
            match(type::T_SQUARE_BRACKET_OPEN);
            bool negative = false;

            if (lookahead->type == type::T_NEGATIVE_CLASS) {
                match(type::T_NEGATIVE_CLASS);
                negative = true;
            }

            auto chars = chars_or_ranges();

            match(type::T_SQUARE_BRACKET_CLOSE);

            if (negative) {
                return std::make_shared<ast::negative_class>(std::move(chars));
            }

            return make_ast_from_set(chars);
        }

        std::set<util::u8char> chars_or_ranges() {
            std::set<util::u8char> chars;

            return chars_or_ranges(chars);
        }

        std::set<util::u8char> chars_or_ranges(std::set<util::u8char>& chars) {
            switch (lookahead->type) {
                case type::T_SYMBOL:
                case type::T_NUMBER:
                case type::T_COMMA:
                    char_or_range(chars);
                    chars_or_ranges(chars);
                case type::T_SQUARE_BRACKET_CLOSE:
                    return chars;
                default:
                    throw syntax_exception("Unexpected token inside a class"_u8);
            }
        }

        void char_or_range(std::set<util::u8char>& chars) {
            util::u8char start = char_in_class();

            if (lookahead->type == type::T_HYPHEN) {
                match(type::T_HYPHEN);
                util::u8char end = char_in_class();

                if (end < start) {
                    throw range_exception("End of the range is lower than the start"_u8);
                }

                for (util::u8char i = start; i <= end; ++i) {
                    chars.insert(i);
                }

                return;
            }

            chars.insert(start);
        }

        util::u8char char_in_class() {
            util::u8char c;

            switch (lookahead->type) {
                case type::T_SYMBOL:
                    c = get_char();
                    match(type::T_SYMBOL);
                    break;
                case type::T_NUMBER:
                    c = get_digit() + '0';
                    match(type::T_NUMBER);
                    break;
                case type::T_COMMA:
                    match(type::T_COMMA);
                    c = ',';
                    break;
                default:
                    throw syntax_exception("Unexpected character inside a class"_u8);
            }

            return c;
        }

        node_ptr shortcut() {
            node_ptr tree;

            switch (lookahead->type) {
                case type::T_CLASS: {
                    auto* class_token = check<lexer::symbol_class_token>(
                            "Expected token to be a symbol class token instance"_u8);

                    tree = make_ast_from_set(ranges::get_classes_by_name(class_token->class_name));
                    break;
                }
                case type::T_DOT:
                    tree = make_ast_from_set(ranges::full);
                    break;
                case type::T_VALID_SEQUENCE:
                    tree = make_ast_from_set(ranges::valid_sequence);
                    break;
                case type::T_UNICODE_NEWLINE:
                    tree = make_ast_from_set(ranges::unicode_newline);
                    break;
                case type::T_SPACE:
                    tree = make_ast_from_set(ranges::space);
                    break;
                case type::T_NON_SPACE:
                    tree = make_ast_from_set(ranges::non_space);
                    break;
                case type::T_HORIZONTAL_SPACE:
                    tree = make_ast_from_set(ranges::h_space);
                    break;
                case type::T_NON_HORIZONTAL_SPACE:
                    tree = make_ast_from_set(ranges::non_h_space);
                    break;
                case type::T_DIGIT:
                    tree = std::make_shared<ast::leaf>(-16);
                    break;
                case type::T_NON_DIGIT:
                    tree = make_ast_from_set(ranges::non_digit);
                    break;
                case type::T_NON_NEWLINE:
                    tree = make_ast_from_set(ranges::non_newline);
                    break;
                case type::T_VERTICAL_SPACE:
                    tree = make_ast_from_set(ranges::v_space);
                    break;
                case type::T_NON_VERTICAL_SPACE:
                    tree = make_ast_from_set(ranges::non_v_space);
                    break;
                case type::T_WORD_CHAR:
                    tree = make_ast_from_set(ranges::word_char);
                    break;
                case type::T_NON_WORD_CHAR:
                    tree = make_ast_from_set(ranges::non_word_char);
                    break;
                default:
                    throw syntax_exception("Expected token to be shortcut token"_u8);
            }

            match(lookahead->type);

            return tree;
        }

        std::pair<bool, quantifier::quantifier*> quantifier() {
            switch (lookahead->type) {
                case type::T_STAR:
                    match(type::T_STAR);
                    return {true, new quantifier::star_quantifier()};
                case type::T_PLUS:
                    match(type::T_PLUS);
                    return {true, new quantifier::plus_quantifier()};
                case type::T_QUESTION_MARK:
                    match(type::T_QUESTION_MARK);
                    return {true, new quantifier::question_quantifier()};
                case type::T_BRACE_OPEN: {
                    unsigned int start, end;

                    match(type::T_BRACE_OPEN);
                    start = numbers();
                    match(type::T_COMMA);
                    end = numbers();
                    match(type::T_BRACE_CLOSE);

                    return {true, new quantifier::range_quantifier(start, end)};
                }
                default:
                    return {false, nullptr};
            }
        }

        unsigned int numbers() {
            if (lookahead->type == type::T_NUMBER) {
                unsigned int prev = get_digit();
                match(type::T_NUMBER);

                return numbers(prev);
            }

            throw syntax_exception("Expected token to be a number"_u8);
        }

        unsigned int numbers(unsigned int prev) {
            if (lookahead->type == type::T_NUMBER) {
                prev = prev * 10 + get_digit();
                match(type::T_NUMBER);

                return numbers(prev);
            }

            return prev;
        }

        util::u8char get_char() {
            auto* symbol = check<lexer::symbol_token>("Expected token to be a symbol token instance"_u8);

            return symbol->symbol;
        }

        uint8_t get_digit() {
            auto* digit = check<lexer::number_token>("Expected token to be a number token instance"_u8);

            return digit->number;
        }

        static node_ptr make_ast_from_set(const std::set<util::u8char>& set) {
            if (set.empty()) {
                return std::make_shared<ast::leaf>(-1);
            }

            if (set.size() == 1) {
                return std::make_shared<ast::leaf>(*set.begin());
            }

            node_ptr tree = std::make_shared<ast::leaf>(*set.rbegin());

            for (auto c = ++set.rbegin(); c != set.rend(); ++c) {
                tree = std::make_shared<ast::or_node>(std::make_shared<ast::leaf>(*c), tree);
            }

            return tree;
        }
    };

}

#endif //ALIEN_REGEX_PARSER_H