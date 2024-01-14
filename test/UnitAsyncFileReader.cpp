#include "Utily/Utily.hpp"

#include <gtest/gtest.h>

const static auto STANFORD_BUNNY_PATH = std::filesystem::path { "resources/stanford_bunny.ply" };
const static auto SMALL_TEXT_PATH = std::filesystem::path { "resources/small.txt" };

void printDirectory(const std::filesystem::path& path, const std::string& indent = "") {
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        std::cerr << "Provided path is not a directory or does not exist.\n";
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        const auto& p = entry.path();
        std::cout << indent << p.filename().string();

        if (std::filesystem::is_directory(p)) {
            std::cout << "/\n";
            printDirectory(p, indent + "  "); // Recursively print subdirectories
        } else {
            std::cout << "\n";
        }
    }
}

TEST(AsyncFileReader, push) {
    {
        EXPECT_TRUE(std::filesystem::exists({"resources"}));

        Utily::AsyncFileReader::push(STANFORD_BUNNY_PATH).on_error([](auto a [[maybe_unused]]) {
            EXPECT_EQ("", a.what());
        });

        Utily::AsyncFileReader::push(SMALL_TEXT_PATH).on_error([](auto a [[maybe_unused]]) {
            EXPECT_EQ("", a.what());
        });
    }
}

TEST(AsyncFileReader, pop) {
    {
        EXPECT_TRUE(std::filesystem::exists({"resources"}));

        Utily::AsyncFileReader::push(STANFORD_BUNNY_PATH);
        Utily::AsyncFileReader::push(SMALL_TEXT_PATH);
        Utily::AsyncFileReader::wait_for_all();
    }
}