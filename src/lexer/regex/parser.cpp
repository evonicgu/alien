#include "lexer/regex/parser.h"

namespace alien::lexer::regex {

    void parser::parse() {
        ast = regex();

        match(type::T_END);
    }

    ast::node_ptr parser::get_ast() {
        return ast;
    }

    void parser::error(token_type expected, token_type got) {
        using namespace util::literals;

        util::pos err_pos = lookahead->start;

        switch (expected) {
            case type::T_END:
                throw std::runtime_error("End of the regex expected");
            case type::T_PARENTHESIS_CLOSE:
                err.push_back("Expected ')' at "_u8 + (util::u8string) err_pos);
                break;
            case type::T_SQUARE_BRACKET_CLOSE:
                err.push_back("Expected ']' at "_u8 + (util::u8string) err_pos);
                break;
            default:
                throw std::runtime_error("Unexpected token at " + (std::string) err_pos);
        }
    }

    ast::node_ptr parser::regex() {
        ast::node_ptr tree = factors();

        while (lookahead->type == type::T_OR) {
            match(type::T_OR);
            tree = std::make_shared<ast::or_node>(tree, factors());
        }

        return tree;
    }

    bool parser::check_lookahead(parser::type t) {
        return t != type::T_OR && t != type::T_END && t != type::T_PARENTHESIS_CLOSE;
    }

    ast::node_ptr parser::factors() {
        using namespace util::literals;

        ast::node_ptr tree = factor();

        while (check_lookahead(lookahead->type)) {
            tree = std::make_shared<ast::concat_node>(tree, factor());
        }

        if (lookahead->type == type::T_PARENTHESIS_CLOSE) {
            if (fold_level == 0) {
                util::pos err_pos = lookahead->start;

                match(type::T_PARENTHESIS_CLOSE);
                err.push_back("Unmatched parenthesis at "_u8 + (util::u8string) err_pos);
                return tree;
            }

            --fold_level;
        }

        return tree;
    }

    ast::node_ptr parser::factor() {
        using namespace util::literals;

        util::pos pos = lookahead->start;

        switch (lookahead->type) {
            case type::T_STAR:
            case type::T_PLUS:
            case type::T_BRACE_OPEN:
            case type::T_QUESTION_MARK:
                throw std::runtime_error("Preceding token is not quantifiable at " + (std::string) pos);
            case type::T_SQUARE_BRACKET_CLOSE:
            case type::T_BRACE_CLOSE:
            case type::T_PARENTHESIS_CLOSE:
                match(lookahead->type);
                err.push_back("Unmatched parenthesis at "_u8 + (util::u8string) pos);
                return std::make_shared<ast::leaf>(-1);
            case type::T_OR:
                match(type::T_OR);
                err.push_back("Unexpected '|' at "_u8 + (util::u8string) pos);
            case type::T_END:
                throw std::runtime_error("Unexpected End of regular expression at " + (std::string) pos);
            default: {
                ast::node_ptr tree = base();

                std::unique_ptr<quantifier::quantifier> q;

                switch (lookahead->type) {
                    case type::T_STAR:
                        q = std::make_unique<quantifier::star_quantifier>();
                        match(type::T_STAR);
                        break;
                    case type::T_PLUS:
                        q = std::make_unique<quantifier::plus_quantifier>();
                        match(type::T_PLUS);
                        break;
                    case type::T_QUESTION_MARK:
                        q = std::make_unique<quantifier::question_quantifier>();
                        match(type::T_QUESTION_MARK);
                        break;
                    case type::T_BRACE_OPEN: {
                        std::size_t start, end;

                        match(type::T_BRACE_OPEN);
                        start = get_number();
                        match(type::T_COMMA);
                        end = get_number();
                        match(type::T_BRACE_CLOSE);

                        q = std::make_unique<quantifier::range_quantifier>(start, end);
                        break;
                    }
                    default:
                        return tree;
                }

                return q->traverse(tree);
            }
        }
    }

