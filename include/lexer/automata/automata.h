#ifndef ALIEN_AUTOMATA_H
#define ALIEN_AUTOMATA_H

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace alien::automata {

    namespace nfa {

        struct state;
        using state_ptr = std::shared_ptr<state>;

        struct state {
            std::map<char, std::set<state_ptr>> transitions;

            bool accepting;

            int rule_number;

            bool operator<(const state& other) const {
                if (this->accepting) {
                    if (other.accepting) {
                        return this->rule_number < other.rule_number;
                    }
                    return true;
                }

                if (other.accepting) {
                    return false;
                }

                return this < &other;
            }
        };

    }

    namespace dfa {

        template<typename T>
        struct ptr_less {
            bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const {
                return *lhs < *rhs;
            }
        };

        using nfa_set = std::set<nfa::state_ptr, ptr_less<nfa::state>>;

        struct state {
            nfa_set nfa_states;
            std::vector<unsigned int> in_transitions, out_transitions;

            bool accepting;

            int rule_number = -1;

            bool operator<(const state& other) const {
                return this->nfa_states < other.nfa_states;
            }
        };

        struct dfa {
            std::vector<state> states;

            std::map<unsigned int, std::vector<unsigned int>> rulemap;
            std::vector<unsigned int> tails, heads, fstates;
            std::vector<char> labels;

            unsigned int transitions = 0, start_state;
        };

    }

}

#endif //ALIEN_AUTOMATA_H