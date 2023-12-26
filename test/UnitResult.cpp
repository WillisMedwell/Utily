#include <gtest/gtest.h>

#include "Utily/Result.hpp"

#include <memory>
#include <string_view>

using namespace std::literals;

TEST(Result, Constructors) {
    { // Value move constructor
        Utily::Result<int, std::string_view> result { 1 };
        EXPECT_TRUE(result.has_value());
        EXPECT_FALSE(result.has_error());
    }

    { // Error move constructor
        Utily::Result<int, std::string_view> result { "hi"sv };
        EXPECT_TRUE(result.has_error());
        EXPECT_FALSE(result.has_value());
    }

    { // Value copy constructor
        int val = 1;
        Utily::Result<int, std::string_view> result { val };
        EXPECT_TRUE(result.has_value());
        EXPECT_FALSE(result.has_error());
    }

    { // Error copy constructor
        std::string_view error = "error";
        Utily::Result<int, std::string_view> result { error };
        EXPECT_TRUE(result.has_error());
        EXPECT_FALSE(result.has_value());
    }

    { // Still compiles despite not having a copy constructor.
        Utily::Result<std::unique_ptr<int>, std::string_view> result { std::make_unique<int>(1) };
        EXPECT_TRUE(result.has_value());
        EXPECT_FALSE(result.has_error());
    }

    { // Still compiles despite not having a move constructor.
        struct NoMove {
            NoMove() = default;
            NoMove(const NoMove&) = default;
            NoMove(NoMove&&) = delete;
        };
        NoMove val;
        Utily::Result<NoMove, std::string_view> result { val };
        EXPECT_TRUE(result.has_value());
        EXPECT_FALSE(result.has_error());
    }

    { // Void error move
        Utily::Result<void, std::string_view> result { "error"sv };
        EXPECT_TRUE(result.has_error());
        EXPECT_FALSE(result.has_value());
    }

    { // Void error copy
        auto error = "error"sv;
        Utily::Result<void, std::string_view> result { error };
        EXPECT_TRUE(result.has_error());
        EXPECT_FALSE(result.has_value());
    }

    { // Void default
        Utily::Result<void, std::string_view> result;
        EXPECT_FALSE(result.has_error());
        EXPECT_TRUE(result.has_value());
    }
}

TEST(Result, Handling) {
    constexpr static int not_handled = 0;
    constexpr static int handled_as_value = 1;
    constexpr static int handled_as_error = 2;

    auto handle_value = [](int& value) { value = handled_as_value; };
    auto handle_error = [](int& error) { error = handled_as_error; };

    auto handle_s_value = [](std::string_view& value [[maybe_unused]]) {};
    auto handle_s_error = [](std::string_view& error [[maybe_unused]]) {};

    { // handle value
        auto result = Utily::Result<int, std::string_view> { not_handled }.on_value(handle_value);
        EXPECT_TRUE(result.has_value());
        EXPECT_FALSE(result.has_error());
        EXPECT_EQ(result.value(), handled_as_value);
    }

    { // handle error
        auto result = Utily::Result<std::string_view, int> { not_handled }.on_error(handle_error);
        EXPECT_FALSE(result.has_value());
        EXPECT_TRUE(result.has_error());
        EXPECT_EQ(result.error(), handled_as_error);
    }

    { // handle either - value
        auto result = Utily::Result<int, std::string_view> { not_handled }
                          .on_either(handle_value, handle_s_error);
        EXPECT_TRUE(result.has_value());
        EXPECT_FALSE(result.has_error());
        EXPECT_EQ(result.value(), handled_as_value);
    }

    { // handle either - error
        auto result = Utily::Result<std::string_view, int> { not_handled }
                          .on_either(handle_s_value, handle_error);
        EXPECT_FALSE(result.has_value());
        EXPECT_TRUE(result.has_error());
        EXPECT_EQ(result.error(), handled_as_error);
    }

    {   // handle error
        auto result = Utily::Result<void, int> {not_handled}
            .on_error(handle_error);
        EXPECT_FALSE(result.has_value());
        EXPECT_TRUE(result.has_error());
        EXPECT_EQ(result.error(), handled_as_error);
    }
}