    ast::node_ptr parser::base() {
        using namespace util::literals;

        ast::node_ptr tree;

        util::pos start = lookahead->start;

        switch (lookahead->type) {
            case type::T_PARENTHESIS_OPEN: {
                ++fold_level;
                match(type::T_PARENTHESIS_OPEN);

                if (lookahead->type == type::T_PARENTHESIS_CLOSE) {
                    err.push_back("Empty group at "_u8 + (util::u8string) start);

                    return std::make_shared<ast::leaf>(-1);
                }

                tree = regex();
                match(type::T_PARENTHESIS_CLOSE);
                break;
            }
            case type::T_COMMA:
            case type::T_HYPHEN:
            case type::T_SYMBOL:
            case type::T_NUMBER:
            case type::T_NEGATIVE_CLASS: {
                util::u8char c = get_char();

                tree = std::make_shared<ast::leaf>(c);
                break;
            }
            case type::T_SQUARE_BRACKET_OPEN:
                tree = char_class();
                break;
            default:
                tree = shortcut();
                break;
        }

        return tree;
    }

    ast::node_ptr parser::char_class() {
        match(type::T_SQUARE_BRACKET_OPEN);
        bool negative = false;

        if (lookahead->type == type::T_NEGATIVE_CLASS) {
            negative = true;
            match(type::T_NEGATIVE_CLASS);
        }

        std::unordered_set<util::u8char> characters = class_characters();

        match(type::T_SQUARE_BRACKET_CLOSE);

        if (negative) {
            return std::make_shared<ast::negative_class>(std::move(characters));
        }

        return ast_from_set(characters);
    }

    std::unordered_set<util::u8char> parser::class_characters() {
        using namespace util::literals;

        std::unordered_set<util::u8char> characters;

        while (lookahead->type != type::T_SQUARE_BRACKET_CLOSE) {
            switch (lookahead->type) {
                case type::T_SYMBOL:
                case type::T_NUMBER:
                case type::T_COMMA:
                case type::T_NEGATIVE_CLASS:
                case type::T_STAR:
                case type::T_OR:
                case type::T_PLUS:
                case type::T_PARENTHESIS_OPEN:
                case type::T_PARENTHESIS_CLOSE:
                case type::T_BRACE_OPEN:
                case type::T_BRACE_CLOSE:
                case type::T_DOT:
                case type::T_QUESTION_MARK: {
                    util::u8char start = get_char();

                    if (lookahead->type == type::T_HYPHEN) {
                        match(type::T_HYPHEN);
                        util::u8char end = get_char();

                        if (start > end) {
                            err.push_back("Range is out of order at "_u8 + (util::u8string) lookahead->start);
                            continue;
                        }

                        for (util::u8char i = start; i <= end; ++i) {
                            characters.insert(i);
                        }
                    } else {
                        characters.insert(start);
                    }

                    break;
                }
                case type::T_END:
                    return characters;
                case type::T_HYPHEN: {
                    if (characters.empty()) {
                        characters.insert('-');
                        match(type::T_HYPHEN);
                    } else {
                        util::pos err_pos = lookahead->start;

                        err.push_back("Unexpected - in character class at "_u8 + (util::u8string) err_pos);
                    }
                    break;
                }
                default: {
                    util::pos err_pos = lookahead->start;
                    match(lookahead->type);

                    err.push_back("Unexpected token in character class at "_u8 + (util::u8string) err_pos);
                }
            }
        }

        return characters;
    }

