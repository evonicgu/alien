#include <string>
#include <numeric>
#include <stdexcept>

#include "gtest/gtest.h"

#include "util/u8string.h"

namespace alien::test {

    TEST(u8string_utils, test_ascii_to_u8string) {
        std::string input = "The quick brown fox jumps over 1azy lazy dog.";
        util::u8string expected = {84, 104, 101, 32, 113, 117, 105, 99, 107, 32, 98, 114, 111, 119, 110, 32, 102, 111, 120, 32, 106, 117, 109, 112, 115, 32, 111, 118, 101, 114, 32, 49, 97, 122, 121, 32, 108, 97, 122, 121, 32, 100, 111, 103, 46};

        EXPECT_EQ(util::ascii_to_u8string(input), expected);
    }

    TEST(u8string_utils, test_ascii_to_u8string_throws_on_invalid_input) {
        std::string input = "The quick brown fox jumps over 1azy lazy dog.";

        // Make the string invalid
        input[1] = -1;

        EXPECT_THROW(util::ascii_to_u8string(input), std::invalid_argument);
    }

    TEST(u8string_utils, test_user_defined_literal) {
        using namespace util::literals;

        std::string input = "The quick brown fox jumps over 1azy lazy dog.";

        EXPECT_EQ(util::ascii_to_u8string(input), "The quick brown fox jumps over 1azy lazy dog."_u8);
    }

