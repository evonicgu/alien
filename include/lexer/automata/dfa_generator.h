#ifndef ALIEN_DFA_GENERATOR_H
#define ALIEN_DFA_GENERATOR_H

#include <array>
#include <queue>
#include <unordered_set>
#include <stack>

#include "dfa.h"
#include "nfa.h"
#include "partition.h"
#include "util/u8string.h"
#include "util/vecset.h"
#include "util/charutils.h"

namespace alien::lexer::automata {

    class dfa_generator {
        std::unordered_set<util::u8char> alphabet;
        std::map<dfa::nfa_set, dfa::nfa_set> cache;

    public:
        explicit dfa_generator(std::unordered_set<util::u8char>&& alphabet)
            : alphabet(std::move(alphabet)) {}

        dfa::dfa get_minimized_dfa(nfa::state* start_state) {
            return minimize(convert_automata(start_state));
        }

    private:
        const dfa::nfa_set& closure(const dfa::nfa_set& states);

        static dfa::nfa_set move(const dfa::nfa_set& states, util::u8char c);

        static dfa::nfa_set cmove(const dfa::nfa_set& states, util::u8char c, util::u8char unwanted);

        static dfa::state make_state(const dfa::nfa_set& states);

        dfa::dfa convert_automata(nfa::state* start_state);

        static dfa::dfa minimize(const dfa::dfa& automata);
    };

}

#endif //ALIEN_DFA_GENERATOR_H