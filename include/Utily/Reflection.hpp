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
#if defined(__clang__) || defined(__GNUC__)
            // This is necessary as clang-cl (aka windows clang) source_location
            // does not provided templated types.
            const std::string_view name = { __PRETTY_FUNCTION__ };
#else
            const std::string_view name = std::source_location::current().function_name();
#endif

            std::string_view t_equals = "T = ";
            auto begin = std::search(name.begin(), name.end(), t_equals.begin(), t_equals.end());
            if (begin == name.end()) {
                constexpr std::string_view function_sig = "Reflection::get_type_name<";
                begin = std::search(name.begin(), name.end(), function_sig.begin(), function_sig.end());
                if (begin == name.end()) {
                    return name;
                }
                begin += function_sig.size();
                std::string_view function_end = ">(void)";

                auto end = std::search(begin, name.end(), function_end.begin(), function_end.end());

                return std::string_view { begin, end };
            } else {
                // This is GCC and Clang
                begin += static_cast<std::ptrdiff_t>(t_equals.size());
                std::string_view end_delims = " ;]";
                auto end = std::find_first_of(begin, name.end(), end_delims.begin(), end_delims.end());
                return std::string_view { begin, end };
            }
            return name;
        }
    };
}