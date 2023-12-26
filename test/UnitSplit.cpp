#include <gtest/gtest.h>

#include "Utily/Split.hpp"

#include <list>
#include <memory>
#include <string_view>

using namespace std::literals;

TEST(Split, ByElement) {
    { // Basic string split
        Utily::SplitByElement splitter { "   this is  test   string "sv, ' ' };
        auto iter = splitter.begin();
        EXPECT_EQ(*(iter), "this"sv);
        EXPECT_EQ(*(++iter), "is"sv);
        EXPECT_EQ(*(++iter), "test"sv);
        EXPECT_EQ(*(++iter), "string"sv);
    }

    { // non-contigious split
        std::list<int> list;
        Utily::SplitByElement splitter { list, 0 };
        using SplitType = decltype(*splitter.begin());
        static_assert(
            !std::same_as<SplitType, std::string_view>
            && !std::same_as<SplitType, std::span<int>>);
    }

    { // std::string
        std::string string;
        Utily::SplitByElement splitter { string, 0 };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::string_view>);
    }

    { // std::string
        std::string_view string;
        Utily::SplitByElement splitter { string, 0 };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::string_view>);
    }

    { // std::vector
        std::vector<int> string;
        Utily::SplitByElement splitter { string, 0 };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::span<const int>>);
    }

    { // evaluate
        Utily::SplitByElement splitter { "   this is  test   string "sv, ' ' };
        auto evaled = splitter.evaluate();
        EXPECT_EQ(evaled.size(), 4);
        EXPECT_EQ(evaled[0], "this"sv);
        EXPECT_EQ(evaled[1], "is"sv);
        EXPECT_EQ(evaled[2], "test"sv);
        EXPECT_EQ(evaled[3], "string"sv);
    }

    { // chaining
    }
}

TEST(Split, Split)
{
    {   // Split - ByElement deduction
        auto splitter = Utily::split("112233"sv, '2');
        static_assert(std::same_as<decltype(splitter), Utily::SplitByElement<std::string_view>>);
        EXPECT_EQ(splitter.evaluate()[0], "11");
    }
}