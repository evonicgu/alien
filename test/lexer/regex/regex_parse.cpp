#include <unordered_map>
#include <list>
#include <unordered_set>
#include <stdexcept>
#include <utility>

#include "gtest/gtest.h"

#include "input/input.h"
#include "lexer/regex/parser.h"
#include "lexer/regex/lexer.h"
#include "util/typeutils.h"
#include "util/u8string.h"

namespace alien::test {

    using namespace util::literals;

#define BASE_REGEX(no_utf8) std::list<util::u8string> err; lexer::regex::lexer l(regex, err); lexer::regex::parser p(l, err, (no_utf8))

#define expect_base(var, name, ntype, struct_type) EXPECT_EQ((var)->type, (ntype)); auto name = util::check<struct_type>(var.get())

#define expect_concat(var, name) expect_base(var, name, lexer::regex::ast::node::node_type::CONCAT, lexer::regex::ast::concat_node)
#define expect_or(var, name) expect_base(var, name, lexer::regex::ast::node::node_type::OR, lexer::regex::ast::or_node)
#define expect_neg_class(var, name) expect_base(var, name, lexer::regex::ast::node::node_type::NEGATIVE_CLASS, lexer::regex::ast::negative_class)
#define expect_star(var, name) expect_base(var, name, lexer::regex::ast::node::node_type::STAR, lexer::regex::ast::star_node)

    bool expect_leaf(const lexer::regex::ast::node_ptr& node, util::u8char check_symbol) {
        EXPECT_EQ(node->type, lexer::regex::ast::node::node_type::LEAF);
        auto leaf = util::check<lexer::regex::ast::leaf>(node.get());
        EXPECT_EQ(leaf->symbol, check_symbol);

        return leaf->symbol == check_symbol;
    }

    bool verify_char_class(const std::unordered_set<util::u8char>& characters, const lexer::regex::ast::node_ptr& node) {
        if (characters.size() == 1) {
            return expect_leaf(node, *characters.begin());
        }

        expect_or(node, char_class);

        std::unordered_map<util::u8char, bool> class_characters;

        for (auto c : characters) {
            class_characters.insert({c, false});
        }

        while (char_class->second->type != lexer::regex::ast::node::node_type::LEAF) {
            EXPECT_EQ(char_class->first->type, lexer::regex::ast::node::node_type::LEAF);
            EXPECT_EQ(char_class->second->type, lexer::regex::ast::node::node_type::OR);

            auto curr_leaf = util::check<lexer::regex::ast::leaf>(char_class->first.get());

            EXPECT_NE(class_characters.find(curr_leaf->symbol), class_characters.end());
            EXPECT_FALSE(class_characters.at(curr_leaf->symbol));

            class_characters[(char) curr_leaf->symbol] = true;

            char_class = util::check<lexer::regex::ast::or_node>(char_class->second.get());
        }

        {
            auto curr_leaf = util::check<lexer::regex::ast::leaf>(char_class->first.get());

            EXPECT_NE(class_characters.find(curr_leaf->symbol), class_characters.end());
            EXPECT_FALSE(class_characters.at(curr_leaf->symbol));

            class_characters[(char) curr_leaf->symbol] = true;
        }

        {
            auto curr_leaf = util::check<lexer::regex::ast::leaf>(char_class->second.get());

            EXPECT_NE(class_characters.find(curr_leaf->symbol), class_characters.end());
            EXPECT_FALSE(class_characters.at(curr_leaf->symbol));

            class_characters[(char) curr_leaf->symbol] = true;
        }

        bool characters_found = true;

        for (auto [_, found] : class_characters) {
            characters_found &= found;
        }

        return characters_found;
    }

    TEST(regex_tests, regex_parse_concat_test) {
        input::string_input regex(R"(ab)"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat);

        expect_leaf(concat->first, 'a');
        expect_leaf(concat->second, 'b');
    }

    TEST(regex_tests, regex_parse_or_test) {
        input::string_input regex(R"(a|b)"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_or(ast, or_node);

        expect_leaf(or_node->first, 'a');
        expect_leaf(or_node->second, 'b');
    }

    TEST(regex_tests, regex_parse_paren_test) {
        input::string_input regex(R"((a|b)b)"_u8); // ab or bb

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat);

        expect_or(concat->first, alternation);
        expect_leaf(concat->second, 'b');

        expect_leaf(alternation->first, 'a');
        expect_leaf(alternation->second, 'b');
    }

