#include <gtest/gtest.h>

#include "Utily/Utily.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <random>
#include <ranges>
#include <string_view>
#include <vector>

using namespace std::literals;

const auto NUMS = std::vector<uint8_t> { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
const auto STRING = std::string { "more stuff here to fill in. hello world! This is a sentenze" };

TEST(Simd128, find) {

    EXPECT_EQ(
        std::ranges::find(STRING, 'z'),
        STRING.begin() + Utily::Simd128::Char::find(STRING.data(), STRING.size(), 'z'));

    for (int64_t i = 0; i < 1000; ++i) {
        std::string tmp;
        tmp.resize(i);
        std::ranges::fill(tmp, 'a');
        tmp.push_back('z');
        auto expected = std::ranges::find(tmp, 'z');
        auto result = tmp.begin() + Utily::Simd128::Char::find(tmp.data(), tmp.size(), 'z');
        EXPECT_EQ(expected, result);

        for (int64_t j = i - 1; j > 0; --j) {
            tmp[j] = 'z';
            expected = std::ranges::find(tmp, 'z');
            result = tmp.begin() + Utily::Simd128::Char::find(tmp.data(), tmp.size(), 'z');
            EXPECT_EQ(expected, result);
        }
    }
}

TEST(Simd512, find) {
 /*   EXPECT_EQ(
        std::ranges::find(STRING, 'z'),
        STRING.begin() + Utily::Simd512::Char::find(STRING.data(), STRING.size(), 'z'));*/

    for (int64_t i = 0; i < 2000; ++i) {
        std::string tmp;
        tmp.resize(i);
        std::ranges::fill(tmp, 'a');
        tmp.push_back('z');
        auto expected = std::ranges::find(tmp, 'z');
        
        auto result = tmp.begin() + Utily::Simd512::Char::find(tmp.data(), tmp.size(), 'z');
        EXPECT_EQ(expected, result);

        for (int64_t j = i - 1; j > 0; --j) {
            tmp[static_cast<size_t>(j)] = 'z';
            expected = std::ranges::find(tmp, 'z');
            result = tmp.begin() + Utily::Simd512::Char::find(tmp.data(), tmp.size(), 'z');
            EXPECT_EQ(expected, result);
        }
       
    }
}

TEST(Simd, find_first_of) {
    std::string_view delims { "azxy" };

    EXPECT_EQ(
        std::ranges::find_first_of(STRING, delims),
        STRING.begin() + Utily::Simd128::Char::find_first_of(STRING.data(), STRING.size(), delims.data(), delims.size()));
}

TEST(Simd128, search_4letters) {
    std::string_view word1 = "sent";
    std::string_view word2 = "worl";

    auto expected1 = std::string_view {
        std::search(STRING.begin(), STRING.end(), word1.begin(), word1.end()),
        STRING.end()
    };
    auto expected2 = std::string_view {
        std::search(STRING.begin(), STRING.end(), word2.begin(), word2.end()),
        STRING.end()
    };

    auto actual1 = std::string_view {
        STRING.begin() + Utily::Simd128::Char::search(STRING.data(), STRING.size(), word1.data(), word1.size()),
        STRING.end()
    };

    auto actual2 = std::string_view {
        STRING.begin() + Utily::Simd128::Char::search(STRING.data(), STRING.size(), word2.data(), word2.size()),
        STRING.end()
    };

    EXPECT_EQ(actual1, expected1);
    EXPECT_EQ(actual2, expected2);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

    std::string src;
    for (size_t i = 0; i < 100; ++i) {
        src.resize(i);
        std::ranges::generate(src, [&]() { return static_cast<char>(dist(gen)); });

        for (size_t j = 3; j < i; ++j) {
            const char* delims = src.data() + j - 3;
            auto uty_result = std::string_view {
                src.begin() + Utily::Simd128::Char::search(src.data(), src.size(), delims, 4),
                src.end()
            };
            auto std_expected = std::string_view {
                std::search(src.begin(), src.end(), delims, delims + 4),
                src.end()
            };
            EXPECT_EQ(std_expected, uty_result);
        }

        std::string delims;
        delims.resize(4);
        for (size_t j = 0; j < 10; ++j) {
            std::ranges::generate(delims, [&]() { return static_cast<char>(dist(gen)); });

            auto uty_result = std::string_view {
                src.begin() + Utily::Simd128::Char::search(src.data(), src.size(), delims.data(), 4),
                src.end()
            };
            auto std_expected = std::string_view {
                std::search(src.begin(), src.end(), delims.begin(), delims.end()),
                src.end()
            };
            if (std_expected != uty_result) {
                std::clog << delims << '\n';
                std::clog << src << '\n';
                EXPECT_EQ(std_expected, uty_result);
                assert(false);
            }
        }
    }
}

TEST(Simd512, search_4letters) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

    std::string src;
    for (size_t i = 0; i < 100; ++i) {
        src.resize(i);
        std::ranges::generate(src, [&]() { return static_cast<char>(dist(gen)); });

        for (size_t j = 3; j < i; ++j) {
            const char* delims = src.data() + j - 3;
            
            auto uty_result = std::string_view {
                
                src.begin() + Utily::Simd512::Char::search(src.data(), src.size(), delims, 4),
                src.end()
            };
            auto std_expected = std::string_view {
                std::search(src.begin(), src.end(), delims, delims + 4),
                src.end()
            };
            EXPECT_EQ(std_expected, uty_result);
        }

        std::string delims;
        delims.resize(4);
        for (size_t j = 0; j < 10; ++j) {
            std::ranges::generate(delims, [&]() { return static_cast<char>(dist(gen)); });

            auto uty_result = std::string_view {
                src.begin() + Utily::Simd512::Char::search(src.data(), src.size(), delims.data(), 4),
                src.end()
            };
            auto std_expected = std::string_view {
                std::search(src.begin(), src.end(), delims.begin(), delims.end()),
                src.end()
            };
            if (std_expected != uty_result) {
                std::clog << delims << '\n';
                std::clog << src << '\n';
                EXPECT_EQ(std_expected, uty_result);
                assert(false);
            }
        }
    }
}


