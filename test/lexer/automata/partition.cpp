#include "gtest/gtest.h"

#include "lexer/automata/partition.h"

#include <unordered_set>

namespace alien::test {

    TEST(partition_tests, test_size) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        EXPECT_EQ(p.size(0), max);

        p.mark(9);
        p.split(0);

        EXPECT_EQ(p.size(0), max - 1);
        EXPECT_EQ(p.size(1), 1);
    }

    TEST(partition_tests, test_set) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        p.mark(8);
        p.mark(9);

        p.split(0);

        for (std::size_t i = 0; i < 8; ++i) {
            EXPECT_EQ(p.set(i), 0);
        }

        EXPECT_EQ(p.set(8), 1);
        EXPECT_EQ(p.set(9), 1);
    }

    TEST(partition_tests, test_get_first) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        p.mark(8);
        p.mark(9);

        p.split(0);

        std::size_t first = p.get_first(0);

        EXPECT_TRUE(first < 8);

        first = p.get_first(1);

        EXPECT_TRUE(first >= 8 && first <= 9);
    }

    TEST(partition_tests, test_get_next) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        p.mark(8);
        p.mark(9);

        std::unordered_set<std::size_t> first_set;

        for (std::size_t i = 1; i < 8; ++i) {
            first_set.insert(i);
        }

        std::ptrdiff_t i = p.get_next(p.get_first(0));

        while (i != -1) {
            first_set.erase(i);

            i = p.get_next(i);
        }

        EXPECT_TRUE(first_set.empty());

        EXPECT_EQ(p.get_next(p.get_first(1)), 9);
    }

    TEST(partition_tests, test_no_marks) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        EXPECT_TRUE(p.no_marks(0));

        p.mark(8);
        p.mark(9);

        EXPECT_FALSE(p.no_marks(0));

        p.split(0);

        EXPECT_TRUE(p.no_marks(0));
        EXPECT_TRUE(p.no_marks(1));
    }

    TEST(partition_tests, test_split_normal) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        p.mark(8);
        p.mark(9);

        EXPECT_EQ(p.split(0), 1);
    }

    TEST(partition_tests, test_split_no_marked) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        EXPECT_EQ(p.split(0), 0);
    }

    TEST(partition_tests, test_split_all_marked) {
        const std::size_t max = 10;

        lexer::automata::partition::partition p(max);

        for (std::size_t i = 0; i < 10; ++i) {
            p.mark(i);
        }

        EXPECT_EQ(p.split(0), 0);
    }

}