    TEST(regex_tests, regex_parse_unmatched_paren_test) {
        input::string_input regex("a|b)"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Unmatched parenthesis at 1:4"_u8);
    }


    TEST(regex_tests, regex_parse_unmatched_paren_at_start_test) {
        input::string_input regex(")"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Unmatched parenthesis at 1:1"_u8);
    }

    TEST(regex_tests, regex_parse_or_at_start_test) {
        input::string_input regex("|"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Unexpected '|' at 1:1"_u8);
    }

    TEST(regex_tests, regex_parse_empty_test) {
        input::string_input regex(""_u8);

        BASE_REGEX(false);

        EXPECT_THROW(p.parse(), std::runtime_error);
    }

    TEST(regex_tests, regex_parse_empty_group_test) {
        input::string_input regex("()"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Empty group at 1:1"_u8);
    }

    TEST(regex_tests, regex_parse_negative_class_test) {
        input::string_input regex("[^a-zA-Z]"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_neg_class(ast, neg_class);

        for (char i = 'a'; i <= 'z'; ++i) {
            EXPECT_NE(neg_class->negative_chars.find(i), neg_class->negative_chars.end());
            EXPECT_NE(neg_class->negative_chars.find(toupper(i)), neg_class->negative_chars.end());
        }

        EXPECT_EQ(neg_class->negative_chars.size(), 26 + 26);
    }

    TEST(regex_tests, regex_parse_class_range_out_of_order_test) {
        input::string_input regex("[^z-a]"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Range is out of order at 1:6"_u8);
    }

    TEST(regex_tests, regex_parse_class_unexpected_end_test) {
        input::string_input regex("[^a-z"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Expected ']' at 1:6"_u8);
    }

    TEST(regex_tests, regex_parse_class_hyphen_at_start_test) {
        input::string_input regex("[^-a-z]"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_neg_class(ast, neg_class);

        for (char i = 'a'; i <= 'z'; ++i) {
            EXPECT_NE(neg_class->negative_chars.find(i), neg_class->negative_chars.end());
        }

        EXPECT_NE(neg_class->negative_chars.find('-'), neg_class->negative_chars.end());
        EXPECT_EQ(neg_class->negative_chars.size(), 26 + 1);
    }

    TEST(regex_tests, regex_parse_hyphen_wrong_position_inside_class_test) {
        input::string_input regex("[^a-b-]"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Unexpected - in character class at 1:6"_u8);
    }

    TEST(regex_tests, regex_parse_unexpected_token_inside_class_test) {
        input::string_input regex("[^a-b[]"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Unexpected token in character class at 1:6"_u8);
    }

    TEST(regex_tests, regex_parse_special_characters_inside_class_test) {
        input::string_input regex("[-a1,^|+*?(){}.]"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        EXPECT_TRUE(verify_char_class({'-', 'a', '1', ',', '^', '|', '+', '*', '?', '(', ')', '{', '}', '.'}, p.get_ast()));
    }

    TEST(regex_tests, regex_parse_hyphen_test) {
        input::string_input regex("-"_u8);

        BASE_REGEX(false);

        p.parse();

        auto ast = p.get_ast();

        EXPECT_EQ(err.size(), 0);

        expect_leaf(ast, '-');
    }

    TEST(regex_tests, regex_parse_empty_neg_class_test) {
        input::string_input regex("[^]"_u8);

        BASE_REGEX(false);

        p.parse();

        auto ast = p.get_ast();

        EXPECT_EQ(err.size(), 0);

        expect_neg_class(ast, neg_class);

        EXPECT_EQ(neg_class->negative_chars.size(), 0);
    }

    TEST(regex_tests, regex_parse_empty_class_test) {
        input::string_input regex("[]"_u8);

        BASE_REGEX(false);

        p.parse();

        auto ast = p.get_ast();

        EXPECT_EQ(err.size(), 0);

        expect_leaf(ast, -1);
    }

    TEST(regex_tests, regex_parse_class_with_space_test) {
        input::string_input regex("[e-\\N]"_u8);

        BASE_REGEX(false);

        EXPECT_THROW(p.parse(), std::runtime_error);
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_dot_test) {
        input::string_input regex("."_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> full_chars;

        for (int i = 0; i < 128; ++i) {
            if (i == 10) {
                continue;
            }

            full_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(full_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_valid_sequence_test) {
        input::string_input regex("\\X"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> valid_sequence_chars;

        for (int i = 0; i < 128; ++i) {
            valid_sequence_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(valid_sequence_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_unicode_newline_test) {
        input::string_input regex("\\R"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> unicode_newline_chars{10, 12, 13};

        EXPECT_TRUE(verify_char_class(unicode_newline_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_space_test) {
        input::string_input regex("\\s"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> space_chars{9, 10, 11, 12, 13, 32};

        EXPECT_TRUE(verify_char_class(space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_non_space_test) {
        input::string_input regex("\\S"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_space_chars;

        for (int i = 0; i < 128; ++i) {
            if ((i >= 9 && i <= 13) || i == 32) {
                continue;
            }

            non_space_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_horizontal_space_test) {
        input::string_input regex("\\h"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> horizontal_space_chars{9, 32};

        EXPECT_TRUE(verify_char_class(horizontal_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_non_horizontal_space_test) {
        input::string_input regex("\\H"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_horizontal_chars;

        for (int i = 0; i < 128; ++i) {
            if (i == 9 || i == 32) {
                continue;
            }

            non_horizontal_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_horizontal_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_digit_test) {
        input::string_input regex("\\d"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> digit_chars;

        for (int i = 0; i < 10; ++i) {
            digit_chars.insert('0' + i);
        }

        EXPECT_TRUE(verify_char_class(digit_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_non_digit_test) {
        input::string_input regex("\\D"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_digit_chars;

        for (int i = 0; i < 128; ++i) {
            if (i >= '0' && i <= '9') {
                continue;
            }

            non_digit_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_digit_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_non_newline_test) {
        input::string_input regex("\\N"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_newline_chars;

        for (int i = 0; i < 128; ++i) {
            if (i == '\n' || i == 12 || i == 13) {
                continue;
            }

            non_newline_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_newline_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_vertical_space_test) {
        input::string_input regex("\\v"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> vertical_space_chars{10, 11, 12, 13,};

        EXPECT_TRUE(verify_char_class(vertical_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_non_vertical_space_test) {
        input::string_input regex("\\V"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> vertical_space_chars;

        for (int i = 0; i < 128; ++i) {
            if (i >= 10 && i <= 13) {
                continue;
            }

            vertical_space_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(vertical_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_word_char_test) {
        input::string_input regex("\\w"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> word_chars{'_'};

        for (int i = 'a'; i <= 'z'; ++i) {
            word_chars.insert(i);
            word_chars.insert(toupper(i));
        }

        for (int i = 0; i < 10; ++i) {
            word_chars.insert('0' + i);
        }

        EXPECT_TRUE(verify_char_class(word_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_non_word_char_test) {
        input::string_input regex("\\W"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_word_chars;

        for (int i = 0; i < 128; ++i) {
            if (isalnum(i) || i == '_') {
                continue;
            }

            non_word_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_word_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_no_utf8_unicode_class_test) {
        input::string_input regex("\\p{Cc}"_u8);

        BASE_REGEX(true);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Unicode property used in no-utf8 mode at pos 1:1"_u8);
    }

    TEST(regex_tests, regex_parse_shortcut_dot_test) {
        input::string_input regex("."_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> full_chars;

        for (int i = -33; i < -2; ++i) {
            if (i == -5 || i == -31 || i == -32) {
                continue;
            }

            full_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(full_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_valid_sequence_test) {
        input::string_input regex("\\X"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> valid_sequence_chars;

        for (int i = -33; i < -2; ++i) {
            valid_sequence_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(valid_sequence_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_unicode_newline_test) {
        input::string_input regex("\\R"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> unicode_newline_chars{-5, -31, -32};

        EXPECT_TRUE(verify_char_class(unicode_newline_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_space_test) {
        input::string_input regex("\\s"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> space_chars{-4, -5, -7, -31, -32, -33};

        EXPECT_TRUE(verify_char_class(space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_non_space_test) {
        input::string_input regex("\\S"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_space_chars;

        for (int i = -33; i < -2; ++i) {
            if (i == -4 || i == -5 || i == -7 || i == -31 || i == -32 || i == -33) {
                continue;
            }

            non_space_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_horizontal_space_test) {
        input::string_input regex("\\h"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> horizontal_space_chars{-4, -7, -33};

        EXPECT_TRUE(verify_char_class(horizontal_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_non_horizontal_space_test) {
        input::string_input regex("\\H"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_horizontal_chars;

        for (int i = -33; i < -2; ++i) {
            if (i == -4 || i == -7 || i == -33) {
                continue;
            }

            non_horizontal_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_horizontal_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_digit_test) {
        input::string_input regex("\\d"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> digit_chars{-16};

        EXPECT_TRUE(verify_char_class(digit_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_non_digit_test) {
        input::string_input regex("\\D"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_digit_chars;

        for (int i = -33; i < -2; ++i) {
            if (i == -16) {
                continue;
            }

            non_digit_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_digit_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_non_newline_test) {
        input::string_input regex("\\N"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_newline_chars;

        for (int i = -33; i < -2; ++i) {
            if (i == -5 || i == -31 || i == -32) {
                continue;
            }

            non_newline_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_newline_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_vertical_space_test) {
        input::string_input regex("\\v"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> vertical_space_chars{-5, -31, -32,};

        EXPECT_TRUE(verify_char_class(vertical_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_non_vertical_space_test) {
        input::string_input regex("\\V"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> vertical_space_chars;

        for (int i = -33; i < -2; ++i) {
            if (i == -5 || i == -31 || i == -32) {
                continue;
            }

            vertical_space_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(vertical_space_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_word_char_test) {
        input::string_input regex("\\w"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> word_chars{-8, -9, -10, -11, -12, -16, -17, -18, -20};

        EXPECT_TRUE(verify_char_class(word_chars, ast));
    }

    TEST(regex_tests, regex_parse_shortcut_non_word_char_test) {
        input::string_input regex("\\W"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        std::unordered_set<util::u8char> non_word_chars;
        const std::unordered_set<util::u8char> word_chars{-8, -9, -10, -11, -12, -16, -17, -18, -20};

        for (int i = -33; i < -2; ++i) {
            if (word_chars.find(i) != word_chars.end()) {
                continue;
            }

            non_word_chars.insert(i);
        }

        EXPECT_TRUE(verify_char_class(non_word_chars, ast));
    }

    class regex_tests_parse_utf8_class_test_fixture :
        public ::testing::TestWithParam<std::pair<std::unordered_set<util::u8char>, util::u8string>> {};

    TEST_P(regex_tests_parse_utf8_class_test_fixture, regex_parse_utf8_class_test) {
        const auto& [expected, value] = GetParam();

        if (value.size() == 1) {
            input::string_input regex("\\p"_u8 + value);

            BASE_REGEX(false);

            p.parse();

            EXPECT_EQ(err.size(), 0);

            auto ast = p.get_ast();

            EXPECT_TRUE(verify_char_class(expected, ast));
        }

        {
            input::string_input regex("\\p{"_u8 + value + (util::u8char) '}');

            BASE_REGEX(false);

            p.parse();

            EXPECT_EQ(err.size(), 0);

            auto ast = p.get_ast();

            EXPECT_TRUE(verify_char_class(expected, ast));
        }
    }

    std::pair<std::unordered_set<util::u8char>, util::u8string> regex_parse_utf8_class_values[] = {
        {{-3, -4, -5, -6, -7}, "c"_u8},
        {{-8, -9, -10, -11, -12}, "l"_u8},
        {{-13, -14, -15}, "m"_u8},
        {{-16, -17, -18}, "n"_u8},
        {{-19, -20, -21, -22, -23, -24, -25, -26}, "p"_u8},
        {{-27, -28, -29, -30}, "s"_u8},
        {{-31, -32, -33}, "z"_u8},
        {{-3, -4, -5}, "cc"_u8},
        {{-6, -7}, "cf"_u8},
        {{-8}, "ll"_u8},
        {{-9}, "lm"_u8},
        {{-10}, "lo"_u8},
        {{-11}, "lt"_u8},
        {{-12}, "lu"_u8},
        {{-13}, "mc"_u8},
        {{-14}, "me"_u8},
        {{-15}, "mn"_u8},
        {{-16}, "nd"_u8},
        {{-17}, "nl"_u8},
        {{-18}, "no"_u8},
        {{-19, -20}, "pc"_u8},
        {{-21}, "pd"_u8},
        {{-22}, "pe"_u8},
        {{-23}, "pf"_u8},
        {{-24}, "pi"_u8},
        {{-25}, "po"_u8},
        {{-26}, "ps"_u8},
        {{-27}, "sc"_u8},
        {{-28}, "sk"_u8},
        {{-29}, "sm"_u8},
        {{-30}, "so"_u8},
        {{-31}, "zl"_u8},
        {{-32}, "zp"_u8},
        {{-33}, "zs"_u8},
    };

    INSTANTIATE_TEST_SUITE_P(regex_tests_parse_utf8_class_values,
                             regex_tests_parse_utf8_class_test_fixture,
                             ::testing::ValuesIn(regex_parse_utf8_class_values));

    class regex_tests_wrong_utf8_class_test_fixture :
        public ::testing::TestWithParam<util::u8string> {};

    TEST_P(regex_tests_wrong_utf8_class_test_fixture, regex_parse_wrong_utf8_class_test) {
        const auto& value = GetParam();

        input::string_input regex(value);

        try {
            BASE_REGEX(false);

            p.parse();

            FAIL() << "Should have thrown an exception";
        } catch (std::runtime_error&) {
            EXPECT_TRUE(true);
        }
    }

    util::u8string regex_parse_wrong_utf8_class_values[] = {
        "\\pK"_u8,
        "\\p{"_u8,
        "\\p{}"_u8,
        "\\p{cz}"_u8,
        "\\p1"_u8,
        "\\p{1c}"_u8,
        "\\p{cz}"_u8,
        "\\p{lz}"_u8,
        "\\p{mz}"_u8,
        "\\p{nz}"_u8,
        "\\p{pz}"_u8,
        "\\p{sz}"_u8,
        "\\p{zz}"_u8
    };

    INSTANTIATE_TEST_SUITE_P(regex_tests_parse_utf8_class_values,
                             regex_tests_wrong_utf8_class_test_fixture,
                             ::testing::ValuesIn(regex_parse_wrong_utf8_class_values));

    TEST(regex_tests, regex_parse_star_quantifier_test) {
        input::string_input regex("a*"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_star(ast, star_node);
        expect_leaf(star_node->first, 'a');
    }

    TEST(regex_tests, regex_parse_plus_quantifier_test) {
        input::string_input regex("a+"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat);

        expect_leaf(concat->first, 'a');

        expect_star(concat->second, star);

        expect_leaf(star->first, 'a');
    }

    TEST(regex_tests, regex_parse_question_quantifier_test) {
        input::string_input regex("a?"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_or(ast, or_node);

        EXPECT_EQ(or_node->first->type, lexer::regex::ast::node::node_type::LEAF);
        EXPECT_EQ(or_node->second->type, lexer::regex::ast::node::node_type::LEAF);

        expect_leaf(or_node->first, 'a');
        expect_leaf(or_node->second, -1);
    }

    TEST(regex_tests, regex_parse_repeat_quantifier_2_2_test) {
        input::string_input regex("a{2,2}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat_node);

        expect_leaf(concat_node->first, 'a');
        expect_leaf(concat_node->first, 'a');
    }

    TEST(regex_tests, regex_parse_repeat_quantifier_1_2_test) {
        input::string_input regex("a{1,2}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat_node);

        expect_leaf(concat_node->first, 'a');

        expect_or(concat_node->second, second_leaf_wrapper);

        expect_leaf(second_leaf_wrapper->first, 'a');
        expect_leaf(second_leaf_wrapper->second, -1);
    }

    TEST(regex_tests, regex_parse_repeat_quantifier_0_2_test) {
        input::string_input regex("a{0,2}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat_node);

        expect_or(concat_node->first, first_leaf_wrapper);

        expect_leaf(first_leaf_wrapper->first, 'a');
        expect_leaf(first_leaf_wrapper->second, -1);

        expect_or(concat_node->second, second_leaf_wrapper);

        expect_leaf(second_leaf_wrapper->first, 'a');
        expect_leaf(second_leaf_wrapper->second, -1);
    }

    TEST(regex_tests, regex_parse_repeat_quantifier_0_0_test) {
        input::string_input regex("a{0,0}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_leaf(ast, -1);
    }

    TEST(regex_tests, regx_parse_repeat_quantifier_none_2_test) {
        input::string_input regex("a{,2}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat_node);

        expect_or(concat_node->first, first_leaf_wrapper);

        expect_leaf(first_leaf_wrapper->first, 'a');
        expect_leaf(first_leaf_wrapper->second, -1);

        expect_or(concat_node->second, second_leaf_wrapper);

        expect_leaf(second_leaf_wrapper->first, 'a');
        expect_leaf(second_leaf_wrapper->second, -1);
    }

    TEST(regex_tests, regex_parse_repeat_quantifier_2_none_test) {
        input::string_input regex("a{2,}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_concat(ast, concat_node);

        expect_concat(concat_node->first, leaf_concat);

        expect_leaf(leaf_concat->first, 'a');
        expect_leaf(leaf_concat->second, 'a');

        expect_star(concat_node->second, star);
        expect_leaf(star->first, 'a');
    }

    TEST(regex_tests, regex_parse_repeat_quantifier_0_none_test) {
        input::string_input regex("a{0,}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_star(ast, star);
        expect_leaf(star->first, 'a');
    }

    TEST(regex_tests, regex_parse_repeat_quantifier_none_none_test) {
        input::string_input regex("a{,}"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        expect_leaf(ast, -1);
    }

    TEST(regex_tests, regex_parse_unpreceded_quantifier_test) {
        input::string_input regex("?"_u8);

        BASE_REGEX(false);

        EXPECT_THROW(p.parse(), std::runtime_error);
    }

    TEST(regex_tests, regex_parse_unclosed_parenthesis_test) {
        input::string_input regex("(a"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 1);

        EXPECT_EQ(err.front(), "Expected ')' at 1:3"_u8);
    }

    class regex_tests_parse_character_test_fixture :
        public ::testing::TestWithParam<std::pair<util::u8char, util::u8string>> {};

    TEST_P(regex_tests_parse_character_test_fixture, regex_tests_parse_character_test) {
        auto& [expected, value] = GetParam();

        input::string_input regex(value);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        const auto ast = p.get_ast();

        expect_leaf(ast, expected);
    }

    std::pair<util::u8char, util::u8string> regex_tests_parse_character_values[] = {
        {'a', "a"_u8},
        {'b', "b"_u8},
        {'1', "1"_u8},
        {'2', "2"_u8},
        {'\n', "\\n"_u8},
        {'\t', "\\t"_u8},
        {'\f', "\\f"_u8},
        {'\r', "\\r"_u8},
        {'\0', "\\0"_u8},
        {4659, "\\u{1233}"_u8}
    };

    INSTANTIATE_TEST_SUITE_P(regex_tests_parse_character_test_values,
                            regex_tests_parse_character_test_fixture,
                            ::testing::ValuesIn(regex_tests_parse_character_values));

    TEST(regex_tests, regex_parse_integration_test) {
        // some basic username validation expression
        input::string_input regex(R"(\@[a-zA-Z_][a-zA-Z_0-9\.])"_u8);

        BASE_REGEX(false);

        p.parse();

        EXPECT_EQ(err.size(), 0);

        auto ast = p.get_ast();

        EXPECT_EQ(ast->type, lexer::regex::ast::node::node_type::CONCAT);

        auto op = util::check<lexer::regex::ast::op_node>(ast.get());

        EXPECT_EQ(op->second->type, lexer::regex::ast::node::node_type::OR);

        EXPECT_EQ(op->first->type, lexer::regex::ast::node::node_type::CONCAT);

        auto first_op = util::check<lexer::regex::ast::op_node>(op->first.get());

        EXPECT_EQ(first_op->first->type, lexer::regex::ast::node::node_type::LEAF);

        auto leaf = util::check<lexer::regex::ast::leaf>(first_op->first.get());

        EXPECT_EQ(leaf->symbol, '@');

        std::unordered_set<util::u8char> first_characters, second_characters;

        for (char i = 'a'; i <= 'z'; ++i) {
            first_characters.insert(i);
            first_characters.insert(toupper(i));
        }

        first_characters.insert('_');

        second_characters = first_characters;

        for (char i = '0'; i <= '9'; ++i) {
            second_characters.insert(i);
        }

        second_characters.insert('.');

        EXPECT_TRUE(verify_char_class(first_characters, first_op->second));

        EXPECT_TRUE(verify_char_class(second_characters, op->second));
    }

}