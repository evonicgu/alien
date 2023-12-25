#include "gtest/gtest.h"

#include "lexer/regex/parser.h"

#include "lexer/automata/nfa_generator.h"

namespace alien::test {

    using namespace util::literals;

#define EXPECT_SINGLE_TRANSITION(curr_state, c, new_state) ASSERT_NE((curr_state)->transitions.find(c), (curr_state)->transitions.end()); \
    ASSERT_EQ((curr_state)->transitions[c].size(), 1); \
    lexer::automata::nfa::state* new_state = *((curr_state)->transitions[c].begin())
#define EXPECT_TWO_TRANSITIONS(curr_state, c, new_state1, new_state2) ASSERT_NE((curr_state)->transitions.find(c), (curr_state)->transitions.end()); \
    ASSERT_EQ((curr_state)->transitions[c].size(), 2); \
    lexer::automata::nfa::state* new_state1 = *((curr_state)->transitions[c].begin()); \
    lexer::automata::nfa::state* new_state2 = *(++(curr_state)->transitions[c].begin())
#define EXPECT_ACCEPTING(curr_state) EXPECT_TRUE((curr_state)->accepting)
#define EXPECT_NOT_ACCEPTING(curr_state) EXPECT_FALSE((curr_state)->accepting)
#define EXPECT_TRANSITION_COUNT(curr_state, count) EXPECT_EQ((curr_state)->transitions.size(), count)
#define EXPECT_RULE(curr_state, rule) EXPECT_EQ((curr_state)->rule_number, (rule))

#define BASE_REGEX(str) input::string_input regex(str##_u8); \
    std::list<util::u8string> err; \
    lexer::regex::lexer l(regex, err); \
    lexer::regex::parser p(l, err, false); \
    p.parse(); \
    auto ast = p.get_ast()

    TEST(nfa_generator_tests, concat_test) {
        BASE_REGEX("str");
        std::vector<std::unique_ptr<lexer::automata::nfa::state>> states;

        lexer::automata::nfa_generator gen(states);

        auto [start, alphabet] = gen.nfa_from_ast(ast, 0, false);

        EXPECT_EQ(alphabet, (std::unordered_set<util::u8char>{'s', 't', 'r'}));
        EXPECT_TRANSITION_COUNT(start, 1);
        EXPECT_NOT_ACCEPTING(start);

        EXPECT_SINGLE_TRANSITION(start, 's', after_s);
        EXPECT_TRANSITION_COUNT(after_s, 1);
        EXPECT_NOT_ACCEPTING(after_s);

        EXPECT_SINGLE_TRANSITION(after_s, 't', after_t);
        EXPECT_TRANSITION_COUNT(after_t, 1);
        EXPECT_NOT_ACCEPTING(after_t);

        EXPECT_SINGLE_TRANSITION(after_t, 'r', after_r);
        EXPECT_TRANSITION_COUNT(after_r, 0);
        EXPECT_ACCEPTING(after_r);
        EXPECT_RULE(after_r, 0);
    }

    TEST(nfa_generator_tests, alternation_test) {
        BASE_REGEX("s|t|r");

        std::vector<std::unique_ptr<lexer::automata::nfa::state>> states;

        lexer::automata::nfa_generator gen(states);

        auto [start, alphabet] = gen.nfa_from_ast(ast, 0, false);

        EXPECT_EQ(alphabet, (std::unordered_set<util::u8char>{'s', 't', 'r'}));

        EXPECT_TRANSITION_COUNT(start, 1);
        EXPECT_NOT_ACCEPTING(start);

        EXPECT_TWO_TRANSITIONS(start, -1, first_alt, second_alt);

        if (first_alt->transitions.find('r') != first_alt->transitions.end()) {
            /**
             * Swapping the pointers does not invalidate the testing, since the order does not matter
             * for further automata construction (even though std::set is used), it is just for purposes of testing
             */
            std::swap(first_alt, second_alt); // the expected order
        }

        EXPECT_TRANSITION_COUNT(first_alt, 1);
        EXPECT_NOT_ACCEPTING(first_alt);

        EXPECT_TWO_TRANSITIONS(first_alt, -1, s_alt, t_alt);

        if (s_alt->transitions.find('t') != s_alt->transitions.end()) {
            std::swap(s_alt, t_alt); // the expected order
        }

        EXPECT_TRANSITION_COUNT(s_alt, 1);
        EXPECT_NOT_ACCEPTING(s_alt);

        EXPECT_SINGLE_TRANSITION(s_alt, 's', after_s);
        EXPECT_TRANSITION_COUNT(after_s, 1);
        EXPECT_NOT_ACCEPTING(after_s);

        EXPECT_SINGLE_TRANSITION(t_alt, 't', after_t);
        EXPECT_TRANSITION_COUNT(after_t, 1);
        EXPECT_NOT_ACCEPTING(after_t);

        EXPECT_SINGLE_TRANSITION(after_s, -1, first_alt_end_after_s);
        EXPECT_TRANSITION_COUNT(first_alt_end_after_s, 1);
        EXPECT_NOT_ACCEPTING(first_alt_end_after_s);

        EXPECT_SINGLE_TRANSITION(after_t, -1, first_alt_end_after_t);
        EXPECT_EQ(first_alt_end_after_s, first_alt_end_after_t);

        EXPECT_TRANSITION_COUNT(second_alt, 1);
        EXPECT_NOT_ACCEPTING(second_alt);

        EXPECT_SINGLE_TRANSITION(second_alt, 'r', after_r);
        EXPECT_TRANSITION_COUNT(after_r, 1);
        EXPECT_NOT_ACCEPTING(after_r);

        EXPECT_SINGLE_TRANSITION(after_r, -1, overall_end_after_r);
        EXPECT_TRANSITION_COUNT(overall_end_after_r, 0);
        EXPECT_ACCEPTING(overall_end_after_r);
        EXPECT_RULE(overall_end_after_r, 0);

        EXPECT_SINGLE_TRANSITION(first_alt_end_after_s, -1, overall_end_after_st);
        EXPECT_EQ(overall_end_after_st, overall_end_after_r);
    }

