#include "lexer/automata/dfa_generator.h"

#include <algorithm>
#include <array>
#include <queue>
#include <stack>
#include <vector>

#include "util/vecset.h"
#include "lexer/automata/partition.h"
#include "util/charutils.h"

namespace alien::lexer::automata {

    const dfa::nfa_set& dfa_generator::closure(const dfa::nfa_set& states) {
        auto cached = cache.find(states);

        if (cached != cache.end()) {
            return cached->second;
        }

        dfa::nfa_set closure_states = states;
        std::queue<nfa::state*> q;

        for (auto state : states) {
            q.push(state);
        }

        while (!q.empty()) {
            auto qf = q.front();

            if (qf == nullptr) {
                q.pop();
                continue;
            }

            auto it = qf->transitions.find(-1);

            if (it == qf->transitions.end()) {
                q.pop();
                continue;
            }

            for (auto closure_state : it->second) {
                if (closure_states.find(closure_state) == closure_states.end()) {
                    closure_states.insert(closure_state);
                    q.push(closure_state);
                }
            }

            q.pop();
        }

        return cache.insert({states, std::move(closure_states)}).first->second;
    }

    dfa::nfa_set dfa_generator::move(const dfa::nfa_set& states, util::u8char c) {
        dfa::nfa_set reached_states;

        for (auto state : states) {
            if (state == nullptr) {
                continue;
            }

            auto it = state->transitions.find(c);

            if (it == state->transitions.end()) {
                continue;
            }

            for (auto reached_state : it->second) {
                reached_states.insert(reached_state);
            }
        }

        return reached_states;
    }

    dfa::nfa_set dfa_generator::cmove(const dfa::nfa_set& states, util::u8char c, util::u8char unwanted) {
        dfa::nfa_set reached_states;

        for (auto state : states) {
            if (state == nullptr || state->transitions.find(unwanted) != state->transitions.end()) {
                continue;
            }

            auto it = state->transitions.find(c);

            if (it == state->transitions.end()) {
                continue;
            }

            for (auto reached_state : it->second) {
                reached_states.insert(reached_state);
            }
        }

        return reached_states;
    }

    dfa::state dfa_generator::make_state(const dfa::nfa_set& states) {
        dfa::state state;
        state.nfa_states = states;

        if (!state.nfa_states.empty()) {
            auto& first = *state.nfa_states.begin();

            if (first != nullptr && first->accepting) {
                state.accepting = true;
                state.rule_number = first->rule_number;
            }
        }

        return state;
    }

    /**
     * Converts NFA to DFA using Powerset Construction Algorithm
     * extended to handle large character classes based on unicode
     * properties efficiently.
     *
     * Character classes based on unicode properties are implemented
     * as negative integers in the range [-33; -3] (the appropriate properties.
     * are listed in the src/lexer/regex/parser.cpp file). So, special handling
     * needs to be done with them and corresponding real characters to
     * construct DFAs equivalent to DFAs that would be hypothetically
     * constructed if each character class was represented as an alternation
     * of its characters.
     */
    dfa::dfa dfa_generator::convert_automata(nfa::state* start_state) {
        dfa::dfa automata;
        automata.start_state = 1;

        // initial states - null state, closure of start state
        util::vecset<dfa::state> states{{}, make_state(closure({start_state}))};

        // partition the unordered alphabet, so that negative characters representing unicode property
        // char classes would be processed first
        std::vector<util::u8char> sorted_alphabet{alphabet.begin(), alphabet.end()};

        std::partition(sorted_alphabet.begin(), sorted_alphabet.end(), [](util::u8char c) {
            return c < 0;
        });

        static const dfa::nfa_set null_state = {nullptr};

        for (std::size_t i = 1; i < states.size(); ++i) {
            // construct the final states array
            if (states[i].accepting) {
                automata.fstates.push_back(i);
                automata.rulemap[states[i].rule_number].push_back(i);
            }

            // track present char classes
            std::array<bool, 31> classes{};
            classes.fill(false);

            for (util::u8char c : sorted_alphabet) {
                if (c == -1) {
                    continue;
                }

                // transition into new state
                dfa::state new_state = make_state(closure(move(states[i].nfa_states, c)));

                if (new_state.nfa_states.empty()) {
                    continue;
                }

                if (c < 0) {
                    classes[c + sizeof classes + 2] = true;
                } else {
                    util::u8char char_class = util::get_class(c);

                    /**
                     * If there is a transition from current state labeled by unicode property
                     * char class that includes current character, than add closure of move from
                     * current state with the char class, but exclude states that have transitions
                     * with the character (they may have different transitions with the char class,
                     * thus, they are unwanted, and are unreachable since the matching process
                     * firstly checks for transitions with the specific character, then the char
                     * class
                     */
                    if (classes[char_class + sizeof classes + 2]) {
                        dfa::nfa_set class_states = closure(cmove(states[i].nfa_states, char_class, c));
                        class_states.merge(new_state.nfa_states);

                        new_state = make_state(class_states);
                    }
                }

                if (new_state.nfa_states == null_state) {
                    states[i].out_transitions.push_back(automata.transitions.size());
                    states[0].in_transitions.push_back(automata.transitions.size());
                    automata.transitions.push_back({i, 0, c});

                    automata.null_state_used = true;
                    continue;
                }

                auto it = states.find(new_state);
                std::size_t to, transition = automata.transitions.size();
                states[i].out_transitions.push_back(transition);

                if (it != states.vend()) {
                    to = it - states.vbegin();
                } else {
                    to = states.size();

                    states.push_back(std::move(new_state));
                }

                states[to].in_transitions.push_back(transition);
                automata.transitions.push_back({i, to, c});
            }
        }

        automata.states = (std::vector<dfa::state>) states;
        return automata;
    }

