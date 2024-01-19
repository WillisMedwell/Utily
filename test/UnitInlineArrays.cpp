#include "Utily/Utily.hpp"

#include <gtest/gtest.h>
#include <vector>

TEST(InlineArrays, Uninitialised) {
    using namespace Utily;
    {
        auto [data, s1, s2] = Utily::InlineArrays::alloc_uninit<uint8_t, uint8_t>(1, 1);
        EXPECT_NE(data.get(), nullptr);
        EXPECT_EQ(reinterpret_cast<std::byte*>(s1.data()), data.get());
        EXPECT_EQ(reinterpret_cast<std::byte*>(s2.data()), data.get() + 1);
    }
    {
        using Allocator = InlineArrays;
        auto [owner, span_int] = Allocator::alloc_uninit<int>(10);
        for (auto& element : span_int) {
            EXPECT_TRUE(Allocator::is_aligned<int>(reinterpret_cast<std::byte*>(&element)));
        }
    }

    {
        using Allocator = InlineArrays;
        auto [owner, span_int, span_double] = Allocator::alloc_uninit<int, double>(10, 5);
        for (auto& element : span_int) {
            EXPECT_TRUE(Allocator::is_aligned<int>(reinterpret_cast<std::byte*>(&element)));
        }
        for (auto& element : span_double) {
            EXPECT_TRUE(Allocator::is_aligned<double>(reinterpret_cast<std::byte*>(&element)));
        }
    }

    {
        using Allocator = InlineArrays;
        auto [owner, span_char, span_int, span_double] = Allocator::alloc_uninit<char, int, double>(20, 10, 5);
        for (auto& element : span_char) {
            EXPECT_TRUE(Allocator::is_aligned<char>(reinterpret_cast<std::byte*>(&element)));
        }
        for (auto& element : span_int) {
            EXPECT_TRUE(Allocator::is_aligned<int>(reinterpret_cast<std::byte*>(&element)));
        }
        for (auto& element : span_double) {
            EXPECT_TRUE(Allocator::is_aligned<double>(reinterpret_cast<std::byte*>(&element)));
        }
    }
}

TEST(InlineArrays, DefaultConstructor) {
    struct Thing1 {
        Thing1() {
            a = "default constructor value";
        }
        std::string a;
    };
    struct Thing2 {
        std::string a = "default value";
    };

    {
        auto [owner, span_1, span_2] = Utily::InlineArrays::alloc_dafault<Thing1, Thing2>(2, 2);

        for (Thing1& t : span_1) {
            EXPECT_EQ(t.a, "default constructor value");
        }
        for (Thing2& t : span_2) {
            EXPECT_EQ(t.a, "default value");
        }
    }
}

TEST(InlineArrays, CopyRanges) {
    {
        std::vector<int32_t> a = { 1, 2, 3, 4 };
        std::vector<bool> b = { false, true, false, true };

        auto [data, s1, s2] = Utily::InlineArrays::alloc_copy<int32_t, bool>(a, b);

        EXPECT_EQ(a.size(), s1.size());
        EXPECT_EQ(b.size(), s2.size());


        for(size_t i = 0; i < a.size(); ++i) {
            auto expected = a[i];
            auto actual = s1[i];
            EXPECT_EQ(expected, actual);
        }
        for(size_t i = 0; i < b.size(); ++i) {
            auto expected = b[i];
            auto actual = s2[i];
            EXPECT_EQ(expected, actual);
        }
    }

    {
        std::vector<int32_t> a = { 1, 2, 3, 4 };
        std::vector<bool> b = { false, true, false, true };
        Utily::StaticVector<char, 10> c = {'a', 'b', 'c'};

        auto [data, s1, s2, s3] = Utily::InlineArrays::alloc_copy(a, b, c);

        EXPECT_EQ(a.size(), s1.size());
        EXPECT_EQ(b.size(), s2.size());
        EXPECT_EQ(c.size(), s3.size());

        for(size_t i = 0; i < a.size(); ++i) {
            auto expected = a[i];
            auto actual = s1[i];
            EXPECT_EQ(expected, actual);
        }
        for(size_t i = 0; i < b.size(); ++i) {
            auto expected = b[i];
            auto actual = s2[i];
            EXPECT_EQ(expected, actual);
        }
        for(size_t i = 0; i < c.size(); ++i) {
            auto expected = c[i];
            auto actual = s3[i];
            EXPECT_EQ(expected, actual);
        }
    }
}