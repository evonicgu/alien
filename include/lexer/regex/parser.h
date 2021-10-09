#ifndef ALIEN_REGEX_PARSER_H
#define ALIEN_REGEX_PARSER_H

#include <algorithm>
#include <set>
#include <memory>
#include "ast.h"
#include "generalized/generalized_parser.h"
#include "lexer.h"
#include "ranges.h"
#include "quantifier.h"


namespace alien::lexer::regex::parser {

    using node_ptr = std::shared_ptr<ast::node>;

    using base_parser = generalized::generalized_parser<lexer::token_type, lexer::lexer>;

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
                case type::T_SQUARE_BRACKET_CLOSE:
                case type::T_NEGATIVE_CLASS:
                case type::T_END:
                case type::T_PARENTHESIS_CLOSE: {
                    throw syntax_exception("The preceding token is not quantifiable");
                }
                default:
                    node_ptr first = factor();

                    if (lookahead->type == type::T_END || lookahead->type == type::T_OR) {
                        return first;
                    }

                    if (lookahead->type == type::T_PARENTHESIS_CLOSE) {
                        if (fold_level == 0) {
                            throw syntax_exception("Unmatched parenthesis");
                        }

                        --fold_level;
                        return first;
                    }

                    return std::make_shared<ast::concat_node>(first, factors());
            }
        }

        node_ptr factor() {
            node_ptr tree = base();

            quantifier::quantifier* q = quantifier();

            if (q) {
                tree = q->traverse(tree);
                delete q;
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
                        throw syntax_exception("Empty group");
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
                    tree = std::make_shared<ast::leaf>((char) (get_digit() + '0'));
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
                std::set<char> negative_chars;

                std::set_difference(chars.begin(),  chars.end(), ranges::full.begin(),  ranges::full.end(),
                                    std::inserter(negative_chars, negative_chars.begin()));

                return make_ast_from_set(negative_chars);
            }

            return make_ast_from_set(chars);
        }

        std::set<char> chars_or_ranges() {
            std::set<char> chars;

            return chars_or_ranges(chars);
        }

        std::set<char> chars_or_ranges(std::set<char>& chars) {
            switch (lookahead->type) {
                case type::T_SYMBOL:
                case type::T_NUMBER:
                case type::T_COMMA:
                    char_or_range(chars);
                    chars_or_ranges(chars);
                case type::T_SQUARE_BRACKET_CLOSE:
                    return chars;
                default:
                    throw syntax_exception("Unexpected token inside a class");
            }
        }

        void char_or_range(std::set<char>& chars) {
            char start = char_in_class();

            if (lookahead->type == type::T_HYPHEN) {
                match(type::T_HYPHEN);
                char end = char_in_class();

                if (end < start) {
                    throw range_exception("End of the range is lower than the start");
                }

                for (char i = start; i <= end; ++i) {
                    chars.insert(i);
                }

                return;
            }

            chars.insert(start);
        }

        char char_in_class() {
            char c;

            switch (lookahead->type) {
                case type::T_SYMBOL:
                    c = get_char();
                    match(type::T_SYMBOL);
                    break;
                case type::T_NUMBER:
                    c = (char) (get_digit() + '0');
                    match(type::T_NUMBER);
                    break;
                case type::T_COMMA:
                    match(type::T_COMMA);
                    c = ',';
                    break;
                default:
                    throw syntax_exception("Unexpected character inside a class");
            }

            return c;
        }

        node_ptr shortcut() {
            std::set<char>* range;

            switch (lookahead->type) {
                case type::T_DOT:
                    range = &ranges::full;
                    break;
                case type::T_SPACE:
                    range = &ranges::space;
                    break;
                case type::T_NON_SPACE:
                    range = &ranges::non_space;
                    break;
                case type::T_DIGIT:
                    range = &ranges::digit;
                    break;
                case type::T_NON_DIGIT:
                    range = &ranges::non_digit;
                    break;
                case type::T_NON_NEWLINE:
                    range = &ranges::non_newline;
                    break;
                case type::T_NON_VERTICAL_TABULATION:
                    range = &ranges::non_vertical_tabulation;
                    break;
                case type::T_WORD_CHAR:
                    range = &ranges::word_chars;
                    break;
                case type::T_NON_WORD_CHAR:
                    range = &ranges::non_word_chars;
                    break;
            }

            match(lookahead->type);

            return make_ast_from_set(*range);
        }

        quantifier::quantifier* quantifier() {
            switch (lookahead->type) {
                case type::T_STAR:
                    match(type::T_STAR);
                    return new quantifier::star_quantifier();
                case type::T_PLUS:
                    match(type::T_PLUS);
                    return new quantifier::plus_quantifier();
                case type::T_QUESTION_MARK:
                    match(type::T_QUESTION_MARK);
                    return new quantifier::question_quantifier();
                case type::T_BRACE_OPEN: {
                    unsigned int start, end;

                    match(type::T_BRACE_OPEN);
                    start = numbers();
                    match(type::T_COMMA);
                    end = numbers();
                    match(type::T_BRACE_CLOSE);

                    return new quantifier::range_quantifier(start, end);
                }
                default:
                    return nullptr;
            }
        }

        unsigned int numbers() {
            if (lookahead->type == type::T_NUMBER) {
                unsigned int prev = get_digit();
                match(type::T_NUMBER);

                return numbers(prev);
            }

            throw syntax_exception("Expected token to be a number");
        }

        unsigned int numbers(unsigned int prev) {
            if (lookahead->type == type::T_NUMBER) {
                prev = prev * 10 + get_digit();
                match(type::T_NUMBER);

                return numbers(prev);
            }

            return prev;
        }

        char get_char() {
            auto* symbol = check<lexer::symbol_token>("Expected token to be a symbol token instance");

            return symbol->symbol;
        }

        uint8_t get_digit() {
            auto* digit = check<lexer::number_token>("Expected token to be a number token instance");

            return digit->number;
        }

        static node_ptr make_ast_from_set(const std::set<char>& set) {
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