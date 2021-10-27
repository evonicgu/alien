#ifndef ALIEN_AUTOMATA_H
#define ALIEN_AUTOMATA_H

#include <map>
#include <memory>
#include <set>
#include <vector>
#include "util/util.h"

namespace alien::automata {

    namespace nfa {

        struct state;
        using state_ptr = std::shared_ptr<state>;

        struct state {
            std::map<util::u8char, std::set<state_ptr>> transitions;

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

                if (this->rule_number == other.rule_number) {
                    return this < &other;
                }

                return this->rule_number < other.rule_number;
            }
        };

    }

    namespace dfa {

        template<typename T>
        struct ptr_less {
            bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const {
                if (rhs == nullptr) {
                    return false;
                }

                if (lhs == nullptr) {
                    return true;
                }

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

        struct transition {
            unsigned int tail, head;

            util::u8char label;

            bool operator<(const transition& other) const {
                if (label == other.label) {
                    if (tail == other.tail) {
                        return head < other.head;
                    }

                    return tail < other.tail;
                }

                return label < other.label;
            }
        };

        struct dfa {
            std::vector<state> states;

            std::map<unsigned int, std::vector<unsigned int>> rulemap;
            std::vector<unsigned int> fstates;
            util::vecset<transition> transitions;

            unsigned int start_state;
        };

    }

}

#endif //ALIEN_AUTOMATA_H