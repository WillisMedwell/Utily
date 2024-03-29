#pragma once

#include <array>
#include <concepts>
#include <iostream>
#include <tuple>
#include <variant>
#include <algorithm>
#include <cassert>

#include "Utily/Concepts.hpp"
#include "Utily/TupleAlgo.hpp"

namespace Utily {
    template <typename T, std::ptrdiff_t S>
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

            constexpr auto operator=(T&& other) -> InternalT& {
                data = std::forward<T>(other);
                return *this;
            }
            constexpr auto operator=(const T& other) -> InternalT& {
                data = other;
                return *this;
            }
            constexpr ~InternalT() { }
        };

        InternalT _data[static_cast<uint64_t>(S)];
        std::ptrdiff_t _size;

    public:
        using value_type = T;

        constexpr StaticVector()
            : _data()
            , _size(0) {
        }

        [[nodiscard]] constexpr auto capacity() const noexcept { return S; }
        [[nodiscard]] constexpr auto size() const noexcept -> size_t { return static_cast<size_t>(_size); }

        struct Iterator {
            using iterator_category = std::contiguous_iterator_tag;
            using value_type = T;
            using element_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            InternalT* current;

            [[nodiscard]] friend constexpr auto operator<=>(const Iterator&, const Iterator&) = default;

            [[nodiscard]] constexpr auto operator*() noexcept -> reference {
                return current->data;
            }
            [[nodiscard]] constexpr auto operator*() const noexcept -> reference {
                return current->data;
            }
            [[nodiscard]] constexpr auto operator->() noexcept -> pointer {
                return &current->data;
            }
            [[nodiscard]] constexpr auto operator->() const noexcept -> pointer {
                return &current->data;
            }
            [[nodiscard]] friend constexpr auto operator+(const Iterator& iterator, difference_type offset) noexcept -> Iterator {
                return { iterator.current + offset };
            }
            [[nodiscard]] friend constexpr auto operator+(difference_type offset, const Iterator& iterator) noexcept -> Iterator {
                return { iterator.current + offset };
            }
            [[nodiscard]] friend constexpr auto operator-(const Iterator& iterator, difference_type offset) noexcept -> Iterator {
                return { iterator.current - offset };
            }
            [[nodiscard]] friend constexpr auto operator-(difference_type offset, const Iterator& iterator) noexcept -> Iterator {
                return { iterator.current - offset };
            }
            [[nodiscard]] friend constexpr auto operator-(const Iterator& lhs, const Iterator& rhs) noexcept -> difference_type {
                return lhs.current - rhs.current;
            }
            constexpr auto operator+=(difference_type offset) noexcept -> Iterator& {
                current += offset;
                return *this;
            }
            constexpr auto operator-=(difference_type offset) noexcept -> Iterator& {
                current -= offset;
                return *this;
            }
            constexpr auto operator++() noexcept -> Iterator& {
                ++current;
                return *this;
            }
            constexpr auto operator--() noexcept -> Iterator& {
                --current;
                return *this;
            }
            [[nodiscard]] constexpr auto operator++(int) noexcept -> Iterator {
                Iterator copy = *this;
                ++current;
                return copy;
            }
            [[nodiscard]] constexpr auto operator--(int) noexcept -> Iterator {
                Iterator copy = *this;
                --current;
                return copy;
            }
            [[nodiscard]] constexpr auto operator[](difference_type index) const noexcept -> reference {
                return (current + index)->data;
            }
        };
        struct ConstIterator {
            using iterator_category = std::contiguous_iterator_tag;
            using value_type = T;
            using element_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            const InternalT* current;

            [[nodiscard]] friend constexpr auto operator<=>(const ConstIterator&, const ConstIterator&) = default;

            [[nodiscard]] constexpr auto operator*() noexcept -> reference {
                return current->data;
            }
            [[nodiscard]] constexpr auto operator*() const noexcept -> reference {
                return current->data;
            }
            [[nodiscard]] constexpr auto operator->() noexcept -> pointer {
                return &current->data;
            }
            [[nodiscard]] constexpr auto operator->() const noexcept -> pointer {
                return &current->data;
            }
            [[nodiscard]] friend constexpr auto operator+(const ConstIterator& iterator, difference_type offset) noexcept -> ConstIterator {
                return { iterator.current + offset };
            }
            [[nodiscard]] friend constexpr auto operator+(difference_type offset, const ConstIterator& iterator) noexcept -> ConstIterator {
                return { iterator.current + offset };
            }
            [[nodiscard]] friend constexpr auto operator-(const ConstIterator& iterator, difference_type offset) noexcept -> ConstIterator {
                return { iterator.current - offset };
            }
            [[nodiscard]] friend constexpr auto operator-(difference_type offset, const ConstIterator& iterator) noexcept -> ConstIterator {
                return { iterator.current - offset };
            }
            [[nodiscard]] friend constexpr auto operator-(const ConstIterator& lhs, const ConstIterator& rhs) noexcept -> difference_type {
                return lhs.current - rhs.current;
            }
            constexpr auto operator+=(difference_type offset) noexcept -> ConstIterator& {
                current += offset;
                return *this;
            }
            constexpr auto operator-=(difference_type offset) noexcept -> ConstIterator& {
                current -= offset;
                return *this;
            }
            constexpr auto operator++() noexcept -> ConstIterator& {
                ++current;
                return *this;
            }
            constexpr auto operator--() noexcept -> ConstIterator& {
                --current;
                return *this;
            }
            [[nodiscard]] constexpr auto operator++(int) noexcept -> ConstIterator {
                ConstIterator copy = *this;
                ++current;
                return copy;
            }
            [[nodiscard]] constexpr auto operator--(int) noexcept -> ConstIterator {
                ConstIterator copy = *this;
                --current;
                return copy;
            }
            [[nodiscard]] constexpr auto operator[](difference_type index) const noexcept -> reference {
                return (current + index)->data;
            }
        };

        [[nodiscard]] constexpr auto begin() noexcept -> Iterator { return Iterator { &_data[0] }; }
        [[nodiscard]] constexpr auto begin() const noexcept -> ConstIterator { return ConstIterator { &_data[0] }; }

        template <typename... Args>
            requires((!std::is_reference_v<Args>) && ...)
        constexpr StaticVector(Args&&... args) {
            static_assert(sizeof(InternalT) == sizeof(T), "Should be unreachable");
            static_assert(sizeof...(Args) <= S, "Increase the inital size of the StaticVector");
            _size = sizeof...(Args);

            std::ptrdiff_t index = 0;

            auto construct_inplace_element = [&]<typename Arg>(Arg&& arg) {
                std::construct_at(&_data[index].data, std::forward<Arg>(arg));
                ++index;
            };

            Utily::TupleAlgo::for_each(std::forward_as_tuple(std::forward<Args>(args)...), construct_inplace_element);
        }

        [[nodiscard]] constexpr auto end() noexcept -> Iterator { return Iterator { &_data[0] + _size }; }
        [[nodiscard]] constexpr auto end() const noexcept -> ConstIterator { return ConstIterator { &_data[0] + _size }; }

        [[nodiscard]] constexpr auto operator[](size_t index) -> T& {
            return _data[index].data;
        }

        template <typename... Args>
        constexpr void emplace_back(Args&&... args) {
            if constexpr (std::is_constructible_v<T, Args...>) {
                std::construct_at(&_data[_size++].data, std::forward<Args>(args)...);
            } else {
                static_assert(std::is_constructible_v<T, Args...>);
            }
        }

        constexpr void push_back() {
            emplace_back();
        }

        // Hack to allow types that don't have copy operations to still be valid.
        template <typename Arg = T>
            requires(
                !std::is_reference_v<Arg>
                && std::same_as<T, Arg>
                && (Utily::Concepts::HasCopyConstructor<Arg> || Utily::Concepts::HasCopyOperator<Arg>))
        constexpr void push_back(const Arg& element) {
            emplace_back(element);
        }

        // Hack to allow types that don't have move operations to still be valid.
        template <typename Arg = T>
            requires(
                !std::is_reference_v<Arg>
                && std::same_as<T, Arg>
                && (Utily::Concepts::HasMoveConstructor<Arg> || Utily::Concepts::HasMoveOperator<Arg>))
        constexpr void push_back(Arg&& element) {
            emplace_back(std::forward<T>(element));
        }

        constexpr ~StaticVector() {
            // for (T& element : (*this)) {
            //     std::destroy_at(&element);
            // }
            std::destroy_n(&_data[0].data, _size);
            //_size = 0;
        }

        [[nodiscard]] constexpr auto front() -> T& {
            return *begin();
        }

        [[nodiscard]] constexpr auto back() -> T& {
            return *(begin() + static_cast<std::ptrdiff_t>(_size) - 1);
        }

        constexpr void resize(int n) noexcept {
            const auto nn = static_cast<std::ptrdiff_t>(n);
            assert(n >= 0 && nn <= S);

            if (nn > _size) [[likely]] {
                std::uninitialized_default_construct_n(&_data[_size].data, nn - _size);
            } else if (nn < _size) [[unlikely]] {
                std::destroy_n(&_data[_size - nn - 1].data, _size - nn);
            }
            _size = nn;
        }
    };
    static_assert(std::contiguous_iterator<StaticVector<int, 10>::Iterator>);
    static_assert(std::contiguous_iterator<StaticVector<int, 10>::ConstIterator>);
    static_assert(std::ranges::range<StaticVector<int, 10>>);
    static_assert(std::ranges::sized_range<StaticVector<int, 10>>);
    static_assert(std::ranges::contiguous_range<StaticVector<int, 10>>);
    static_assert(std::ranges::common_range<StaticVector<int, 10>>);
}