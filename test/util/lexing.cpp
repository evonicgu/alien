#include <utility>

#include "gtest/gtest.h"

#include "input/input.h"
#include "util/lexing.h"
#include "util/u8string.h"

namespace alien::test {

    using namespace util::literals;

    class lexing_utils_xdigit_to_num_test_fixture :
            public ::testing::TestWithParam<std::pair<short, util::u8char>> {};

    TEST_P(lexing_utils_xdigit_to_num_test_fixture, test_xdigit_to_num) {
        auto [expected, value] = GetParam();

        EXPECT_EQ(util::xdigit_to_num(value), expected);
    }

    std::pair<short, util::u8char> test_xdigit_to_num_values[] = {
            {0, '0'},
            {1, '1'},
            {2, '2'},
            {3, '3'},
            {4, '4'},
            {5, '5'},
            {6, '6'},
            {7, '7'},
            {8, '8'},
            {9, '9'},
            {10, 'a'},
            {10, 'A'},
            {11, 'b'},
            {11, 'B'},
            {12, 'c'},
            {12, 'C'},
            {13, 'd'},
            {13, 'D'},
            {14, 'e'},
            {14, 'E'},
            {15, 'f'},
            {15, 'F'},
            {-1, 'o'},
            {-1, 0x2028}
    };

    INSTANTIATE_TEST_SUITE_P(lexing_utils_xdigit_to_num_values,
                             lexing_utils_xdigit_to_num_test_fixture,
                             ::testing::ValuesIn(test_xdigit_to_num_values));

    class lexing_utils_parse_escape_test_fixture :
            public ::testing::TestWithParam<std::pair<util::u8char, input::string_input>> {};

    TEST_P(lexing_utils_parse_escape_test_fixture, test_parse_escape) {
        auto [expected, value] = GetParam();

        EXPECT_EQ(util::parse_escape(value), expected);
    }

    std::pair<util::u8char, input::string_input> test_parse_escape_values[] = {
            {'\'',       input::string_input{"\'"_u8}},
            {'"',        input::string_input{"\""_u8}},
            {'?',        input::string_input{"?"_u8}},
            {'?',        input::string_input{"?"_u8}},
            {'\\',       input::string_input{"\\"_u8}},
            {'\a',       input::string_input{"a"_u8}},
            {'\b',       input::string_input{"b"_u8}},
            {'\f',       input::string_input{"f"_u8}},
            {'\n',       input::string_input{"n"_u8}},
            {'\r',       input::string_input{"r"_u8}},
            {'\t',       input::string_input{"t"_u8}},
            {'\v',       input::string_input{"v"_u8}},
            {-2,         input::string_input{"m"_u8}},
            {0x15,       input::string_input{"X15"_u8}},
            {0x2028,     input::string_input{"u2028"_u8}},
            {0x12043,    input::string_input{"U00012043"_u8}},
            {-1,         input::string_input{"U12043"_u8}},
            {-1,         input::string_input{"Uffffffff"_u8}}
    };

    INSTANTIATE_TEST_SUITE_P(lexing_utils_parse_escape_values,
                             lexing_utils_parse_escape_test_fixture,
                             ::testing::ValuesIn(test_parse_escape_values));

    class lexing_utils_get_identifier_first_test_fixture :
            public ::testing::TestWithParam<std::tuple<util::u8string, input::string_input, util::u8char>> {};

    TEST_P(lexing_utils_get_identifier_first_test_fixture, test_get_identifier_first) {
        auto [expected, input, first] = GetParam();

        EXPECT_EQ(util::get_identifier(input, first), expected);
    }

    std::tuple<util::u8string, input::string_input, util::u8char> test_get_identifier_first_values[] = {
            {{},              input::string_input{"valid"_u8}, '0'},
            {"$123valid_0"_u8,  input::string_input{"123valid_0 other"_u8}, '$'}
    };

    INSTANTIATE_TEST_SUITE_P(lexing_utils_get_identifier_first_values,
                             lexing_utils_get_identifier_first_test_fixture,
                             ::testing::ValuesIn(test_get_identifier_first_values));

    class lexing_utils_get_identifier_test_fixture :
            public ::testing::TestWithParam<std::pair<util::u8string, input::string_input>> {};

    TEST_P(lexing_utils_get_identifier_test_fixture, test_get_identifier) {
        auto [expected, input] = GetParam();

        EXPECT_EQ(util::get_identifier(input), expected);
    }

    std::pair<util::u8string, input::string_input> test_get_identifier_values[] = {
            {{},           input::string_input{"0valid"_u8}},
            {"$123valid_0"_u8,  input::string_input{"$123valid_0 other"_u8}}
    };

    INSTANTIATE_TEST_SUITE_P(lexing_utils_get_identifier_values,
                             lexing_utils_get_identifier_test_fixture,
                             ::testing::ValuesIn(test_get_identifier_values));

    class lexing_utils_get_code_block_test_fixture :
        public ::testing::TestWithParam<std::pair<std::pair<util::u8string, bool>, input::string_input>> {};

    TEST_P(lexing_utils_get_code_block_test_fixture, test_get_code_block) {
        auto [expected, input] = GetParam();

        EXPECT_EQ(util::get_code_block(input), expected);
    }

    std::pair<std::pair<util::u8string, bool>, input::string_input> test_get_code_block_values[] = {
            {{"some text "_u8, false}, input::string_input{"some text }"_u8}},
            {{"//some text }\n"_u8, false}, input::string_input{"//some text }\n}"_u8}},
            {{"//some text }"_u8, true}, input::string_input{"//some text }"_u8}},
            {{"'}'\"}}}}}}\"{{}}"_u8, false}, input::string_input{"'}'\"}}}}}}\"{{}}}"_u8}},
            {{"'}'\"}}}}}}\"{{}}"_u8, true}, input::string_input{"'}'\"}}}}}}\"{{}}"_u8}},
            {{"'\\}''\"'\"\\\"}\""_u8, false}, input::string_input{"'\\}''\"'\"\\\"}\"}"_u8}},
            {{"'\\}''\"'\"\\\"}\""_u8, true}, input::string_input{"'\\}''\"'\"\\\"}\""_u8}},
            {{{}, true}, input::string_input{{}}},
            {{"/**\n}}}}}}}\n}\n}\n}\n}///**{{{}}}*/"_u8, false}, input::string_input{"/**\n}}}}}}}\n}\n}\n}\n}///**{{{}}}*/}"_u8}},
            {{"/**\n}}}}}}}\n}\n}\n}\n}///**{{{}}}*/"_u8, true}, input::string_input{"/**\n}}}}}}}\n}\n}\n}\n}///**{{{}}}*/"_u8}}
    };

    INSTANTIATE_TEST_SUITE_P(lexing_utils_get_code_block_values,
                             lexing_utils_get_code_block_test_fixture,
                             ::testing::ValuesIn(test_get_code_block_values));
}