#ifndef ALIEN_TRANSITION_TABLE_H
#define ALIEN_TRANSITION_TABLE_H

namespace alien::lexer::automata {

    struct state_data {
        bool accepting;
        std::ptrdiff_t rule_number;

        std::map<std::size_t, std::unordered_set<util::u8char>> transitions;
    };

    struct transition_table {
        std::size_t total_states;

        std::vector<state_data> states;
    };

}

#endif //ALIEN_TRANSITION_TABLE_H