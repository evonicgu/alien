#include "lexer/automata/nfa_generator.h"

namespace alien::lexer::automata {

    std::pair<nfa::state*, std::unordered_set<util::u8char>>
    nfa_generator::nfa_from_ast(const regex::ast::node_ptr& ast, std::ptrdiff_t rule_number, bool no_utf8) {
        using stack_element = std::tuple<regex::ast::node_ptr, nfa::simple_nfa, bool, bool, std::size_t>;

        std::vector<stack_element> callstack{
                {nullptr, {}, false, false, 0},
                {ast, {}, false, false, 0}
        };

        std::unordered_set<util::u8char> alphabet;
        std::size_t current = 1;

        while (current != 0) {
            auto& [tree, curr_nfa, traversed1, traversed2, ret_index] = callstack[current];

            if (!traversed1) {
                traversed1 = true;

                switch (tree->type) {
                    case regex::ast::node::node_type::CONCAT: {
                        auto* node = util::check<regex::ast::concat_node>(tree.get());

                        callstack.push_back({node->first, {}, false, false, current});
                        ++current;
                        break;
                    }
                    case regex::ast::node::node_type::OR: {
                        auto* node = util::check<regex::ast::or_node>(tree.get());

                        callstack.push_back({node->first, {}, false, false, current});

                        ++current;
                        break;
                    }
                    case regex::ast::node::node_type::STAR: {
                        auto* node = util::check<regex::ast::star_node>(tree.get());

                        traversed2 = true;
                        callstack.push_back({node->first, {}, false, false, current});

                        ++current;
                        break;
                    }
                    case regex::ast::node::node_type::LEAF: {
                        auto* node = util::check<regex::ast::leaf>(tree.get());

                        auto* start = new nfa::state{{}, false, rule_number};
                        auto* end = new nfa::state{{}, true, rule_number};

                        if (no_utf8 && node->symbol > 127) {
                            throw std::runtime_error("Unicode symbol used with 'noutf8' specified");
                        }

                        alphabet.insert(node->symbol);
                        start->transitions[node->symbol].insert(end);

                        curr_nfa = {start, end};
                        current = ret_index;

                        nfa_states.push_back(start);
                        nfa_states.push_back(end);
                        break;
                    }
                    case regex::ast::node::node_type::NEGATIVE_CLASS: {
                        auto* node = util::check<regex::ast::negative_class>(tree.get());

                        auto* start = new nfa::state{{}, false, rule_number};
                        auto* end = new nfa::state{{}, true, rule_number};

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

                        curr_nfa = {start, end};
                        current = ret_index;

                        nfa_states.push_back(start);
                        nfa_states.push_back(end);
                        break;
                    }
                }
            } else if (!traversed2) {
                traversed2 = true;

                switch (tree->type) {
                    case regex::ast::node::node_type::OR: {
                        auto* node = util::check<regex::ast::or_node>(tree.get());

                        callstack.push_back({node->second, {}, false, false, current});
                        current += 2;

                        break;
                    }
                    case regex::ast::node::node_type::CONCAT: {
                        auto* node = util::check<regex::ast::concat_node>(tree.get());

                        callstack.push_back({node->second, {}, false, false, current});
                        current += 2;

                        break;
                    }
                }
            } else {
                switch (tree->type) {
                    case regex::ast::node::node_type::OR: {
                        auto* start = new nfa::state{{}, false, rule_number};
                        auto* end = new nfa::state{{}, true, rule_number};

                        auto& lhs = std::get<1>(callstack[current + 1]);
                        auto& rhs = std::get<1>(callstack[current + 2]);

                        lhs.end->accepting = false;
                        rhs.end->accepting = false;

                        start->transitions[-1] = {lhs.start, rhs.start};
                        lhs.end->transitions[-1] = {end};
                        rhs.end->transitions[-1] = {end};

                        curr_nfa = {start, end};
                        callstack.erase(callstack.begin() + current + 1, callstack.end());
                        current = ret_index;

                        nfa_states.push_back(start);
                        nfa_states.push_back(end);
                        break;
                    }
                    case regex::ast::node::node_type::CONCAT: {
                        auto& lhs = std::get<1>(callstack[current + 1]);
                        auto& rhs = std::get<1>(callstack[current + 2]);

                        lhs.end->accepting = false;
                        lhs.end->transitions = std::move(rhs.start->transitions);

                        curr_nfa = {lhs.start, rhs.end};
                        callstack.erase(callstack.begin() + current + 1, callstack.end());
                        current = ret_index;

                        break;
                    }
                    case regex::ast::node::node_type::STAR: {
                        auto* start = new nfa::state{{}, false, rule_number};
                        auto* end = new nfa::state{{}, true, rule_number};

                        auto& quantified = std::get<1>(callstack[current + 1]);
                        quantified.end->accepting = false;
                        quantified.end->transitions[-1] = {end, quantified.start};

                        start->transitions[-1] = {end, quantified.start};

                        curr_nfa = {start, end};
                        callstack.erase(callstack.begin() + current + 1, callstack.end());
                        current = ret_index;

                        nfa_states.push_back(start);
                        nfa_states.push_back(end);
                        break;
                    }
                }
            }
        }

        return {std::get<1>(callstack[1]).start, alphabet};
    }

}