    ast::node_ptr parser::shortcut() {
        using namespace util::literals;

        ast::node_ptr tree;

        if (no_utf8) {
            switch (lookahead->type) {
                case type::T_CLASS: {
                    check<symbol_class_token>();

                    err.push_back("Unicode property used in no-utf8 mode at pos "_u8 + (util::u8string) lookahead->start);
                    tree = std::make_shared<ast::leaf>(-1);
                    break;
                }
                case type::T_DOT:
                    tree = ast_from_set(ranges::no_utf8::full);
                    break;
                case type::T_VALID_SEQUENCE:
                    tree = ast_from_set(ranges::no_utf8::valid_sequence);
                    break;
                case type::T_UNICODE_NEWLINE:
                    tree = ast_from_set(ranges::no_utf8::unicode_newline);
                    break;
                case type::T_SPACE:
                    tree = ast_from_set(ranges::no_utf8::space);
                    break;
                case type::T_NON_SPACE:
                    tree = ast_from_set(ranges::no_utf8::non_space);
                    break;
                case type::T_HORIZONTAL_SPACE:
                    tree = ast_from_set(ranges::no_utf8::h_space);
                    break;
                case type::T_NON_HORIZONTAL_SPACE:
                    tree = ast_from_set(ranges::no_utf8::non_h_space);
                    break;
                case type::T_DIGIT:
                    tree = ast_from_set({48, 49, 50, 51, 52, 53, 54, 55, 56, 57});
                    break;
                case type::T_NON_DIGIT:
                    tree = ast_from_set(ranges::no_utf8::non_digit);
                    break;
                case type::T_NON_NEWLINE:
                    tree = ast_from_set(ranges::no_utf8::non_newline);
                    break;
                case type::T_VERTICAL_SPACE:
                    tree = ast_from_set(ranges::no_utf8::v_space);
                    break;
                case type::T_NON_VERTICAL_SPACE:
                    tree = ast_from_set(ranges::no_utf8::non_v_space);
                    break;
                case type::T_WORD_CHAR:
                    tree = ast_from_set(ranges::no_utf8::word_char);
                    break;
                case type::T_NON_WORD_CHAR:
                    tree = ast_from_set(ranges::no_utf8::non_word_char);
                    break;
            }
        } else {
            switch (lookahead->type) {
                case type::T_CLASS: {
                    auto* token = check<symbol_class_token>();

                    tree = ast_from_set(symbol_class(token->class_name));
                    break;
                }
                case type::T_DOT:
                    tree = ast_from_set(ranges::full);
                    break;
                case type::T_VALID_SEQUENCE:
                    tree = ast_from_set(ranges::valid_sequence);
                    break;
                case type::T_UNICODE_NEWLINE:
                    tree = ast_from_set(ranges::unicode_newline);
                    break;
                case type::T_SPACE:
                    tree = ast_from_set(ranges::space);
                    break;
                case type::T_NON_SPACE:
                    tree = ast_from_set(ranges::non_space);
                    break;
                case type::T_HORIZONTAL_SPACE:
                    tree = ast_from_set(ranges::h_space);
                    break;
                case type::T_NON_HORIZONTAL_SPACE:
                    tree = ast_from_set(ranges::non_h_space);
                    break;
                case type::T_DIGIT:
                    tree = std::make_shared<ast::leaf>(-16);
                    break;
                case type::T_NON_DIGIT:
                    tree = ast_from_set(ranges::non_digit);
                    break;
                case type::T_NON_NEWLINE:
                    tree = ast_from_set(ranges::non_newline);
                    break;
                case type::T_VERTICAL_SPACE:
                    tree = ast_from_set(ranges::v_space);
                    break;
                case type::T_NON_VERTICAL_SPACE:
                    tree = ast_from_set(ranges::non_v_space);
                    break;
                case type::T_WORD_CHAR:
                    tree = ast_from_set(ranges::word_char);
                    break;
                case type::T_NON_WORD_CHAR:
                    tree = ast_from_set(ranges::non_word_char);
                    break;
            }
        }

        match(lookahead->type);

        return tree;
    }

    std::unordered_set<util::u8char> parser::symbol_class(const util::u8string& classname) {
        util::pos pos = lookahead->start;
        std::unordered_set<util::u8char> characters;

        if (classname.size() == 1) {
            characters = simple_symbol_class(classname);
        } else {
            characters = composed_symbol_class(classname);
        }

        if (characters.empty()) {
            throw std::runtime_error(
                    "Invalid class name: " + util::u8string_to_bytes(classname) + " at " + (std::string) pos
            );
        }

        return characters;
    }

