#ifndef ALIEN_UTIL_PARSER_H
#define ALIEN_UTIL_PARSER_H

#include <list>
#include <stdexcept>

#include "lexer.h"
#include "token.h"
#include "util/typeutils.h"
#include "util/u8string.h"

namespace alien::util {

    template<typename T>
    class parser {
        void initialize() {
            lookahead = l.lex();
        }

    protected:
        using lexer_t = lexer<T>;
        using token_t = token<T>;
        using type = T;

        lexer_t& l;
        token_t* lookahead;

        std::list<util::u8string>& err;

    public:
        explicit parser(lexer_t& l, std::list<util::u8string>& err)
            : l(l),
              err(err) {
            initialize();
        }

        virtual void parse() = 0;

    protected:
        virtual void error(T expected, T got) = 0;

        void match(T type) {
            if (lookahead->type == type) {
                delete lookahead;

                // in case l.lex() throws an exception
                lookahead = nullptr;
                lookahead = l.lex();
                return;
            }

            error(type, lookahead->type);
        }

        template<typename To>
        To* check() {
            return util::check<To, token_t>(lookahead);
        }

        virtual ~parser() {
            delete lookahead;
        }
    };

}

#endif //ALIEN_UTIL_PARSER_H