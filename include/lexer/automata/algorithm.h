#ifndef ALIEN_AUTOMATA_ALGORITHM_H
#define ALIEN_AUTOMATA_ALGORITHM_H

#include <queue>
#include <memory>
#include "automata.h"
#include "generalized/generalized exception.h"
#include "lexer/regex/ast.h"

namespace alien::automata::algorithm {

    namespace {
        
        namespace exception {

            static constexpr const char node_type_exception_str[] = "Wrong node type";
            
            using node_type_exception = alien::generalized::generalized_exception<node_type_exception_str>;
        }
        
        using namespace lexer::regex::parser::ast;

        struct simple_nfa {
            nfa::state_ptr first, last;
        };

        template<typename T>
        T* check(const std::shared_ptr<node>& n) {
            auto* casted = dynamic_cast<T*>(n.get());
            
            if (casted == nullptr) {
                throw exception::node_type_exception();
            }
            
            return casted;
        }

        std::pair<simple_nfa, std::set<char>> traverse(const std::shared_ptr<node>& tree, int rule_number) {
            switch (tree->type) {
                case node::node_type::CONCAT: {
                    auto* n = check<concat_node>(tree);

                    auto lhs = traverse(n->first, rule_number), rhs = traverse(n->second, rule_number);
                    auto nfa1 = lhs.first, nfa2 = rhs.first;

                    nfa1.last->accepting = false;
                    nfa1.last->transitions = nfa2.first->transitions;

                    std::set<char> new_alphabet;
                    std::merge(lhs.second.begin(), lhs.second.end(),
                               rhs.second.begin(), rhs.second.end(),
                               std::inserter(new_alphabet, new_alphabet.begin()));

                    return {{nfa1.first, nfa2.last}, new_alphabet};
                }
                case node::node_type::STAR: {
                    auto* n = check<star_node>(tree);

                    auto lhs = traverse(n->first, rule_number);
                    auto nfa1 = lhs.first;
                    nfa1.last->accepting = false;

                    nfa::state_ptr first = std::make_shared<nfa::state>(), last = std::make_shared<nfa::state>();
                    first->transitions[-1] = {nfa1.first, last};
                    first->rule_number = rule_number;
                    nfa1.last->transitions[-1] = {nfa1.first, last};
                    last->rule_number = rule_number;
                    last->accepting = true;

                    return {{first, last}, lhs.second};
                }
                case node::node_type::OR: {
                    auto* n = check<or_node>(tree);

                    auto lhs = traverse(n->first, rule_number), rhs = traverse(n->second, rule_number);
                    auto nfa1 = lhs.first, nfa2 = rhs.first;

                    nfa1.last->accepting = false;
                    nfa2.last->accepting = false;

                    nfa::state_ptr first = std::make_shared<nfa::state>(), last = std::make_shared<nfa::state>();
                    first->transitions[-1] = {nfa1.first, nfa2.first};
                    first->rule_number = rule_number;
                    last->accepting = true;
                    last->rule_number = rule_number;
                    nfa1.last->transitions[-1] = {last};
                    nfa2.last->transitions[-1] = {last};

                    std::set<char> new_alphabet;
                    std::merge(lhs.second.begin(), lhs.second.end(),
                               rhs.second.begin(), rhs.second.end(),
                               std::inserter(new_alphabet, new_alphabet.begin()));

                    return {{first, last}, new_alphabet};
                }
                case node::node_type::LEAF: {
                    auto* n = check<leaf>(tree);

                    nfa::state_ptr first = std::make_shared<nfa::state>(), last = std::make_shared<nfa::state>();
                    first->transitions[n->symbol] = {last};
                    first->rule_number = rule_number;
                    last->accepting = true;
                    last->rule_number = rule_number;

                    return {{first, last}, {n->symbol}};
                }
            }
        }

        std::pair<nfa::state_ptr, std::set<char>> nfa_from_tree(const std::shared_ptr<node>& tree, int rule_number) {
            auto result = traverse(tree, rule_number);

            return {result.first.first, result.second};
        }

    }

    namespace {

        template<typename T>
        struct access_less {
            const std::vector<T>& arr;

            using is_transparent = std::true_type;

            explicit access_less(const std::vector<T>& arr) : arr(arr) {}

            bool operator()(const T& lhs, const unsigned int rhs) const {
                return lhs < arr[rhs];
            }

            bool operator()(const unsigned int lhs, const T& rhs) const {
                return arr[lhs] < rhs;
            }

            bool operator()(const unsigned int lhs, unsigned int rhs) const {
                return arr[lhs] < arr[rhs];
            }
        };

        using simple_set = std::stack<unsigned int>;

        struct partition {
            std::vector<unsigned int> elements, loc, sidx, first, last, mid;
            unsigned int sets = 1;

            explicit partition(unsigned int max) {
                elements.resize(max);
                loc.resize(max);
                sidx.resize(max);
                first.resize(max);
                last.resize(max);
                mid.resize(max);

                first[0] = mid[0] = 0;
                last[0] = max;

                for (unsigned int i = 0; i < max; ++i) {
                    elements[i] = loc[i] = i;
                    sidx[i] = 0;
                }
            }

