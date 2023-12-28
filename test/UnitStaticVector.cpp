#include <gtest/gtest.h>

#include "Utily/StaticVector.hpp"

TEST(StaticVector, Constructors) {
    {   // default
        Utily::StaticVector<int, 10> sv;
        EXPECT_EQ(sv.capacity(), 10);
        EXPECT_EQ(sv.size(), 0);
        EXPECT_EQ(sv.begin(), sv.end());
    }

    {   // with args&&
        Utily::StaticVector<int, 10> sv{1, 2, 3, 4};
        EXPECT_EQ(sv.capacity(), 10);
        EXPECT_EQ(sv.size(), 0);
        EXPECT_EQ(sv.begin(), sv.end());
    }
}