#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <source_location>
#include <string_view>
#include <type_traits>

namespace Utily {
struct Reflection {
    template <typename T>
    consteval static auto get_type_name() -> std::string_view {
        std::string_view name = std::source_location::current().function_name();
        std::string_view t_equals = "T = ";
        auto begin = std::search(name.begin(), name.end(), t_equals.begin(),
                                 t_equals.end());
        if (begin == name.end()) {
            // This is MSVC
            constexpr std::string_view function_sig = "Reflection::get_type_name<";

            begin = std::search(name.begin(), name.end(), function_sig.begin(), function_sig.end());
            if(begin == name.end()) {
                return "error";
            }
            begin += function_sig.size();
            std::string_view function_end = ">(void)";

            auto end = std::search(begin, name.end(), function_end.begin(),
                                   function_end.end());

            name = std::string_view{begin, end};
        } else {
            // This is GCC and Clang
            begin += t_equals.size();
            std::string_view end_delims = " ;]";
            auto end = std::find_first_of(begin, name.end(), end_delims.begin(),
                                          end_delims.end());
            name = std::string_view{begin, end};
        }
        return name;
    }
};
}