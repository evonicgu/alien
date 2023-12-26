#include <vector>
#include <set>
#include <stdexcept>

#include "gtest/gtest.h"

#include "util/vecset.h"

namespace alien::test {

    TEST(vecset_tests, vecset_initialization_test) {
        std::vector<int> values{1, 2, 3, 4, 5};
        auto copy = values;

        std::set<int> svalues{1, 2, 3, 4, 5};

        util::vecset<int> vs[5];

        vs[0] = util::vecset<int>{1, 2, 3, 4, 5};

        vs[1] = util::vecset<int>(values);
        vs[2] = util::vecset<int>(std::move(values));

        vs[3] = util::vecset<int>(svalues);
        vs[4] = util::vecset<int>(std::move(svalues));

        for (auto & v : vs) {
            for (int j = 0; j < 5; ++j) {
                EXPECT_EQ(v[j], copy[j]);
            }
        }

        util::vecset<int> v1(vs[0]);
        util::vecset<int> v2(std::move(vs[0]));

        for (int i = 0; i < 5; ++i) {
            EXPECT_EQ(v1[i], copy[i]);
        }

        for (int i = 0; i < 5; ++i) {
            EXPECT_EQ(v2[i], copy[i]);
        }

        // test self-move
        v1 = std::move(v1);

        for (int i = 0; i < 5; ++i) {
            EXPECT_EQ(v1[i], copy[i]);
        }

        // test self-copy
        v1 = v1;

        for (int i = 0; i < 5; ++i) {
            EXPECT_EQ(v1[i], copy[i]);
        }

        util::vecset<int> v3{1, 2, 3, 4, 5, 6};
        util::vecset<int> v4;

        v4 = v3;

        for (int i = 0; i < v3.size(); ++i) {
            EXPECT_EQ(v3[i], v4[i]);
        }
    }

    TEST(vecset_tests, vecset_size_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        EXPECT_EQ(v.size(), 6);
    }

    TEST(vecset_tests, vecset_clear_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        v.clear();

        EXPECT_EQ(v.size(), 0);
        EXPECT_EQ(v.vbegin(), v.vend());
        EXPECT_EQ(v.sbegin(), v.send());
    }

    TEST(vecset_tests, vecset_to_set_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        auto set = (std::set<int>) v;

        EXPECT_EQ(v.size(), set.size());

        for (int i = 0; i < v.size(); ++i) {
            EXPECT_NE(set.find(v[i]), set.end());
        }
    }

    TEST(vecset_tests, vecset_to_vector_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        auto vector = (std::vector<int>) v;

        EXPECT_EQ(v.size(), vector.size());

        for (int i = 0; i < v.size(); ++i) {
            EXPECT_EQ(v[i], vector[i]);
        }
    }

    TEST(vecset_tests, vecset_push_back_rvalue_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        auto copy = (std::vector<int>) v;
        copy.push_back(7);

        v.push_back(7);

        EXPECT_EQ(v.size(), copy.size());

        for (int i = 0; i < v.size(); ++i) {
            EXPECT_EQ(v[i], copy[i]);
        }

        // the value is already in the set, so the vecset is left unchanged
        v.push_back(4);

        EXPECT_EQ(v.size(), copy.size());

        for (int i = 0; i < v.size(); ++i) {
            EXPECT_EQ(v[i], copy[i]);
        }
    }

    TEST(vecset_tests, vecset_push_back_lvalue_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        constexpr int value7 = 7, value4 = 4;

        auto copy = (std::vector<int>) v;
        copy.push_back(value7);

        v.push_back(value7);

        EXPECT_EQ(v.size(), copy.size());

        for (int i = 0; i < v.size(); ++i) {
            EXPECT_EQ(v[i], copy[i]);
        }

        // the value is already in the set, so the vecset is left unchanged
        v.push_back(value4);

        EXPECT_EQ(v.size(), copy.size());

        for (int i = 0; i < v.size(); ++i) {
            EXPECT_EQ(v[i], copy[i]);
        }
    }

    TEST(vecset_tests, vecset_index_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};
        const util::vecset<int> v_const{1, 2, 3, 4, 5, 6};

        EXPECT_EQ(v[1], 2);
        EXPECT_EQ(v_const[1], 2);

        EXPECT_THROW(v[10], std::out_of_range);
        EXPECT_THROW(v_const[10], std::out_of_range);
    }

    TEST(vecset_tests, vecset_hint_push_back_rvalue_test) {
        util::vecset<int> v{1, 2, 3, 4, 5};

        // hint end
        v.push_back(v.send(), 6);

        // hint beginning
        v.push_back(v.sbegin(), 1);

        EXPECT_EQ(v.size(), 6);
    }

    TEST(vecset_tests, vecset_hint_push_back_lvalue_test) {
        util::vecset<int> v{1, 2, 3, 4, 5};

        int value6 = 6, value1 = 1;

        // hint end
        v.push_back(v.send(), value6);

        // hint beginning
        v.push_back(v.sbegin(), value1);

        EXPECT_EQ(v.size(), 6);
    }

    TEST(vecset_tests, vecset_find_regular_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        EXPECT_EQ(v.vend(), v.find(7));

        EXPECT_EQ(std::distance(v.vbegin(), v.find(2)), 1);
    }

    TEST(vecset_tests, vecset_find_const_test) {
        const util::vecset<int> v{1, 2, 3, 4, 5, 6};

        EXPECT_EQ(v.cvend(), v.find(7));

        EXPECT_EQ(std::distance(v.cvbegin(), v.find(2)), 1);
    }

    TEST(vecset_tests, vecset_find_templated_regular_test) {
        util::vecset<int> v{1, 2, 3, 4, 5, 6};

        EXPECT_EQ(v.vend(), v.find(7.0));

        EXPECT_EQ(std::distance(v.vbegin(), v.find(2.0)), 1);
    }

    TEST(vecset_tests, vecset_find_templated_const_test) {
        const util::vecset<int> v{1, 2, 3, 4, 5, 6};

        EXPECT_EQ(v.cvend(), v.find(7.0));

        EXPECT_EQ(std::distance(v.cvbegin(), v.find(2.0)), 1);
    }

}