    TEST(u8string_utils, test_u8string_to_bytes) {
        // Text: ⟰⮵⮏K⮞⺓ⓑ⻡☒❨Ⰿ⌞⤯⃺╊⹫⅊⮊⁽⊣⮈ⷕ⥄Ⳮ⊫⢘➓⢇ℊ₸▜ⷄ⇓⯪⺣⎻⋀☑⥈⩬⪾⁰⬥◠ⷮ⩣⋑⛈⭥⢩⼥⹾≊⊀⺷☘⭞⊒◫ⶱ⬖⨡⑫Ⅽ⎍⁠ⓔ✡⡑⓺❞⬡ⓐ⏉∽⢵⺰⬠⏧⍶⸲ⷉ⌠⫚ⳬ⴪ⴙ⹥⫕⍀⟕⒬◡ℝ▉⃈⣀⎬≖◝
        util::u8string text = {0x27f0, 0x2bb5, 0x2b8f, 0x212a, 0x2b9e, 0x2e93, 0x24d1, 0x2ee1, 0x2612, 0x2768, 0x2c0f, 0x231e, 0x292f, 0x20fa, 0x254a, 0x2e6b, 0x214a, 0x2b8a, 0x207d, 0x22a3, 0x2b88, 0x2dd5, 0x2944, 0x2ced, 0x22ab, 0x2898, 0x2793, 0x2887, 0x210a, 0x20b8, 0x259c, 0x2dc4, 0x21d3, 0x2bea, 0x2ea3, 0x23bb, 0x22c0, 0x2611, 0x2948, 0x2a6c, 0x2abe, 0x2070, 0x2b25, 0x25e0, 0x2dee, 0x2a63, 0x22d1, 0x26c8, 0x2b65, 0x28a9, 0x2f25, 0x2e7e, 0x224a, 0x2280, 0x2eb7, 0x2618, 0x2b5e, 0x2292, 0x25eb, 0x2db1, 0x2b16, 0x2a21, 0x246b, 0x216d, 0x238d, 0x2060, 0x24d4, 0x2721, 0x2851, 0x24fa, 0x275e, 0x2b21, 0x24d0, 0x23c9, 0x223d, 0x28b5, 0x2eb0, 0x2b20, 0x23e7, 0x2376, 0x2e32, 0x2dc9, 0x2320, 0x2ada, 0x2cec, 0x2d2a, 0x2d19, 0x2e65, 0x2ad5, 0x2340, 0x27d5, 0x24ac, 0x25e1, 0x211d, 0x2589, 0x20c8, 0x28c0, 0x23ac, 0x2256, 0x25dd};

        int bytes[] = {0xe2, 0x9f, 0xb0, 0xe2, 0xae, 0xb5, 0xe2, 0xae, 0x8f, 0xe2, 0x84, 0xaa, 0xe2, 0xae, 0x9e, 0xe2, 0xba, 0x93, 0xe2, 0x93, 0x91, 0xe2, 0xbb, 0xa1, 0xe2, 0x98, 0x92, 0xe2, 0x9d, 0xa8, 0xe2, 0xb0, 0x8f, 0xe2, 0x8c, 0x9e, 0xe2, 0xa4, 0xaf, 0xe2, 0x83, 0xba, 0xe2, 0x95, 0x8a, 0xe2, 0xb9, 0xab, 0xe2, 0x85, 0x8a, 0xe2, 0xae, 0x8a, 0xe2, 0x81, 0xbd, 0xe2, 0x8a, 0xa3, 0xe2, 0xae, 0x88, 0xe2, 0xb7, 0x95, 0xe2, 0xa5, 0x84, 0xe2, 0xb3, 0xad, 0xe2, 0x8a, 0xab, 0xe2, 0xa2, 0x98, 0xe2, 0x9e, 0x93, 0xe2, 0xa2, 0x87, 0xe2, 0x84, 0x8a, 0xe2, 0x82, 0xb8, 0xe2, 0x96, 0x9c, 0xe2, 0xb7, 0x84, 0xe2, 0x87, 0x93, 0xe2, 0xaf, 0xaa, 0xe2, 0xba, 0xa3, 0xe2, 0x8e, 0xbb, 0xe2, 0x8b, 0x80, 0xe2, 0x98, 0x91, 0xe2, 0xa5, 0x88, 0xe2, 0xa9, 0xac, 0xe2, 0xaa, 0xbe, 0xe2, 0x81, 0xb0, 0xe2, 0xac, 0xa5, 0xe2, 0x97, 0xa0, 0xe2, 0xb7, 0xae, 0xe2, 0xa9, 0xa3, 0xe2, 0x8b, 0x91, 0xe2, 0x9b, 0x88, 0xe2, 0xad, 0xa5, 0xe2, 0xa2, 0xa9, 0xe2, 0xbc, 0xa5, 0xe2, 0xb9, 0xbe, 0xe2, 0x89, 0x8a, 0xe2, 0x8a, 0x80, 0xe2, 0xba, 0xb7, 0xe2, 0x98, 0x98, 0xe2, 0xad, 0x9e, 0xe2, 0x8a, 0x92, 0xe2, 0x97, 0xab, 0xe2, 0xb6, 0xb1, 0xe2, 0xac, 0x96, 0xe2, 0xa8, 0xa1, 0xe2, 0x91, 0xab, 0xe2, 0x85, 0xad, 0xe2, 0x8e, 0x8d, 0xe2, 0x81, 0xa0, 0xe2, 0x93, 0x94, 0xe2, 0x9c, 0xa1, 0xe2, 0xa1, 0x91, 0xe2, 0x93, 0xba, 0xe2, 0x9d, 0x9e, 0xe2, 0xac, 0xa1, 0xe2, 0x93, 0x90, 0xe2, 0x8f, 0x89, 0xe2, 0x88, 0xbd, 0xe2, 0xa2, 0xb5, 0xe2, 0xba, 0xb0, 0xe2, 0xac, 0xa0, 0xe2, 0x8f, 0xa7, 0xe2, 0x8d, 0xb6, 0xe2, 0xb8, 0xb2, 0xe2, 0xb7, 0x89, 0xe2, 0x8c, 0xa0, 0xe2, 0xab, 0x9a, 0xe2, 0xb3, 0xac, 0xe2, 0xb4, 0xaa, 0xe2, 0xb4, 0x99, 0xe2, 0xb9, 0xa5, 0xe2, 0xab, 0x95, 0xe2, 0x8d, 0x80, 0xe2, 0x9f, 0x95, 0xe2, 0x92, 0xac, 0xe2, 0x97, 0xa1, 0xe2, 0x84, 0x9d, 0xe2, 0x96, 0x89, 0xe2, 0x83, 0x88, 0xe2, 0xa3, 0x80, 0xe2, 0x8e, 0xac, 0xe2, 0x89, 0x96, 0xe2, 0x97, 0x9d};

        std::string bytes_str{std::begin(bytes), std::end(bytes)};

        EXPECT_EQ(util::u8string_to_bytes(text), bytes_str);

        std::string out_str;
        util::u8string_to_bytes(text, out_str);

        EXPECT_EQ(out_str, bytes_str);

        // Make input invalid
        text[1] = std::numeric_limits<util::u8char>::max();

        EXPECT_THROW(util::u8string_to_bytes(text), std::invalid_argument);
    }

    TEST(u8string_utils, test_u8_stoi) {
        using namespace util::literals;

        EXPECT_EQ(util::u8_stoi("1234"_u8), 1234);

        EXPECT_EQ(util::u8_stoi("-1234"_u8), -1234);

        EXPECT_EQ(util::u8_stoi("123456789"_u8), 123456789);

        EXPECT_THROW(util::u8_stoi({}), std::invalid_argument);

        EXPECT_THROW(util::u8_stoi("abcdef"_u8), std::invalid_argument);
    }

    TEST(u8string_utils, test_to_u8string) {
        // Currently unsigned long long only

        using namespace util::literals;

        EXPECT_EQ(util::to_u8string(1234), "1234"_u8);

        EXPECT_EQ(util::to_u8string(123456789), "123456789"_u8);

        EXPECT_EQ(util::to_u8string(0), "0"_u8);
    }

}