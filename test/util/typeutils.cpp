#include "gtest/gtest.h"

#include "util/typeutils.h"

namespace alien::test {

    class base {
    public:
        virtual ~base() = default;
    };

    class derived : public base {
    };

    TEST(typeutils_tests, typeutils_check_test) {
        std::unique_ptr<base> derived_ptr = std::make_unique<derived>();

        auto casted = util::check<derived>(derived_ptr.get());

        EXPECT_EQ(casted, derived_ptr.get());

        std::unique_ptr<base> base_ptr = std::make_unique<base>();

        EXPECT_THROW(util::check<derived>(base_ptr.get()), std::logic_error);
    }

}