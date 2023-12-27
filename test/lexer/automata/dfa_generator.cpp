#include <memory>
#include <set>

#include "gtest/gtest.h"

#include "lexer/automata/dfa_generator.h"
#include "lexer/automata/nfa.h"
#include "lexer/automata/dfa.h"

namespace alien::test {

    TEST(dfa_generator_tests, closure_test) {
        lexer::automata::dfa_generator gen({});

        std::unique_ptr<lexer::automata::nfa::state> start(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> end(new lexer::automata::nfa::state({}, false, -1));

        start->transitions.insert({-1, {end.get(), nullptr}});
        auto result = gen.closure({start.get()});

        EXPECT_EQ(result.size(), 3);

        EXPECT_NE(result.find(start.get()), result.end());
        EXPECT_NE(result.find(end.get()), result.end());
        EXPECT_NE(result.find(nullptr), result.end());
    }

    TEST(dfa_generator_tests, move_test) {
        std::unique_ptr<lexer::automata::nfa::state> start(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> first(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> second(new lexer::automata::nfa::state({}, false, -1));

        start->transitions.insert({'a', {first.get(), nullptr}});
        start->transitions.insert({'b', {second.get()}});

        auto result_a = lexer::automata::dfa_generator::move({start.get(), nullptr, second.get()}, 'a');

        EXPECT_EQ(result_a.size(), 2);
        EXPECT_NE(result_a.find(first.get()), result_a.end());
        EXPECT_NE(result_a.find(nullptr), result_a.end());

        auto result_b = lexer::automata::dfa_generator::move({start.get()}, 'b');

        EXPECT_EQ(result_b.size(), 1);
        EXPECT_NE(result_b.find(second.get()), result_b.end());
    }

    TEST(dfa_generator_tests, cmove_test) {
        std::unique_ptr<lexer::automata::nfa::state> start(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> first(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> second(new lexer::automata::nfa::state({}, false, -1));

        start->transitions.insert({'a', {first.get()}});

        auto result = lexer::automata::dfa_generator::cmove({start.get(), second.get(), nullptr}, 'a', 'b');

        EXPECT_EQ(result.size(), 1);
        EXPECT_NE(result.find(first.get()), result.end());

        start->transitions.insert({'b', {second.get()}});

        result = lexer::automata::dfa_generator::cmove({start.get()}, 'a', 'b');

        EXPECT_TRUE(result.empty());
    }

    TEST(dfa_generator_tests, make_state_test) {
        std::unique_ptr<lexer::automata::nfa::state> start(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> first(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> second(new lexer::automata::nfa::state({}, true, 2));

        auto dfa_state = lexer::automata::dfa_generator::make_state({start.get(), first.get()});

        EXPECT_FALSE(dfa_state.accepting);
        EXPECT_EQ(dfa_state.rule_number, -1);
        EXPECT_EQ(dfa_state.nfa_states.size(), 2);
        EXPECT_NE(dfa_state.nfa_states.find(start.get()), dfa_state.nfa_states.end());
        EXPECT_NE(dfa_state.nfa_states.find(first.get()), dfa_state.nfa_states.end());

        dfa_state = lexer::automata::dfa_generator::make_state({start.get(), first.get(), second.get()});

        EXPECT_TRUE(dfa_state.accepting);
        EXPECT_EQ(dfa_state.rule_number, 2);
        EXPECT_EQ(dfa_state.nfa_states.size(), 3);
        EXPECT_NE(dfa_state.nfa_states.find(start.get()), dfa_state.nfa_states.end());
        EXPECT_NE(dfa_state.nfa_states.find(first.get()), dfa_state.nfa_states.end());
        EXPECT_NE(dfa_state.nfa_states.find(second.get()), dfa_state.nfa_states.end());
    }

    TEST(dfa_generator_tests, convert_test) {
        std::unique_ptr<lexer::automata::nfa::state> start(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> first(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> second(new lexer::automata::nfa::state({}, true, 0));

        start->transitions[-1] = {first.get()};

        first->transitions['a'] = {second.get()};
        first->transitions['b'] = {nullptr};
        first->transitions[-8] = {nullptr};

        lexer::automata::dfa_generator gen({'a', 'b', -8});

        auto dfa = gen.convert_automata(start.get());

        EXPECT_EQ(dfa.states.size(), 3);
        EXPECT_TRUE(dfa.null_state_used);
        ASSERT_EQ(dfa.fstates.size(), 1);
        EXPECT_EQ(dfa.fstates[0], 2);
        EXPECT_FALSE(dfa.transitions_to_start);
        EXPECT_EQ(dfa.start_state, 1);
        ASSERT_EQ(dfa.rulemap[0].size(), 1);
        EXPECT_EQ(dfa.rulemap[0][0], 2);
        ASSERT_EQ(dfa.transitions.size(), 3);

        auto transitions = (std::set<lexer::automata::dfa::transition>) dfa.transitions;

        EXPECT_NE(transitions.find({1, 2, 'a'}), transitions.end());
        EXPECT_NE(transitions.find({1, 0, 'b'}), transitions.end());
        EXPECT_NE(transitions.find({1, 0, -8}), transitions.end());
    }

    TEST(dfa_generator_tests, simple_minimize_test) {
        std::unique_ptr<lexer::automata::nfa::state> start(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> first(new lexer::automata::nfa::state({}, false, -1));
        std::unique_ptr<lexer::automata::nfa::state> second(new lexer::automata::nfa::state({}, true, 0));

        lexer::automata::dfa_generator gen({'a', 'b'});

        start->accepting = true;
        start->rule_number = 0;
        start->transitions['a'] = {first.get()};
        start->transitions['b'] = {first.get()};

        first->accepting = true;
        first->rule_number = 0;
        first->transitions['a'] = {second.get()};
        first->transitions['b'] = {second.get()};

        second->accepting = true;
        second->rule_number = 0;
        second->transitions['a'] = {second.get()};
        second->transitions['b'] = {second.get()};

        auto result = gen.convert_automata(start.get());

        auto minimized = gen.minimize(result);

        ASSERT_EQ(minimized.states.size(), 2);
        EXPECT_EQ(minimized.null_state_used, false);
        EXPECT_EQ(minimized.start_state, 1);
        EXPECT_EQ(minimized.transitions_to_start, true);
        ASSERT_EQ(minimized.transitions.size(), 2);
        EXPECT_EQ(minimized.transitions[1], (lexer::automata::dfa::transition{1, 1, 'a'}));
        EXPECT_EQ(minimized.transitions[0], (lexer::automata::dfa::transition{1, 1, 'b'}));

        auto null_state = minimized.states[0];

        EXPECT_FALSE(null_state.accepting);
        EXPECT_TRUE(null_state.in_transitions.empty());
        EXPECT_TRUE(null_state.out_transitions.empty());
        EXPECT_EQ(null_state.rule_number, -1);

        auto start_state = minimized.states[1];

        EXPECT_TRUE(start_state.accepting);
        ASSERT_EQ(start_state.in_transitions.size(), 2);
        ASSERT_EQ(start_state.out_transitions.size(), 2);
        EXPECT_EQ(start_state.rule_number, 0);
        EXPECT_EQ(start_state.in_transitions[0], 0);
        EXPECT_EQ(start_state.in_transitions[1], 1);
        EXPECT_EQ(start_state.out_transitions[0], 0);
        EXPECT_EQ(start_state.out_transitions[1], 1);
    }

    TEST(dfa_generator_tests, minimize_test) {
        lexer::automata::dfa::dfa automata;

        automata.null_state_used = false;
        automata.transitions_to_start = false;
        automata.fstates = {5};
        automata.rulemap = {{0, {5}}};
        automata.start_state = 1;
        automata.transitions = {
            {1, 2, 'a'},
            {1, 3, 'b'},
            {2, 2, 'a'},
            {2, 4, 'b'},
            {3, 3, 'b'},
            {3, 2, 'a'},
            {4, 2, 'a'},
            {4, 5, 'b'},
            {5, 3, 'b'},
            {5, 2, 'a'}
        };
        automata.states = {
            {{}, {}, {}, false, -1},
            {{}, {}, {0, 1}, false, 0},
            {{}, {0, 2, 5, 6, 9}, {2, 3}, false, 0},
            {{}, {0, 4, 8}, {4, 5}, false, 0},
            {{}, {3}, {6, 7}, false, 0},
            {{}, {7}, {8, 9}, true, 0}
        };

        lexer::automata::dfa_generator gen({'a', 'b'});

        auto minimized = gen.minimize(automata);

        ASSERT_EQ(minimized.states.size(), 5);
        EXPECT_EQ(minimized.null_state_used, false);
        EXPECT_EQ(minimized.transitions_to_start, true);
        ASSERT_EQ(minimized.transitions.size(), 8);

        auto null_state = minimized.states[0];

        EXPECT_FALSE(null_state.accepting);
        EXPECT_TRUE(null_state.in_transitions.empty());
        EXPECT_TRUE(null_state.out_transitions.empty());
        EXPECT_EQ(null_state.rule_number, -1);

        auto start_state = minimized.states[minimized.start_state];

        EXPECT_FALSE(start_state.accepting);
        EXPECT_EQ(start_state.rule_number, -1);
        ASSERT_EQ(start_state.in_transitions.size(), 2);
        ASSERT_EQ(start_state.out_transitions.size(), 2);

        auto first_transition = minimized.transitions[start_state.in_transitions[0]];
        auto second_transition = minimized.transitions[start_state.in_transitions[1]];

        if (first_transition.head != first_transition.tail) {
            std::swap(first_transition, second_transition);
        }

        EXPECT_EQ(first_transition, (lexer::automata::dfa::transition{minimized.start_state, minimized.start_state, 'b'}));

        auto final_state = minimized.states[second_transition.tail];

        EXPECT_TRUE(final_state.accepting);
        EXPECT_EQ(final_state.rule_number, 0);
        ASSERT_EQ(final_state.out_transitions.size(), 2);
        ASSERT_EQ(final_state.in_transitions.size(), 1);

        auto final_first_out_transition = minimized.transitions[final_state.out_transitions[0]];
        auto final_second_out_transition = minimized.transitions[final_state.out_transitions[1]];

        if (final_second_out_transition.head == minimized.start_state) {
            std::swap(final_first_out_transition, final_second_out_transition);
        }

        EXPECT_EQ(final_first_out_transition, second_transition);

        auto second_out_transition = minimized.transitions[start_state.out_transitions[1]];

        if (second_out_transition.head == second_out_transition.tail) {
            second_out_transition = minimized.transitions[start_state.out_transitions[0]];
        }

        EXPECT_EQ(final_second_out_transition.head, second_out_transition.head);
        EXPECT_EQ(final_second_out_transition.label, 'a');
        EXPECT_EQ(second_out_transition.label, 'a');

        EXPECT_NE(std::find(
            minimized.transitions.vbegin(),
            minimized.transitions.vend(),
            lexer::automata::dfa::transition{second_out_transition.head, second_out_transition.head, 'a'}),
            minimized.transitions.vend());

        auto final_in_transition = minimized.transitions[final_state.in_transitions[0]];

        EXPECT_EQ(final_in_transition.label, 'b');

        EXPECT_NE(std::find(
            minimized.transitions.vbegin(),
            minimized.transitions.vend(),
            lexer::automata::dfa::transition{second_out_transition.head, final_in_transition.tail, 'b'}),
            minimized.transitions.vend());

        EXPECT_NE(std::find(
            minimized.transitions.vbegin(),
            minimized.transitions.vend(),
            lexer::automata::dfa::transition{final_in_transition.tail, second_out_transition.head, 'a'}),
            minimized.transitions.vend());
    }
}
