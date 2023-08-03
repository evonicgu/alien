#ifndef ALIEN_NFA_GENERATOR_H
#define ALIEN_NFA_GENERATOR_H

#include <memory>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "lexer/automata/nfa.h"
#include "lexer/regex/ast.h"
#include "util/u8string.h"
#include "util/typeutils.h"

namespace alien::lexer::automata {

    class nfa_generator {
        std::vector<nfa::state*>& nfa_states;

    public:
        explicit nfa_generator(std::vector<nfa::state*>& nfa_states)
            : nfa_states(nfa_states) {}

        std::pair<nfa::state*, std::unordered_set<util::u8char>> nfa_from_ast(
                const regex::ast::node_ptr& ast,
                std::ptrdiff_t rule_number,
                bool no_utf8);
    };

}

#endif //ALIEN_NFA_GENERATOR_H