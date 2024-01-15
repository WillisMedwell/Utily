#pragma once

#include <source_location>
#include <string>
#include <string_view>
#include <variant>

namespace Utily {
    class Error
    {
        std::variant<std::monostate, std::string, std::string_view> _error;

    public:
        constexpr Error()
            : _error(std::monostate {}) { }

        constexpr Error(std::string&& message)
            : _error(std::forward<std::string>(message)) { }

        constexpr Error(std::string_view& message)
            : _error(std::forward<std::string_view>(message)) { }

        constexpr Error(const char* message)
            : _error(std::string_view { message }) { }

        constexpr auto what() const noexcept -> std::string_view {
            auto get_error_msg = []<typename T>(const T& error) {
                if constexpr (std::same_as<T, std::string>) {
                    return std::string_view { error };
                } else if constexpr (std::same_as<T, std::string_view>) {
                    return error;
                } else {
                    return std::string_view { "Undefined error" };
                }
            };
            return std::visit(get_error_msg, _error);
        }
    };

} // namespace Utily
