#ifndef ALIEN_AUTOMATA_GENERATOR_H
#define ALIEN_AUTOMATA_GENERATOR_H

#include <list>
#include <vector>
#include <memory>

#include "dfa.h"
#include "nfa.h"
#include "lexer/config/rules/rules.h"
#include "util/u8string.h"

namespace alien::lexer::automata {

    class generator {
        std::list<util::u8string>& err;

        std::vector<std::unique_ptr<nfa::state>> nfa_states;

        bool no_utf8;

    public:
        generator(std::list<util::u8string>& err, bool no_utf8)
            : err(err),
              no_utf8(no_utf8) {}

        dfa::dfa generate_automata(std::vector<rules::rule>& rules);
    };

}

#endif //ALIEN_AUTOMATA_GENERATOR_H