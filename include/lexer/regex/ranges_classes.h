#ifndef ALIEN_RANGES_CLASSES_H
#define ALIEN_RANGES_CLASSES_H

#include <set>
#include <stdexcept>
#include "lexer/regex/lexer.h"
#include "util/u8string.h"

namespace alien::lexer::regex::ranges {

    using set = std::set<util::u8char>;

    using namespace util::literals;

    set full = {
            -3, -4, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26,
            -27, -28, -29, -30, -33
    };

    set h_space = {
            -4, -7, -33
    };

    set non_h_space = {
            -3, -5, -6, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26,
            -27, -28, -29, -30, -31, -32
    };

    set valid_sequence = {
            -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, -25,
            -26, -27, -28, -29, -30, -31, -32, -33
    };

    set unicode_newline = {
            -5, -31, -32
    };

    set space = {
            -4, -5, -7, -31, -32, -33
    };

    set non_space = {
            -3, -6, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26, -27,
            -28, -29, -30
    };

    set non_digit = {
            -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26,
            -27, -28, -29, -30, -31, -32, -33
    };

    set non_newline = {
            -3, -4, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26,
            -27, -28, -29, -30, -33
    };

    set v_space = {
            -5, -31, -32,
    };

    set non_v_space = {
            -3, -4, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26,
            -27, -28, -29, -30, -33
    };

    set word_char = {
            -8, -9, -10, -11, -12, -16, -17, -18, -20
    };

    set non_word_char = {
            -3, -4, -5, -6, -7, -13, -14, -15, -19, -21, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31, -32, -33
    };

    util::u8char get_class(util::u8char c) {
        switch (utf8proc_category(c)) {
            case UTF8PROC_CATEGORY_CN:
                throw std::invalid_argument("Codepoint must be assigned");
            case UTF8PROC_CATEGORY_LU:
                return -12;
            case UTF8PROC_CATEGORY_LL:
                return -8;
            case UTF8PROC_CATEGORY_LT:
                return -11;
            case UTF8PROC_CATEGORY_LM:
                return -9;
            case UTF8PROC_CATEGORY_LO:
                return -10;
            case UTF8PROC_CATEGORY_MN:
                return -15;
            case UTF8PROC_CATEGORY_MC:
                return -13;
            case UTF8PROC_CATEGORY_ME:
                return -14;
            case UTF8PROC_CATEGORY_ND:
                return -16;
            case UTF8PROC_CATEGORY_NL:
                return -17;
            case UTF8PROC_CATEGORY_NO:
                return -18;
            case UTF8PROC_CATEGORY_PC:
                if (c == '_') {
                    return -20;
                }

                return -19;
            case UTF8PROC_CATEGORY_PD:
                return -21;
            case UTF8PROC_CATEGORY_PS:
                return -26;
            case UTF8PROC_CATEGORY_PE:
                return -22;
            case UTF8PROC_CATEGORY_PI:
                return -24;
            case UTF8PROC_CATEGORY_PF:
                return -23;
            case UTF8PROC_CATEGORY_PO:
                return -25;
            case UTF8PROC_CATEGORY_SM:
                return -29;
            case UTF8PROC_CATEGORY_SC:
                return -27;
            case UTF8PROC_CATEGORY_SK:
                return -28;
            case UTF8PROC_CATEGORY_SO:
                return -30;
            case UTF8PROC_CATEGORY_ZS:
                return -33;
            case UTF8PROC_CATEGORY_ZL:
                return -31;
            case UTF8PROC_CATEGORY_ZP:
                return -32;
            case UTF8PROC_CATEGORY_CC:
                if (c == '\t') {
                    return -4;
                }

                if (c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == 133) { // 133 - u+85, NEL
                    return -5;
                }

                return -3;
            case UTF8PROC_CATEGORY_CF:
                if (c == 6158) { // 6158 - u+180e, Mongolian Vowel Separator (MVS)
                    return -7;
                }

                return -6;
            case UTF8PROC_CATEGORY_CS:
            case UTF8PROC_CATEGORY_CO:
                throw std::invalid_argument("Invalid character category");
        }
    }

    set get_classes_by_name(const util::u8string& name) {
        using exception = alien::lexer::regex::lexer::lexer::class_name_exception;

        if (name.size() == 1) {
            switch (name[0]) {
                case 'c':
                    return {-3, -4, -5, -6, -7};
                case 'l':
                    return {-8, -9, -10, -11, -12};
                case 'm':
                    return {-13, -14, -15};
                case 'n':
                    return {-16, -17, -18};
                case 'p':
                    return {-19, -20, -21, -22, -23, -24, -25, -26};
                case 's':
                    return {-27, -28, -29, -30};
                case 'z':
                    return {-31, -32, -33};
            }

            throw exception();
        }

        switch (name[0]) {
            case 'c':
                switch (name[1]) {
                    case 'c':
                        return {-3, -4, -5};
                    case 'f':
                        return {-6, -7};
                }
                break;
            case 'l':
                switch (name[1]) {
                    case 'l':
                        return {-8};
                    case 'm':
                        return {-9};
                    case 'o':
                        return {-10};
                    case 't':
                        return {-11};
                    case 'u':
                        return {-12};
                }
                break;
            case 'm':
                switch (name[1]) {
                    case 'c':
                        return {-13};
                    case 'e':
                        return {-14};
                    case 'n':
                        return {-15};
                }
                break;
            case 'n':
                switch (name[1]) {
                    case 'd':
                        return {-16};
                    case 'l':
                        return {-17};
                    case 'o':
                        return {-18};
                }
                break;
            case 'p':
                switch (name[1]) {
                    case 'c':
                        return {-19, -20};
                    case 'd':
                        return {-21};
                    case 'e':
                        return {-22};
                    case 'f':
                        return {-23};
                    case 'i':
                        return {-24};
                    case 'o':
                        return {-25};
                    case 's':
                        return {-26};
                }
                break;
            case 's':
                switch (name[1]) {
                    case 'c':
                        return {-27};
                    case 'k':
                        return {-28};
                    case 'm':
                        return {-29};
                    case 'o':
                        return {-30};
                }
                break;
            case 'z':
                switch (name[1]) {
                    case 'l':
                        return {-31};
                    case 'p':
                        return {-32};
                    case 's':
                        return {-33};
                }
                break;
        }

        throw exception();
    }

}

#endif //ALIEN_RANGES_CLASSES_H