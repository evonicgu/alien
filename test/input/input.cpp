#include "gtest/gtest.h"

#include <memory>
#include <sstream>

#include "input/input.h"
#include "util/token.h"
#include "util/u8string.h"

namespace alien::test {

    TEST(input_tests, string_input_get_test) {
        using namespace util::literals;

        input::string_input in{"so\n"_u8};

        EXPECT_EQ(in.get(), 's');
        EXPECT_EQ(in.get(), 'o');
        EXPECT_EQ(in.get(), '\n');

        EXPECT_EQ(in.get(), -2);
    }

    TEST(input_tests, string_input_peek_test) {
        using namespace util::literals;

        input::string_input in{"so\n"_u8};

        EXPECT_EQ(in.peek(), 's');
        EXPECT_EQ(in.peek(), 's');
        EXPECT_EQ(in.peek(), 's');

        in.get(), in.get(), in.get();

        EXPECT_EQ(in.peek(), -2);
    }

    TEST(input_tests, input_get_pos_test) {
        using namespace util::literals;

        input::string_input in{"so\n"_u8};

        EXPECT_EQ(in.get_pos(), (util::pos{1, 1}));

        in.peek();

        EXPECT_EQ(in.get_pos(), (util::pos{1, 1}));

        in.get();

        EXPECT_EQ(in.get_pos(), (util::pos{1, 2}));

        in.get(), in.get();

        EXPECT_EQ(in.get_pos(), (util::pos{2, 1}));
    }

    TEST(input_tests, input_set_pos_test) {
        using namespace util::literals;

        input::string_input in{"so\n"_u8};

        // pretend like we start from line 5
        in.set_pos({5, 1});

        in.get(), in.get();

        EXPECT_EQ(in.get_pos(), (util::pos{5, 3}));
    }


    TEST(input_tests, stream_input_get_test) {
        input::stream_input i(std::make_unique<std::stringstream>("so\n"));

        EXPECT_EQ(i.get(), 's');
        EXPECT_EQ(i.get(), 'o');
        EXPECT_EQ(i.get(), '\n');

        EXPECT_EQ(i.get(), -2);
    }

    TEST(input_tests, stream_input_unicode_get_ascii_anchor_test) {
        // Stream buffer size is 32771 characters, so we make the buffer
        // of 8191 unicode 4 byte characters, then one 3-byte character,
        // then one ascii character, then one 4-byte character again,
        // which is 32773, so the
        // stream_input instance will be forced to evaluate the contents
        // of the buffer in order to determine which characters are fully
        // read, and which characters are to be left unread (after the
        // buffer was filled this time)

        std::string data;
        data.reserve(4 * 8193 + 1); // 8193 4-byte unicode characters

        const std::string char_4byte = "\U00012043";
        const std::string char_3byte = "\U00000986";
        const int char3_code = 2438;
        const int char4_code = 73795;


        for (int i = 0; i < 8191; ++i) {
            data += char_4byte;
        }

        data += char_3byte;

        data += 'a';
        data += char_4byte;

        input::stream_input in(std::make_unique<std::stringstream>(std::move(data)));

        for (int i = 0; i < 8191; ++i) {
            EXPECT_EQ(in.get(), char4_code);
        }

        EXPECT_EQ(in.get(), char3_code);

        EXPECT_EQ(in.get(), 'a');
        EXPECT_EQ(in.get(), char4_code);
    }

    TEST(input_tests, stream_input_unicode_get_unicode_anchor_test) {
        // Stream buffer size is 32771 characters, so we make the buffer
        // of 8191 unicode 4 byte characters, then one 3-byte character,
        // then one ascii character, then one 4-byte character again,
        // which is 32773, so the
        // stream_input instance will be forced to evaluate the contents
        // of the buffer in order to determine which characters are fully
        // read, and which characters are to be left unread (after the
        // buffer was filled this time)

        std::string data;
        data.reserve(4 * 8193 + 1); // 8193 4-byte unicode characters

        const std::string char_4byte = "\U00012043";
        const std::string char_2byte = "\U00000443";

        const int char2_code = 1091;
        const int char4_code = 73795;

        for (int i = 0; i < 8191; ++i) {
            data += char_4byte;
        }

        data += char_2byte;

        data += char_4byte;
        data += char_4byte;

        input::stream_input in(std::make_unique<std::stringstream>(std::move(data)));

        for (int i = 0; i < 8191; ++i) {
            EXPECT_EQ(in.get(), char4_code);
        }

        EXPECT_EQ(in.get(), char2_code);
        EXPECT_EQ(in.get(), char4_code);
        EXPECT_EQ(in.get(), char4_code);
    }

    TEST(input_tests, stream_input_get_na_test) {
        input::stream_input in(std::make_unique<std::stringstream>("\u0378"));

        EXPECT_THROW(in.get(), std::runtime_error);
    }

    TEST(input_tests, stream_input_peek_test) {
        input::stream_input in(std::make_unique<std::stringstream>("so\n"));

        EXPECT_EQ(in.peek(), 's');
        EXPECT_EQ(in.peek(), 's');

        EXPECT_EQ(in.get(), 's');

        EXPECT_EQ(in.peek(), 'o');
        EXPECT_EQ(in.peek(), 'o');

        in.get(), in.get();

        EXPECT_EQ(in.peek(), -2);
    }

}