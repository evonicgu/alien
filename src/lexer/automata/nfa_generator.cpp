#include "lexer/automata/nfa_generator.h"

namespace alien::lexer::automata {

    struct stack_element {
        regex::ast::node_ptr tree;
        nfa::simple_nfa curr_nfa;
        bool traversed1, traversed2;
        std::size_t ret_index;
    };

#define create_start_end nfa_states.push_back(std::unique_ptr<nfa::state>(new nfa::state{{}, false, rule_number})); \
    nfa_states.push_back(std::unique_ptr<nfa::state>(new nfa::state{{}, true, rule_number})); \
    auto* start = nfa_states[nfa_states.size() - 2].get(); \
    auto* end = nfa_states[nfa_states.size() - 1].get()

    std::pair<nfa::state*, std::unordered_set<util::u8char>>
    nfa_generator::nfa_from_ast(const regex::ast::node_ptr& ast, std::ptrdiff_t rule_number, bool no_utf8) {
        std::vector<stack_element> callstack{
                {nullptr, {}, false, false, 0},
                {ast, {}, false, false, 0}
        };

        std::unordered_set<util::u8char> alphabet;
        std::size_t current = 1;

        while (current != 0) {
            auto& elem = callstack[current];

            if (!elem.traversed1) {
                elem.traversed1 = true;

                switch (elem.tree->type) {
                    case regex::ast::node::node_type::CONCAT: {
                        auto* node = util::check<regex::ast::concat_node>(elem.tree.get());

                        callstack.push_back({node->first, {}, false, false, current});
                        ++current;
                        break;
                    }
                    case regex::ast::node::node_type::OR: {
                        auto* node = util::check<regex::ast::or_node>(elem.tree.get());

                        callstack.push_back({node->first, {}, false, false, current});

                        ++current;
                        break;
                    }
                    case regex::ast::node::node_type::STAR: {
                        auto* node = util::check<regex::ast::star_node>(elem.tree.get());

                        elem.traversed2 = true;
                        callstack.push_back({node->first, {}, false, false, current});

                        ++current;
                        break;
                    }
                    case regex::ast::node::node_type::LEAF: {
                        auto* node = util::check<regex::ast::leaf>(elem.tree.get());

                        create_start_end;

                        if (no_utf8 && node->symbol > 127) {
                            throw std::runtime_error("Unicode symbol used with 'noutf8' specified");
                        }

                        alphabet.insert(node->symbol);
                        start->transitions[node->symbol].insert(end);

                        elem.curr_nfa = {start, end};
                        current = elem.ret_index;
                        break;
                    }
                    case regex::ast::node::node_type::NEGATIVE_CLASS: {
                        auto* node = util::check<regex::ast::negative_class>(elem.tree.get());

                        create_start_end;

                        if (no_utf8) {
                            for (util::u8char c = 0; c < 128; ++c) {
                                if (node->negative_chars.find(c) != node->negative_chars.end()) {
                                    continue;
                                }

                                alphabet.insert(c);
                                start->transitions[c].insert(end);
                            }
                        } else {
                            for (util::u8char c = -33; c <= -3; ++c) {
                                alphabet.insert(c);
                                start->transitions[c].insert(end);
                            }
                        }

                        for (auto c : node->negative_chars) {
                            alphabet.insert(c);
                            start->transitions[c].insert(nullptr);
                        }

                        elem.curr_nfa = {start, end};
                        current = elem.ret_index;
                        break;
                    }
                }
            } else if (!elem.traversed2) {
                elem.traversed2 = true;

                switch (elem.tree->type) {
                    case regex::ast::node::node_type::OR: {
                        auto* node = util::check<regex::ast::or_node>(elem.tree.get());

                        callstack.push_back({node->second, {}, false, false, current});
                        current += 2;

                        break;
                    }
                    case regex::ast::node::node_type::CONCAT: {
                        auto* node = util::check<regex::ast::concat_node>(elem.tree.get());

                        callstack.push_back({node->second, {}, false, false, current});
                        current += 2;

                        break;
                    }
                }
            } else {
                switch (elem.tree->type) {
                    case regex::ast::node::node_type::OR: {
                        create_start_end;

                        auto& lhs = callstack[current + 1].curr_nfa;
                        auto& rhs = callstack[current + 2].curr_nfa;

                        lhs.end->accepting = false;
                        rhs.end->accepting = false;

                        start->transitions[-1] = {lhs.start, rhs.start};
                        lhs.end->transitions[-1] = {end};
                        rhs.end->transitions[-1] = {end};

                        elem.curr_nfa = {start, end};
                        callstack.erase(callstack.begin() + current + 1, callstack.end());
                        current = elem.ret_index;
                        break;
                    }
                    case regex::ast::node::node_type::CONCAT: {
                        auto& lhs = callstack[current + 1].curr_nfa;
                        auto& rhs = callstack[current + 2].curr_nfa;

                        lhs.end->accepting = false;
                        lhs.end->transitions = std::move(rhs.start->transitions);

                        elem.curr_nfa = {lhs.start, rhs.end};
                        callstack.erase(callstack.begin() + current + 1, callstack.end());
                        current = elem.ret_index;

                        break;
                    }
                    case regex::ast::node::node_type::STAR: {
                        create_start_end;

                        auto& quantified = callstack[current + 1].curr_nfa;
                        quantified.end->accepting = false;
                        quantified.end->transitions[-1] = {end, quantified.start};

                        start->transitions[-1] = {end, quantified.start};

                        elem.curr_nfa = {start, end};
                        callstack.erase(callstack.begin() + current + 1, callstack.end());
                        current = elem.ret_index;
                        break;
                    }
                }
            }
        }

        return {callstack[1].curr_nfa.start, alphabet};
    }

}