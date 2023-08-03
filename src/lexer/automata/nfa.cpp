#include "lexer/automata/nfa.h"

namespace alien::lexer::automata::nfa {

    bool state::operator<(const state& other) const {
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

}