#include <gtest/gtest.h>

#include "Utily/Split.hpp"

#include <list>
#include <memory>
#include <string_view>

using namespace std::literals;

TEST(Split, SplitsByElement) {
    { // Basic string split
        auto string = "   this is  test   string "sv;
        Utily::Split::ByElement splitter { string, ' ' };
        auto iter = splitter.begin();
        EXPECT_EQ(*(iter), "this"sv);
        EXPECT_EQ(*(++iter), "is"sv);
        EXPECT_EQ(*(++iter), "test"sv);
        EXPECT_EQ(*(++iter), "string"sv);
    }

    { // non-contigious split
        std::list<int> list;
        Utily::Split::ByElement splitter { list, 0 };
        using SplitType = std::iter_value_t<decltype(splitter.begin())>;
        static_assert(
            !std::same_as<SplitType, std::string_view>
            && !std::same_as<SplitType, std::span<int>>);
    }

    { // std::string
        std::string string;
        Utily::Split::ByElement splitter { string, 0 };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::string_view>);
    }

    { // std::string
        std::string_view string;
        Utily::Split::ByElement splitter { string, 0 };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::string_view>);
    }

    { // std::vector
        std::vector<int> string;
        Utily::Split::ByElement splitter { string, 0 };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::span<const int>>);
    }

    { // evaluate
        auto string = "   this is  test   string "sv;
        Utily::Split::ByElement splitter { string, ' ' };
        auto evaled = splitter.evaluate();
        EXPECT_EQ(evaled.size(), 4);
        EXPECT_EQ(evaled[0], "this"sv);
        EXPECT_EQ(evaled[1], "is"sv);
        EXPECT_EQ(evaled[2], "test"sv);
        EXPECT_EQ(evaled[3], "string"sv);
    }
}

TEST(Split, SplitByElements) {
    { // Basic string split
        auto string = "   this is  test   string "sv;
        Utily::Split::ByElements splitter { string, std::to_array({ ' ', 's' }) };
        auto iter = splitter.begin();
        EXPECT_EQ(*(iter), "thi"sv);
        EXPECT_EQ(*(++iter), "i"sv);
        EXPECT_EQ(*(++iter), "te"sv);
        EXPECT_EQ(*(++iter), "t"sv);
        EXPECT_EQ(*(++iter), "tring"sv);
    }

    { // non-contigious split
        std::list<int> list;
        Utily::Split::ByElements splitter { list, std::to_array({ 0, 1 }) };
        using SplitType = decltype(*splitter.begin());
        static_assert(
            !std::same_as<SplitType, std::string_view>
            && !std::same_as<SplitType, std::span<int>>);
    }

    { // std::string
        std::string string;
        Utily::Split::ByElements splitter { string, std::to_array({ 0, 1 }) };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::string_view>);
    }

    { // std::vector
        std::vector<int> vector;
        Utily::Split::ByElements splitter { vector, std::to_array({ 0, 1 }) };
        using SplitType = decltype(*splitter.begin());
        static_assert(std::same_as<SplitType, std::span<const int>>);
    }

    { // Basic string split
        auto string = "   this is  test   string "sv;
        Utily::Split::ByElements splitter { string, std::to_array({ ' ', 's' }) };
        EXPECT_EQ(splitter.evaluate()[0], "thi"sv);
        EXPECT_EQ(splitter.evaluate()[1], "i"sv);
        EXPECT_EQ(splitter.evaluate()[2], "te"sv);
        EXPECT_EQ(splitter.evaluate()[3], "t"sv);
        EXPECT_EQ(splitter.evaluate()[4], "tring"sv);
    }
}

TEST(Split, split) {
    { // Split - ByElement deduction
        auto val = "112233"sv;
        auto splitter = Utily::split(val, '2');
        static_assert(std::same_as<decltype(splitter), Utily::Split::ByElement<std::string_view>>);
        EXPECT_EQ(splitter.evaluate()[0], "11");
        EXPECT_EQ(splitter.evaluate().size(), 2);
    }

    { // Split - ByElements deduction
        auto val = "112233"sv;
        auto splitter = Utily::split(val, '2', '1');
        static_assert(std::same_as<decltype(splitter), Utily::Split::ByElements<std::string_view, 2, char>>);
        EXPECT_EQ(splitter.evaluate()[0], "33");
        EXPECT_EQ(splitter.evaluate().size(), 1);
    }

}