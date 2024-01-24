#include "Utily/Utily.hpp"

#include <gtest/gtest.h>

#if 1

static std::vector<uint8_t> readFileToVector(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + path.string());
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg); 

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Error occurred while reading the file: " + path.string());
    }
    return buffer;
}
const static auto STANFORD_BUNNY_PATH = std::filesystem::path { "resources/stanford_bunny.ply" };
const static auto SMALL_TEXT_PATH = std::filesystem::path { "resources/small.txt" };

const static auto STANFORD_BUNNY_DATA = readFileToVector(STANFORD_BUNNY_PATH);
const static auto SMALL_TEXT_DATA = readFileToVector(SMALL_TEXT_PATH);

TEST(FileReader, load_entire_file) {
    auto result_bunny = Utily::FileReader::load_entire_file(STANFORD_BUNNY_PATH);
    auto result_text = Utily::FileReader::load_entire_file(SMALL_TEXT_PATH);

    EXPECT_FALSE(result_bunny.has_error());
    EXPECT_FALSE(result_text.has_error());

    EXPECT_EQ(result_bunny.value(), STANFORD_BUNNY_DATA);
    EXPECT_EQ(result_text.value(), SMALL_TEXT_DATA);
}
#endif