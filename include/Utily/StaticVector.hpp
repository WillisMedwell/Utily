#pragma once

#include <array>
#include <concepts>
#include <tuple>
#include <variant>

#include "Utily/Concepts.hpp"
#include "Utily/TupleAlgo.hpp"

namespace Utily {
    template <typename T, size_t S>
        requires Utily::Concepts::HasCopyConstructor<T>
        && Utily::Concepts::HasMoveConstructor<T>
        && Utily::Concepts::HasMoveOperator<T>
        && Utily::Concepts::HasCopyOperator<T>
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

        InternalT _data[S];
        size_t _size;

    public:
        using value_type = T;

        constexpr StaticVector()
            : _data()
            , _size(0) {
        }

        [[nodiscard]] constexpr auto capacity() const noexcept -> size_t { return S; }
        [[nodiscard]] constexpr auto size() const noexcept -> size_t { return _size; }

        struct Iterator {
            using iterator_category = std::contiguous_iterator_tag;
            using value_type = T;
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
            requires ((!std::is_reference_v<Args>) && ...)
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
            requires ((!std::is_reference_v<Args>) && ...)
        constexpr void emplace_back(Args&&... args) {
            std::construct_at(&_data[_size++].data, std::forward<Args>(args)...);
        }
        template<typename... Args>
             requires ((!std::is_reference_v<Args>) && ...)
        constexpr void emplace_back(const Args&... args) {
            std::construct_at(&_data[_size++].data, args...);
        } 
        constexpr void emplace_back() {
            std::construct_at(&_data[_size++].data);
        }

        constexpr void push_back() {
            emplace_back();
        }
        constexpr void push_back(const T& element) {
            emplace_back(element);
        }
        constexpr void push_back(T&& element) {
            emplace_back(std::forward<T>(element));
        }

        constexpr ~StaticVector() {
            for (T& element : (*this)) {
                std::destroy_at(&element);
            }
            _size = 0;
        }

        [[nodiscard]] constexpr auto front() -> T& {
            return *begin();
        }

        [[nodiscard]] constexpr auto back() -> T& {
            return *(begin() + static_cast<std::ptrdiff_t>(_size) - 1);
        }
    };
    // static_assert(std::contiguous_iterator<StaticVector<int, 10>::Iterator>);
    // static_assert(std::contiguous_iterator<StaticVector<int, 10>::ConstIterator>);
    // static_assert(std::ranges::range<StaticVector<int, 10>>);
    // static_assert(std::ranges::sized_range<StaticVector<int, 10>>);
    // static_assert(std::ranges::contiguous_range<StaticVector<int, 10>>);
    // static_assert(std::ranges::common_range<StaticVector<int, 10>>);
}