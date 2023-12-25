#include "gtest/gtest.h"

#include "util/charutils.h"

namespace alien::test {

    class char_utils_get_class_test_fixture :
            public ::testing::TestWithParam<std::pair<util::u8char, util::u8char>> {};

    TEST_P(char_utils_get_class_test_fixture, test_get_class) {
        auto [expected_class, value] = GetParam();

        EXPECT_EQ(util::get_class(value), expected_class);
    }

    std::pair<util::u8char, util::u8char> get_class_values[] = {
            {-1, -1},
            // Uppercase Letter
            {-12, 0x42},   // 'B'
            // Lowercase Letter
            {-8, 0x61},    // 'a'
            // Titlecase Letter
            {-11, 0x1C5},  // 'ǅ'
            // Modifier Letter
            {-9, 0x2B0},   // 'ʰ'
            // Other Letter
            {-10, 0x1C1},  // 'ǁ'
            // Nonspacing Mark
            {-15, 0x32D},  // '◌̭'
            // Spacing Mark
            {-13, 0x903},  // 'ः'
            // Enclosing Mark
            {-14, 0x489},  // '҉'
            // Decimal Number
            {-16, 0x30},   // '0'
            // Letter Number
            {-17, 0x16F0}, // 'ᛰ'
            // Other Number
            {-18, 0xBD},   // '½'
            // Connector Punctuation
            {-19, 0x203F}, // '‿'
            // Underscore
            {-20, '_'},
            // Dash Punctuation
            {-21, 0x2D},   // '-'
            // Open Punctuation
            {-26, 0x28},   // '('
            // Close Punctuation
            {-22, 0x29},   // ')'
            // Initial Punctuation
            {-24, 0xAB},   // '«'
            // Final Punctuation
            {-23, 0xBB},   // '»'
            // Other Punctuation
            {-25, 0x21},   // '!'
            // Math Symbol
            {-29, 0x2B},   // '+'
            // Currency Symbol
            {-27, 0x24},   // '$'
            // Modifier Symbol
            {-28, 0x5E},   // '^'
            // Other Symbol
            {-30, 0xA6},   // '¦'
            // Space Separator
            {-33, 0x20},   // ' '
            // Line Separator
            {-31, 0x2028}, // ' '
            // Paragraph Separator
            {-32, 0x2029}, // ' '
            // Tab
            {-4, '\t'},
            // Carriage Return
            {-4, '\r'},
            // Newline
            {-5, '\n'},
            // Vertical Space
            {-5, '\v'},
            // Form Feed
            {-5, '\f'},
            // NEL
            {-5, 0x85},    // 'NEL'
            // Control
            {-3, 0x00},    // '\0'
            // MVS (Mongolian Vowel Separator)
            {-7, 0x180e},  // 'MVS'
            // Format
            {-6, 0xAD}    // 'SHY'
    };

    INSTANTIATE_TEST_SUITE_P(char_utils_get_class_values,
                             char_utils_get_class_test_fixture,
                             testing::ValuesIn(get_class_values));

    TEST(char_utils, test_get_class_throws) {
        EXPECT_THROW(util::get_class(0x05FF), std::invalid_argument); // Unassigned code point
    }

    class char_utils_isspace_fixture :
            public ::testing::TestWithParam<std::pair<bool, util::u8char>> {};

    TEST_P(char_utils_isspace_fixture, test_isspace) {
        auto [expected, value] = GetParam();

        EXPECT_EQ(util::isspace(value), expected);
    }

    std::pair<bool, util::u8char> isspace_values[] = {
            {true,  -4},
            {true,  '\t'},
            {true,  '\r'},
            {true,  '\n'},
            {true,  '\v'},
            {true,  '\f'},
            {true,  0x85},
            {true,  0x180e},
            {true,  0x2028},
            {true,  0x2029},
            {true,  ' '},
            {false, 'a'},
            {false, 0xAD},
            {false, 0x28}
    };

    INSTANTIATE_TEST_SUITE_P(char_utils_isspace_values,
                             char_utils_isspace_fixture,
                             ::testing::ValuesIn(isspace_values));

}