TEST(Simd, search_8letters_basic) {
    std::string_view word1 = "sentenze";
    std::string_view word2 = "world! T";

    auto expected1 = std::string_view {
        std::search(STRING.begin(), STRING.end(), word1.begin(), word1.end()),
        STRING.end()
    };
    auto expected2 = std::string_view {
        std::search(STRING.begin(), STRING.end(), word2.begin(), word2.end()),
        STRING.end()
    };

    auto actual1 = std::string_view {
        STRING.begin() + Utily::Simd128::Char::search(STRING.data(), STRING.size(), word1.data(), word1.size()),
        STRING.end()
    };

    auto actual2 = std::string_view {
        STRING.begin() + Utily::Simd128::Char::search(STRING.data(), STRING.size(), word2.data(), word2.size()),
        STRING.end()
    };

    EXPECT_EQ(actual1, expected1);
    EXPECT_EQ(actual2, expected2);
}

TEST(Simd, search_8letters_find_existing) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

    std::string src;

    for (size_t i = 0; i < 1000; ++i) {
        src.resize(i);
        std::ranges::generate(src, [&]() { return static_cast<char>(dist(gen)); });

        for (size_t j = 7; j < i; ++j) {
            const char* delims = src.data() + j - 7;
            auto uty_result = std::string_view {
                src.begin() + Utily::Simd128::Char::search(src.data(), src.size(), delims, 8),
                src.end()
            };
            auto std_expected = std::string_view {
                std::search(src.begin(), src.end(), delims, delims + 8),
                src.end()
            };
            if (std_expected != uty_result) {
                std::clog << "i: " << i << '\n';
                std::clog << std::string_view { delims, 8 } << '\n';
                std::clog << src << '\n';
                assert(false);
            }
            EXPECT_EQ(std_expected, uty_result);
        }
    }
}

TEST(Simd, search_8letters_find_random) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

    std::string src;
    std::array<char, 8> delims;

    for (size_t i = 0; i < 1000; ++i) {
        src.resize(i);
        std::ranges::generate(src, [&]() { return static_cast<char>(dist(gen)); });

        for (size_t j = 0; j < 100; ++j) {
            std::ranges::generate(delims, [&]() { return static_cast<char>(dist(gen)); });

            const auto uty_result = std::string_view {
                src.begin() + Utily::Simd128::Char::search(src.data(), src.size(), delims.data(), delims.size()),
                src.end()
            };
            const auto std_expected = std::string_view {
                std::search(src.begin(), src.end(), delims.begin(), delims.end()),
                src.end()
            };
            if (std_expected != uty_result) {
                std::clog << std::string_view { delims.data(), delims.size() } << '\n';
                std::clog << src << '\n';
                assert(false);
            }
            EXPECT_EQ(std_expected, uty_result);
        }
    }
}
