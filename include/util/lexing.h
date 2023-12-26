#ifndef ALIEN_LEXING_H
#define ALIEN_LEXING_H

#include <utility>

#include "input/input.h"
#include "u8string.h"

namespace alien::util {

    std::pair<u8string, bool> get_code_block(input::input& i);

    bool is_start_identifier_char(u8char c);

    bool is_continuation_identifier_char(util::u8char c);

    u8string get_identifier(input::input& i, u8char first);

    u8string get_identifier(input::input& i);

    short xdigit_to_num(u8char c);

    int hex_to_codepoint(input::input& input, unsigned short size);

    u8char parse_escape(input::input& i);

}

#endif //ALIEN_LEXING_H