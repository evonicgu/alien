#include "util/charutils.h"

#include <stdexcept>

#include "utf8proc.h"

namespace alien::util {

    u8char get_class(u8char c) {
        if (c < 0) {
            return c;
        }

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
                if (c == '\t' || c == '\r') {
                    return -4;
                }

                if (c == '\n' || c == '\v' || c == '\f' || c == 133) { // 133 - u+85, NEL
                    return -5;
                }

                return -3;
            case UTF8PROC_CATEGORY_CF:
                if (c == 6158) { // 6158 - u+180e, Mongolian Vowel Separator (MVS)
                    return -7;
                }

                return -6;
            default:
                throw std::invalid_argument("Invalid utf-8 character category");
        }
    }

    bool isspace(u8char c) {
        u8char cc = get_class(c);

        if (cc == -4 || cc == -5 || cc == -7 || cc == -31 || cc == -32 || cc == -33) {
            return true;
        }

        return false;
    }

}