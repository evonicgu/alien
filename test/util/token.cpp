#include "gtest/gtest.h"

#include "util/token.h"

namespace alien::test {

    TEST(token_utils, test_pos_forwards) {
        util::pos pos{1, 1};

        EXPECT_EQ((pos.forwards(1, 1)), (util::pos{2, 2}));

        EXPECT_EQ((pos.forwards(6, 7)), (util::pos{7, 8}));
    }

    TEST(token_utils, test_pos_backwards) {
        util::pos pos{100, 100};

        EXPECT_EQ((pos.backwards(1, 1)), (util::pos{99, 99}));

        EXPECT_EQ((pos.backwards(101, 1)), (util::pos{0, 99}));

        EXPECT_EQ((pos.backwards(200, 200)), (util::pos{0, 0}));
    }

    TEST(token_utils, test_eq) {
        util::pos pos1{100, 100}, pos2{200, 200};

        EXPECT_FALSE(pos1 == pos2);
        EXPECT_TRUE(pos1 == pos1);
    }

}