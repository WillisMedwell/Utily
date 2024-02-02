#include <gtest/gtest.h>

#include "Utily/TypeErasedVector.hpp"

#include <iterator>

TEST(TypeErasedVector, Constructor) {
    {
        auto vector = Utily::TypeErasedVector {};
#ifndef NDEBUG
        EXPECT_DEBUG_DEATH(vector.emplace_back<int>(1), "");
#endif
        vector.set_underlying_type<float>();
        EXPECT_EQ(vector.size(), 0);
        EXPECT_EQ(vector.size_bytes(), 0);
        EXPECT_EQ(vector.capacity(), 0);
    }

    {
        auto vector = Utily::TypeErasedVector { float {} };
        EXPECT_EQ(vector.size(), 0);
        EXPECT_EQ(vector.size_bytes(), 0);
        EXPECT_EQ(vector.capacity(), 0);
    }

    {
        auto vector = Utily::TypeErasedVector { float {}, 10 };
        EXPECT_EQ(vector.size(), 10);
        EXPECT_EQ(vector.size_bytes(), 10 * sizeof(float));
        EXPECT_EQ(vector.capacity(), 10);
    }
}

TEST(TypeErasedVector, push_back) {
    auto vector = Utily::TypeErasedVector {};
    vector.set_underlying_type<float>();

    EXPECT_EQ(vector.size(), 0);
    EXPECT_EQ(vector.size_bytes(), 0);
    EXPECT_EQ(vector.capacity(), 0);

#ifndef NDEBUG
    EXPECT_DEBUG_DEATH(vector.emplace_back<int>(1), "");
    EXPECT_DEBUG_DEATH(vector.emplace_back<double>(1), "");
#endif

    vector.push_back<float>(0.0f);
    vector.push_back<float>(1.0f);
    vector.push_back<float>(2.0f);
    vector.push_back<float>(3.0f);
    vector.push_back<float>(4.0f);
    vector.emplace_back<float>(5.0f);
    vector.emplace_back<float>(6.0f);
    vector.emplace_back<float>(7.0f);
    vector.emplace_back<float>(8.0f);

    float expected = 0.0f;
    for (float& v : vector.as_span<float>()) {
        EXPECT_EQ(v, expected);
        expected += 1.0f;
    }
}

TEST(TypeErasedVector, resize) {
    auto vector = Utily::TypeErasedVector {};
    vector.set_underlying_type<float>();
    vector.resize(10);

    EXPECT_EQ(vector.size(), 10);
    EXPECT_EQ(vector.size_bytes(), 10 * sizeof(float));
    EXPECT_EQ(vector.capacity(), 10);

    vector.resize(0);
    EXPECT_EQ(vector.size(), 0);
    EXPECT_EQ(vector.size_bytes(), 0);
    EXPECT_EQ(vector.capacity(), 10);
}