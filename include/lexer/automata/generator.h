#ifndef ALIEN_AUTOMATA_GENERATOR_H
#define ALIEN_AUTOMATA_GENERATOR_H

#include <list>
#include <set>
#include <vector>

#include "dfa.h"
#include "dfa_generator.h"
#include "lexer/config/rules/rules.h"
#include "lexer/regex/parser.h"
#include "nfa_generator.h"
#include "transition_table.h"
#include "util/u8string.h"

namespace alien::lexer::automata {

    class generator {
        std::list<util::u8string>& err;

        std::vector<nfa::state*> nfa_states;

        bool no_utf8;

    public:
        generator(std::list<util::u8string>& err, bool no_utf8)
            : err(err),
              no_utf8(no_utf8) {}

        dfa::dfa generate_automata(std::vector<rules::rule>& rules) {
            auto* start_state = new nfa::state{{}, false, -1};
            std::set<util::u8char> automata_alphabet;

            nfa_generator nfa_gen(nfa_states);

            for (auto& rule : rules) {
                input::string_input i(rule.regex);
                regex::lexer l(i, err);
                regex::parser p(l, err, rule.no_utf8 || no_utf8);

                p.parse();

                auto ast = p.get_ast();

                auto [nfa, alphabet] = nfa_gen.nfa_from_ast(ast, rule.rule_number, rule.no_utf8 || no_utf8);

                start_state->transitions[-1].insert(nfa);
                automata_alphabet.merge(alphabet);
            }

            dfa_generator dfa_gen(std::move(automata_alphabet));

            return dfa_gen.get_minimized_dfa(start_state);
        }

        ~generator() {
            for (auto state : nfa_states) {
                delete state;
            }
        }
    };

}

#endif //ALIEN_AUTOMATA_GENERATOR_H