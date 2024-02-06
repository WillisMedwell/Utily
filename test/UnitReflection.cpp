#include "Utily/Utily.hpp"

#include <gtest/gtest.h>

#if 0

TEST(Reflection, BaseTypes) {
    {
        auto result = Utily::Reflection::get_type_name<float>();
        auto expected = std::string_view { "float" };
        EXPECT_EQ(result, expected);
    }

    {
        auto result = Utily::Reflection::get_type_name<double>();
        auto expected = std::string_view { "double" };
        EXPECT_EQ(result, expected);
    }

    {
        auto result = Utily::Reflection::get_type_name<Utily::Result<void, Utily::Error>>();
        auto expected = std::string_view { "Utily::Result<void, Utily::Error>" };
        EXPECT_NE(result, "error");
    }
}

#endif