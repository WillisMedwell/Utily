#pragma once

#include <array>
#include <concepts>
#include <tuple>
#include <variant>


#include "Utily/TupleAlgorithms.hpp"

namespace Utily {
    template <typename T, size_t S>
    class StaticVector
    {
        union InternalT {
            std::monostate dummy;
            T data;

            constexpr InternalT()
                : dummy({}) { }

            constexpr InternalT(const T& t)
                : data(t) { }

            constexpr InternalT(T&& t)
                : data(t) { }
        };

        InternalT _data[S];
        size_t _size;

    public:
        constexpr StaticVector()
            : _data()
            , _size(0) {
            static_assert(sizeof(InternalT) == sizeof(T), "Should be unreachable");
        }

        template <typename... Args>
        constexpr StaticVector(Args&&... args) {
            static_assert(sizeof(InternalT) == sizeof(T), "Should be unreachable");
            static_assert(sizeof...(Args) <= S, "Increase the inital size of the StaticVector");

            size_t i = 0;
            auto pred = [&](auto param) {
                _data[i++].data = param;
            };
            auto t = std::make_tuple(args...);
            Utily::tuple_for_each(t, pred);

            _size = sizeof...(Args);
        }

        [[nodiscard]] consteval auto capacity() const noexcept -> size_t { return S; }
        [[nodiscard]] constexpr auto size() const noexcept -> size_t { return _size; }

        struct Iterator {
            InternalT* current;

            [[nodiscard]] constexpr bool operator==(const Iterator& other) const noexcept {
                return this->current == other.current;
            }
            [[nodiscard]] constexpr bool operator!=(const Iterator& other) const noexcept {
                return !(*this == other);
            }
            [[nodiscard]] constexpr auto operator*() noexcept -> T& {
                return current->data;
            }
            [[nodiscard]] constexpr auto operator*() const noexcept -> const T& {
                return current->data;
            }
            constexpr auto operator++() noexcept -> Iterator& {
                ++current;
                return *this;
            }
            constexpr auto operator++(int) noexcept -> Iterator {
                Iterator copy = *this;
                ++current;
                return copy;
            }
        };

        [[nodiscard]] constexpr auto begin() noexcept -> Iterator { return Iterator { &_data[0] }; }
        [[nodiscard]] constexpr auto begin() const noexcept -> Iterator { return Iterator { &_data[0] }; }

        [[nodiscard]] constexpr auto end() noexcept -> Iterator { return Iterator { &_data[0] + _size }; }
        [[nodiscard]] constexpr auto end() const noexcept -> Iterator { return Iterator { &_data[0] + _size }; }

        [[nodiscard]] constexpr auto operator[](size_t index) -> T& {
            return _data[index].data;
        }
    };
}