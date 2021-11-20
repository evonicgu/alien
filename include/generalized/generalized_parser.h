#ifndef ALIEN_GENERALIZED_PARSER_H
#define ALIEN_GENERALIZED_PARSER_H

#include "generalized_exception.h"
#include "generalized_lexer.h"
#include "generalized_token.h"

namespace alien::generalized {

    static constexpr const char syntax_exception_str[] = "Syntax error. ";
    static constexpr const char token_type_exception_str[] = "Wrong token type. ";

    using namespace util::literals;

    template<typename T1, typename T2>
    class generalized_parser {
    protected:
        using lex = T2;
        using tok = generalized_token<T1>;
        using type = T1;

        lex l;
        tok* lookahead;

        void initialize() {
            lookahead = l.lex();
        }

    public:
        explicit generalized_parser(const lex& l) : l(l) {
            initialize();
        }

        explicit generalized_parser(lex&& l) : l(std::move(l)) {
            initialize();
        }

        virtual void parse() = 0;

        using syntax_exception = generalized_exception<syntax_exception_str>;
        using token_type_exception = generalized_exception<token_type_exception_str>;

    protected:
        void match(T1 type) {
            if (lookahead->type == type) {
                delete lookahead;

                lookahead = l.lex();
                return;
            }

            throw syntax_exception("Unexpected token"_u8);
        }

        template<typename To, typename... Args>
        To* check(Args&&... args) {
            return check<To, token_type_exception, tok, Args...>(lookahead, std::forward<Args>(args)...);
        }

        template<typename To, typename Ex, typename From, typename... Args>
        To* check(From* from, Args&&... args) {
            auto* casted = dynamic_cast<To*>(from);

            if (casted == nullptr) {
                throw Ex(args...);
            }

            return casted;
        }

        virtual ~generalized_parser() {
            delete lookahead;
        }
    };

}

#endif //ALIEN_GENERALIZED_PARSER_H