            unsigned int size(unsigned int index) {
                return last[index] - first[index];
            }

            unsigned int set(unsigned int element) {
                return sidx[element];
            }

            unsigned int get_first(unsigned int index) {
                return elements[first[index]];
            }

            unsigned int get_next(unsigned int element) {
                if (loc[element] + 1 >= last[sidx[element]]) {
                    return 0;
                }

                return elements[loc[element] + 1];
            }

            bool no_marks(unsigned int index) {
                return mid[index] == first[index];
            }

            void mark(unsigned int element) {
                unsigned int s = sidx[element];
                unsigned int l = loc[element], m = mid[s];

                if (l >= m) {
                    elements[l] = elements[m];
                    loc[elements[l]] = l;

                    elements[m] = element;
                    loc[element] = m;
                    mid[s] = m + 1;
                }
            }

            unsigned int split(unsigned int index) {
                if (mid[index] == last[index]) {
                    mid[index] = first[index];
                }

                if (mid[index] == first[index]) {
                    return 0;
                }
                ++sets;

                first[sets - 1] = first[index];
                mid[sets - 1] = first[index];
                last[sets - 1] = mid[index];
                first[index] = mid[index];

                for (unsigned int l = first[sets - 1]; l < last[sets - 1]; ++l) {
                    sidx[elements[l]] = sets - 1;
                }

                return sets - 1;
            }
        };

        void _closure(dfa::nfa_set& closure_states, std::queue<nfa::state_ptr>& q) {
            while (!q.empty()) {
                auto& qf = q.front();

                if (qf->transitions.find(-1) != qf->transitions.end()) {
                    for (const auto& closure_state : qf->transitions[-1]) {
                        if (closure_states.find(closure_state) == closure_states.end()) {
                            closure_states.insert(closure_state);
                            q.push(closure_state);
                        }
                    }
                }

                q.pop();
            }
        }

        dfa::nfa_set closure(const nfa::state_ptr& state) {
            dfa::nfa_set closure_states = {state};

            std::queue<nfa::state_ptr> q;
            q.push(state);

            _closure(closure_states, q);

            return closure_states;
        }

        dfa::nfa_set closure(const dfa::nfa_set& states) {
            dfa::nfa_set closure_states = states;

            std::queue<nfa::state_ptr> q;
            for (const auto& state : states) {
                q.push(state);
            }

            _closure(closure_states, q);

            return closure_states;
        }

        dfa::nfa_set move(const dfa::nfa_set& states, char c) {
            dfa::nfa_set reached_states;

            for (const auto& state : states) {
                if (state->transitions.find(c) != state->transitions.end()) {
                    for (const auto& reached_state : state->transitions[c]) {
                        reached_states.insert(reached_state);
                    }
                }
            }

            return reached_states;
        }

        void set_accepting(dfa::state& state) {
            for (const auto& nfa_state : state.nfa_states) {
                if (nfa_state->accepting) {
                    state.accepting = true;
                    state.rule_number = nfa_state->rule_number;

                    return;
                }
            }

            state.accepting = false;
        }

        dfa::state make_state(dfa::nfa_set&& nfa_states) {
            dfa::state state;
            state.nfa_states = std::move(nfa_states);
            set_accepting(state);

            return state;
        }

        dfa::dfa convert_nfa2dfa(const nfa::state_ptr& state, const std::set<char>& alphabet) {
            dfa::dfa automata;
            automata.start_state = 0;
            automata.states.push_back(make_state(closure(state)));
            unsigned int state_counter = 0;

            std::set<unsigned int, access_less<dfa::state>> states{{0}, access_less<dfa::state>(automata.states)};
            std::queue<unsigned int> q;
            q.push(0);

            while (!q.empty()) {
                dfa::state current = automata.states[q.front()];
                if (current.accepting) {
                    automata.fstates.push_back(q.front());
                    automata.rulemap[current.rule_number].push_back(q.front());
                }

                for (char c : alphabet) {
                    dfa::state new_state = make_state(closure(move(current.nfa_states, c)));

                    if (new_state.nfa_states.empty()) {
                        continue;
                    }

                    auto found_state = states.find<dfa::state>(new_state);
                    unsigned int to_state, tr_index = automata.tails.size();
                    current.out_transitions.push_back(tr_index);

                    if (found_state != states.end()) {
                        to_state = *found_state;

                        if (to_state == q.front()) {
                            current.in_transitions.push_back(tr_index);
                        } else {
                            automata.states[to_state].in_transitions.push_back(tr_index);
                        }
                    } else {
                        to_state = ++state_counter;

                        automata.states.push_back(std::move(new_state));
                        states.insert(state_counter);
                        q.push(state_counter);
                        automata.states[state_counter].in_transitions.push_back(tr_index);
                    }

                    automata.tails.push_back(q.front());
                    automata.labels.push_back(c);
                    automata.heads.push_back(to_state);
                }

                automata.states[q.front()] = std::move(current);
                automata.transitions += automata.states[q.front()].out_transitions.size();

                q.pop();
            }

            return automata;
        }

