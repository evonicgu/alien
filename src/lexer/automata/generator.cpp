#include "lexer/automata/generator.h"

#include <unordered_set>

#include "lexer/automata/nfa_generator.h"
#include "input/input.h"
#include "lexer/regex/lexer.h"
#include "lexer/regex/parser.h"
#include "lexer/automata/dfa_generator.h"

namespace alien::lexer::automata {

    dfa::dfa generator::generate_automata(std::vector<rules::rule>& rules) {
        auto* start_state = new nfa::state{{}, false, -1};
        std::unordered_set<util::u8char> automata_alphabet;

        nfa_generator nfa_gen(nfa_states);

        for (auto& rule : rules) {
            input::string_input i(rule.regex);

            i.set_pos(rule.position);

            regex::lexer l(i, err);
            regex::parser p(l, err, rule.no_utf8 || no_utf8);

            p.parse();

            auto ast = p.get_ast();

            auto [nfa, alphabet] = nfa_gen.nfa_from_ast(ast, rule.rule_number, rule.no_utf8 || no_utf8);

            start_state->transitions[-1].insert(nfa);
            automata_alphabet.merge(alphabet);
        }

        auto result = get_minimized_dfa(start_state, std::move(automata_alphabet));

        delete start_state;

        return result;
    }

}