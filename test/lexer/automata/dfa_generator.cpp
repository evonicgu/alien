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

}