    TEST(nfa_generator_tests, star_test) {
        BASE_REGEX("a*");

        std::vector<std::unique_ptr<lexer::automata::nfa::state>> states;

        lexer::automata::nfa_generator gen(states);

        auto [start, alphabet] = gen.nfa_from_ast(ast, 0, false);

        EXPECT_EQ(alphabet, std::unordered_set<util::u8char>{'a'});

        EXPECT_TRANSITION_COUNT(start, 1);
        EXPECT_NOT_ACCEPTING(start);

        EXPECT_TWO_TRANSITIONS(start, -1, quantified_start, overall_end_from_start);

        if (quantified_start->transitions.find('a') == quantified_start->transitions.end()) {
            std::swap(quantified_start, overall_end_from_start);
        }

        EXPECT_TRANSITION_COUNT(quantified_start, 1);
        EXPECT_NOT_ACCEPTING(quantified_start);

        EXPECT_SINGLE_TRANSITION(quantified_start, 'a', quantified_end);
        EXPECT_TRANSITION_COUNT(quantified_end, 1);
        EXPECT_NOT_ACCEPTING(quantified_end);

        EXPECT_TWO_TRANSITIONS(quantified_end, -1, to_quantified_start, overall_end);

        if (to_quantified_start != quantified_start) {
            std::swap(to_quantified_start, overall_end); // expected order
        }

        EXPECT_EQ(to_quantified_start, quantified_start);

        EXPECT_EQ(overall_end, overall_end_from_start);
        EXPECT_ACCEPTING(overall_end);
        EXPECT_TRANSITION_COUNT(overall_end, 0);
        EXPECT_RULE(overall_end, 0);
    }

    TEST(nfa_generator_tests, negative_class_test) {
        BASE_REGEX("[^abc]");

        std::vector<std::unique_ptr<lexer::automata::nfa::state>> states;

        lexer::automata::nfa_generator gen(states);

        auto [start, alphabet] = gen.nfa_from_ast(ast, 0, false);

        std::unordered_set<util::u8char> expected_alphabet{'a', 'b', 'c'};

        for (util::u8char i = -33; i <= -3; ++i) {
            expected_alphabet.insert(i);
        }

        EXPECT_EQ(alphabet, expected_alphabet);

        EXPECT_TRANSITION_COUNT(start, 34);
        EXPECT_NOT_ACCEPTING(start);

        EXPECT_SINGLE_TRANSITION(start, 'a', after_a);
        EXPECT_SINGLE_TRANSITION(start, 'b', after_b);
        EXPECT_SINGLE_TRANSITION(start, 'c', after_c);

        EXPECT_EQ(after_a, nullptr);
        EXPECT_EQ(after_b, nullptr);
        EXPECT_EQ(after_c, nullptr);

        lexer::automata::nfa::state* prev = nullptr;

        for (util::u8char i = -33; i <= -3; ++i) {
            EXPECT_SINGLE_TRANSITION(start, i, after_char_prop);

            EXPECT_TRANSITION_COUNT(after_char_prop, 0);
            EXPECT_ACCEPTING(after_char_prop);
            EXPECT_RULE(after_char_prop, 0);

            if (prev != nullptr) {
                EXPECT_EQ(after_char_prop, prev);
            }

            prev = after_char_prop;
        }
    }

    TEST(nfa_generator_tests, negative_class_test_no_utf8) {
        BASE_REGEX("[^abc]");

        std::vector<std::unique_ptr<lexer::automata::nfa::state>> states;

        lexer::automata::nfa_generator gen(states);

        auto [start, alphabet] = gen.nfa_from_ast(ast, 0, true);

        std::unordered_set<util::u8char> expected_alphabet;

        for (util::u8char i = 0; i < 128; ++i) {
            expected_alphabet.insert(i);
        }

        EXPECT_EQ(alphabet, expected_alphabet);

        EXPECT_TRANSITION_COUNT(start, 128);
        EXPECT_NOT_ACCEPTING(start);

        EXPECT_SINGLE_TRANSITION(start, 'a', after_a);
        EXPECT_SINGLE_TRANSITION(start, 'b', after_b);
        EXPECT_SINGLE_TRANSITION(start, 'c', after_c);

        EXPECT_EQ(after_a, nullptr);
        EXPECT_EQ(after_b, nullptr);
        EXPECT_EQ(after_c, nullptr);

        lexer::automata::nfa::state* prev = nullptr;

        for (util::u8char i = 0; i < 128; ++i) {
            if (i == 'a' || i == 'b' || i == 'c') {
                continue;
            }

            EXPECT_SINGLE_TRANSITION(start, i, after_char_prop);

            EXPECT_TRANSITION_COUNT(after_char_prop, 0);
            EXPECT_ACCEPTING(after_char_prop);
            EXPECT_RULE(after_char_prop, 0);

            if (prev != nullptr) {
                EXPECT_EQ(after_char_prop, prev);
            }

            prev = after_char_prop;
        }
    }

    TEST(nfa_generator_tests, no_utf8_wrong_character_exception_test) {
        BASE_REGEX("\\u{1233}");

        std::vector<std::unique_ptr<lexer::automata::nfa::state>> states;

        lexer::automata::nfa_generator gen(states);

        EXPECT_THROW(gen.nfa_from_ast(ast, 0, true), std::runtime_error);
    }

}