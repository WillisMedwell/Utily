#include <gtest/gtest.h>

#include "Utily/Utily.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

using namespace std::literals;

const auto NUMS = std::vector<uint8_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
const auto STRING = std::string { "hello world! This is a sentenze" };

TEST(Simd, find) {
    EXPECT_EQ(
        std::ranges::find(STRING, 'z'),
        Utily::Simd::find(STRING.begin(), STRING.end(), 'z'));

    EXPECT_EQ(
        std::ranges::find(NUMS, 16),
        Utily::Simd::find(NUMS.begin(), NUMS.end(), static_cast<uint16_t>(16)));
}

TEST(Simd, find_first_of) {
    std::string_view delims {
        "azxy" };

    EXPECT_EQ(
        std::ranges::find_first_of(STRING, delims),
        Utily::Simd::find_first_of(STRING.begin(), STRING.end(), delims.begin(), delims.end()));

}