    std::unordered_set<util::u8char> parser::simple_symbol_class(const util::u8string& classname) {
        switch (classname[0]) {
            case 'c':
                return {-3, -4, -5, -6, -7};
            case 'l':
                return {-8, -9, -10, -11, -12};
            case 'm':
                return {-13, -14, -15};
            case 'n':
                return {-16, -17, -18};
            case 'p':
                return {-19, -20, -21, -22, -23, -24, -25, -26};
            case 's':
                return {-27, -28, -29, -30};
            case 'z':
                return {-31, -32, -33};
        }

        return {};
    }

    std::unordered_set<util::u8char> parser::composed_symbol_class(const util::u8string& classname) {
        switch (classname[0]) {
            case 'c':
                switch (classname[1]) {
                    case 'c':
                        return {-3, -4, -5};
                    case 'f':
                        return {-6, -7};
                }
                break;
            case 'l':
                switch (classname[1]) {
                    case 'l':
                        return {-8};
                    case 'm':
                        return {-9};
                    case 'o':
                        return {-10};
                    case 't':
                        return {-11};
                    case 'u':
                        return {-12};
                }
                break;
            case 'm':
                switch (classname[1]) {
                    case 'c':
                        return {-13};
                    case 'e':
                        return {-14};
                    case 'n':
                        return {-15};
                }
                break;
            case 'n':
                switch (classname[1]) {
                    case 'd':
                        return {-16};
                    case 'l':
                        return {-17};
                    case 'o':
                        return {-18};
                }
                break;
            case 'p':
                switch (classname[1]) {
                    case 'c':
                        return {-19, -20};
                    case 'd':
                        return {-21};
                    case 'e':
                        return {-22};
                    case 'f':
                        return {-23};
                    case 'i':
                        return {-24};
                    case 'o':
                        return {-25};
                    case 's':
                        return {-26};
                }
                break;
            case 's':
                switch (classname[1]) {
                    case 'c':
                        return {-27};
                    case 'k':
                        return {-28};
                    case 'm':
                        return {-29};
                    case 'o':
                        return {-30};
                }
                break;
            case 'z':
                switch (classname[1]) {
                    case 'l':
                        return {-31};
                    case 'p':
                        return {-32};
                    case 's':
                        return {-33};
                }
                break;
        }

        return {};
    }

    util::u8char parser::get_char() {
        util::u8char c;

        switch (lookahead->type) {
            case type::T_SYMBOL: {
                auto* token = check<symbol_token>();
                c = token->symbol;
                break;
            }
            case type::T_NUMBER: {
                auto* token = check<number_token>();

                c = token->number + '0';
                break;
            }
            case type::T_COMMA:
                c = ',';
                break;
            case type::T_HYPHEN:
                c = '-';
                break;
            case type::T_NEGATIVE_CLASS:
                c = '^';
                break;
            case type::T_OR:
                c = '|';
                break;
            case type::T_PLUS:
                c = '+';
                break;
            case type::T_STAR:
                c = '*';
                break;
            case type::T_QUESTION_MARK:
                c = '?';
                break;
            case type::T_PARENTHESIS_OPEN:
                c = '(';
                break;
            case type::T_PARENTHESIS_CLOSE:
                c = ')';
                break;
            case type::T_BRACE_OPEN:
                c = '{';
                break;
            case type::T_BRACE_CLOSE:
                c = '}';
                break;
            case type::T_DOT:
                c = '.';
                break;
            default:
                throw std::runtime_error("Expected a character at " + (std::string) lookahead->start);
        }

        match(lookahead->type);
        return c;
    }

    std::size_t parser::get_number() {
        std::size_t number = 0;

        while (lookahead->type == type::T_NUMBER) {
            auto* token = check<number_token>();
            number = number * 10 + token->number;
            match(type::T_NUMBER);
        }

        return number;
    }

    ast::node_ptr parser::ast_from_set(const std::unordered_set<util::u8char>& charset) {
        if (charset.empty()) {
            return std::make_shared<ast::leaf>(-1);
        }

        ast::node_ptr tree = nullptr;

        for (auto c : charset) {
            if (tree == nullptr) {
                tree = std::make_shared<ast::leaf>(c);
            } else {
                tree = std::make_shared<ast::or_node>(std::make_shared<ast::leaf>(c), tree);
            }
        }

        return tree;
    }

}