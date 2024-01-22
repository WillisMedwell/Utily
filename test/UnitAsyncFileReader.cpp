#include "Utily/Utily.hpp"

#include <gtest/gtest.h>

#if 0

const static auto STANFORD_BUNNY_PATH = std::filesystem::path { "resources/stanford_bunny.ply" };
const static auto STANFORD_BUNNY_DATA = std::vector<char> (
    (std::istreambuf_iterator<char>(std::ifstream(STANFORD_BUNNY_PATH, std::ios::binary).rdbuf())),
    std::istreambuf_iterator<char>()
);

const static auto SMALL_TEXT_PATH = std::filesystem::path { "resources/small.txt" };
const static auto SMALL_TEXT_DATA = std::vector<char> (
    (std::istreambuf_iterator<char>(std::ifstream(SMALL_TEXT_PATH, std::ios::binary).rdbuf())),
    std::istreambuf_iterator<char>()
);

TEST(AsyncFileReader, WaitForAll) {
    {
        auto handle_error = [](auto a [[maybe_unused]]) {
            EXPECT_EQ("No Error", a.what());
        };
        EXPECT_TRUE(std::filesystem::exists({ "resources" }));
        Utily::AsyncFileReader::push(STANFORD_BUNNY_PATH).on_error(handle_error);
        Utily::AsyncFileReader::push(SMALL_TEXT_PATH).on_error(handle_error);
        Utily::AsyncFileReader::wait_for_all();
        Utily::AsyncFileReader::pop(SMALL_TEXT_PATH)
            .on_error(handle_error)
            .on_value([&](auto data) {
                EXPECT_EQ(SMALL_TEXT_DATA, data);
            });
        Utily::AsyncFileReader::pop(STANFORD_BUNNY_PATH)
            .on_error(handle_error)
            .on_value([&](auto data) {
                EXPECT_EQ(STANFORD_BUNNY_DATA, data);
            });
    }
}

TEST(AsyncFileReader, WaitForIndividual) {
    {
        auto handle_error = [](auto a [[maybe_unused]]) {
            EXPECT_EQ("No Error", a.what());
        };

        EXPECT_TRUE(std::filesystem::exists({ "resources" }));
        Utily::AsyncFileReader::push(STANFORD_BUNNY_PATH).on_error(handle_error);
        Utily::AsyncFileReader::push(SMALL_TEXT_PATH).on_error(handle_error);
        Utily::AsyncFileReader::wait_pop(SMALL_TEXT_PATH)
            .on_error(handle_error)
            .on_value([&](auto data) {
                EXPECT_EQ(SMALL_TEXT_DATA, data);
            });
        Utily::AsyncFileReader::wait_pop(STANFORD_BUNNY_PATH)
            .on_error(handle_error)
            .on_value([&](auto data) {
                EXPECT_EQ(STANFORD_BUNNY_DATA, data);
            });
    }
}

#endif