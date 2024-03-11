#include "Utily/Utily.hpp"

#include <gtest/gtest.h>

#if 1

TEST(FileWriter, dump_entire_file) {

    std::string_view text = "hello world!";
    auto text_u8 = std::span { reinterpret_cast<const uint8_t*>(text.data()), text.size() };

    auto write_result = Utily::FileWriter::dump_to_file("hello.txt", text_u8);
    EXPECT_FALSE(write_result.has_error());

    auto read_result = Utily::FileReader::load_entire_file("hello.txt");
    EXPECT_FALSE(read_result.has_error());

    if (read_result.has_value()) {
        EXPECT_TRUE(std::ranges::equal(read_result.value(), text_u8));
    }
}
#endif