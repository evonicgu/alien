#ifndef ALIEN_REGEX_PARSER_H
#define ALIEN_REGEX_PARSER_H

#include <list>
#include <memory>

#include "ast.h"
#include "charclasses.h"
#include "lexer.h"
#include "quantifier.h"
#include "token.h"
#include "util/parser.h"
#include "util/u8string.h"

namespace alien::lexer::regex {

    class parser : util::parser<token_type> {
        std::size_t fold_level = 0;

        ast::node_ptr ast;

        bool no_utf8;

    public:
        parser(lexer_t& l, std::list<util::u8string>& err, bool no_utf8)
            : util::parser<token_type>(l, err),
              no_utf8(no_utf8) {}

        void parse() override;

        ast::node_ptr get_ast();

    private:
        void error(token_type expected, token_type got) override;

        ast::node_ptr regex();

        static bool check_lookahead(type t);

        ast::node_ptr factors();

        ast::node_ptr factor();

        ast::node_ptr base();

        ast::node_ptr char_class();

        std::unordered_set<util::u8char> class_characters();

        ast::node_ptr shortcut();

        std::unordered_set<util::u8char> symbol_class(const util::u8string& classname);

        static std::unordered_set<util::u8char> simple_symbol_class(const util::u8string& classname);

        static std::unordered_set<util::u8char> composed_symbol_class(const util::u8string& classname);

        util::u8char get_char();

        std::size_t get_number();

        static ast::node_ptr ast_from_set(const std::unordered_set<util::u8char>& charset);
    };

}

#endif //ALIEN_REGEX_PARSER_H