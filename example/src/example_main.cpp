#include <Utily/Utily.hpp>

#include <iostream>

int main() {
    std::string_view hw = "Hello World!";
    for (const auto& word : Utily::split(hw, ' ', '!')) {
        std::cout << word << '\n';
    }
}