    dfa::dfa dfa_generator::minimize(const dfa::dfa& automata) {
        dfa::dfa min_automata;
        partition::partition blocks(automata.states.size()), splitters(automata.transitions.size());
        std::stack<std::size_t> unready_splitters, touched_blocks, touched_splitters;
        unready_splitters.push(0);

        min_automata.null_state_used = automata.null_state_used;

        auto split = [&](std::size_t block) {
            std::size_t b = blocks.split(block);

            if (b == 0) {
                return;
            }

            if (blocks.size(block) < blocks.size(b)) {
                std::swap(block, b);
            }

            std::ptrdiff_t state = blocks.get_first(b);

            while (state != -1) {
                for (std::size_t transition : automata.states[state].in_transitions) {
                    std::size_t splitter = splitters.set(transition);

                    if (splitters.no_marks(splitter)) {
                        touched_splitters.push(splitter);
                    }

                    splitters.mark(transition);
                }

                state = blocks.get_next(state);
            }

            while (!touched_splitters.empty()) {
                std::size_t splitter = touched_splitters.top(), nsplitter = splitters.split(splitter);
                touched_splitters.pop();

                if (nsplitter != 0) {
                    unready_splitters.push(nsplitter);
                }
            }
        };

        auto it = automata.transitions.sbegin();
        util::u8char lchar = automata.transitions[it->index].label;

        while (it != automata.transitions.send()) {
            if (automata.transitions[it->index].label != lchar) {
                splitters.split(0);
                unready_splitters.push(unready_splitters.top() + 1);
                lchar = automata.transitions[it->index].label;
            }

            splitters.mark(it->index);
            ++it;
        }

        splitters.split(0);

        for (const auto& rule_states : automata.rulemap) {
            for (std::size_t state : rule_states.second) {
                blocks.mark(state);
            }

            split(0);
        }

        while (!unready_splitters.empty()) {
            std::size_t splitter = unready_splitters.top();
            std::ptrdiff_t transition = splitters.get_first(splitter);
            unready_splitters.pop();

            while (transition != -1) {
                std::size_t state = automata.transitions[transition].tail, block = blocks.set(state);

                if (blocks.no_marks(block)) {
                    touched_blocks.push(block);
                }

                blocks.mark(state);
                transition = splitters.get_next(transition);
            }

            while (!touched_blocks.empty()) {
                std::size_t block = touched_blocks.top();
                touched_blocks.pop();

                split(block);
            }
        }

        min_automata.states.resize(blocks.sets, {{}, {}, {}, false, -1});
        min_automata.start_state = blocks.set(1);

        for (std::size_t fstate : automata.fstates) {
            std::size_t min_fstate = blocks.set(fstate);

            min_automata.states[min_fstate].accepting = true;
            min_automata.states[min_fstate].rule_number = automata.states[fstate].rule_number;
        }

        for (std::size_t i = 0; i < automata.transitions.size(); ++i) {
            std::size_t tail = automata.transitions[i].tail, head = automata.transitions[i].head;
            std::size_t min_tail = blocks.set(tail), min_head = blocks.set(head);
            util::u8char label = automata.transitions[i].label;

            if (min_automata.transitions.find({min_tail, min_head, label}) != min_automata.transitions.vend()) {
                continue;
            }

            if (min_head == min_automata.start_state) {
                min_automata.transitions_to_start = true;
            }

            std::size_t transition = min_automata.transitions.push_back({min_tail, min_head, label});
            min_automata.states[min_tail].out_transitions.push_back(transition);
            min_automata.states[min_head].in_transitions.push_back(transition);
        }

        return min_automata;
    }
}