        void sort_transitions(dfa::dfa& automata) {
            std::array<std::pair<unsigned int, std::vector<unsigned int>>, 128> buffer = {};
            buffer.fill({0, {}});

            std::vector<char> labels(automata.transitions);
            std::vector<unsigned int> tails(automata.transitions), heads(automata.transitions);

            for (auto& state : automata.states) {
                state.in_transitions.clear();
                state.out_transitions.clear();
            }

            for (unsigned int i = 0; i < automata.transitions; ++i) {
                ++buffer[automata.labels[i]].first;
                buffer[automata.labels[i]].second.push_back(i);
            }

            for (unsigned int i = 1; i < 128; ++i) {
                buffer[i].first += buffer[i - 1].first;
            }

            for (unsigned int i = 0; i < automata.transitions; ++i) {
                char value = automata.labels[i];
                unsigned int& index = buffer[value].first;

                labels[index - 1] = value;
                tails[index - 1] = automata.tails[i];
                automata.states[automata.tails[i]].out_transitions.push_back(index - 1);
                heads[index - 1] = automata.heads[i];
                automata.states[automata.heads[i]].in_transitions.push_back(--index);
            }

            automata.labels = labels;
            automata.tails = tails;
            automata.heads = heads;
        }

        dfa::dfa minimize(dfa::dfa&& automata, const std::set<char>& alphabet) {
            dfa::dfa min_automata;
            partition blocks(automata.states.size()), splitters(automata.transitions);
            simple_set unready_splitters, touched_blocks, touched_splitters;
            unready_splitters.push(0);
            sort_transitions(automata);

            auto split = [&](unsigned int block) {
                unsigned int b = blocks.split(block);

                if (b == 0) {
                    return;
                }

                if (blocks.size(block) < blocks.size(b)) {
                    std::swap(block, b);
                }

                unsigned int q = blocks.get_first(b);

                while (q != 0) {
                    for (unsigned int transition : automata.states[q].in_transitions) {
                        unsigned int splitter = splitters.set(transition);

                        if (splitters.no_marks(splitter)) {
                            touched_splitters.push(splitter);
                        }

                        splitters.mark(transition);
                    }

                    q = blocks.get_next(q);
                }

                while (!touched_splitters.empty()) {
                    unsigned int splitter = touched_splitters.top(), nsplitter = splitters.split(splitter);
                    touched_splitters.pop();

                    if (nsplitter != 0) {
                        unready_splitters.push(nsplitter);
                    }
                }
            };

            char lchar = automata.labels[0];

            for (unsigned int i = 0; i < automata.transitions; ++i) {
                if (lchar != automata.labels[i]) {
                    splitters.split(0);
                    unready_splitters.push(unready_splitters.top() + 1);
                    lchar = automata.labels[i];
                }

                splitters.mark(i);
            }

            splitters.split(0);

            for (const auto& rule_states : automata.rulemap) {
                for (unsigned int state : rule_states.second) {
                    blocks.mark(state);
                }

                split(0);
            }

            while (!unready_splitters.empty()) {
                unsigned int splitter = unready_splitters.top(), transition = splitters.get_first(splitter);
                unready_splitters.pop();

                while (transition != 0) {
                    unsigned int state = automata.tails[transition], block = blocks.set(state);

                    if (blocks.no_marks(block)) {
                        touched_blocks.push(block);
                    }

                    blocks.mark(state);
                    transition = splitters.get_next(transition);
                }

                while (!touched_blocks.empty()) {
                    unsigned int block = touched_blocks.top();
                    touched_blocks.pop();

                    split(block);
                }
            }

            min_automata.states.resize(blocks.sets, {{}, {}, {}, false, -1});
            min_automata.start_state = blocks.set(0);
            std::set<std::pair<unsigned int, char>> transitions;

            for (unsigned int fstate : automata.fstates) {
                unsigned int m_fstate = blocks.set(fstate);

                min_automata.states[m_fstate].accepting = true;
                min_automata.states[m_fstate].rule_number = automata.states[fstate].rule_number;
            }

            for (unsigned int i = 0; i < automata.transitions; ++i) {
                unsigned int s_tail = automata.tails[i], s_head = automata.heads[i];
                unsigned int m_tail = blocks.set(s_tail), m_head = blocks.set(s_head);
                char label = automata.labels[i];

                if (transitions.find({m_tail, label}) != transitions.end()) {
                    continue;
                }

                transitions.insert({m_tail, label});
                min_automata.states[m_tail].out_transitions.push_back(min_automata.transitions++);
                min_automata.tails.push_back(m_tail);
                min_automata.heads.push_back(m_head);
                min_automata.labels.push_back(label);
            }

            return min_automata;
        }

    }

}

#endif //ALIEN_AUTOMATA_ALGORITHM_H