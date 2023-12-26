#pragma once

#include <algorithm>
#include <iterator>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include <Utily/Concepts.hpp>

namespace Utily {

    template <std::ranges::range Container>
    class SplitByElement
    {
        using ContainerIter = Container::const_iterator;
        using ContainerValue = std::ranges::range_value_t<Container>;

        const Container& _container;
        ContainerValue _delim;

    public:
        constexpr SplitByElement(const Container& container, const ContainerValue& delim)
            : _container(container)
            , _delim(delim) { }

        struct Iterator {
        public:
            ContainerIter current_begin;
            ContainerIter current_end;
            ContainerIter end;
            ContainerValue delim;

            constexpr auto operator++() noexcept -> Iterator& {
                if (current_end != end) {
                    current_begin = std::ranges::find_if_not(
                        current_end, end, [&](const auto& element) { return element == delim; });
                    current_end = std::ranges::find(current_begin, end, delim);
                } else {
                    current_begin = end;
                }
                return *this;
            }

            constexpr auto operator++(int) noexcept -> Iterator {
                Iterator copy = (*this);
                ++(*this);
                return copy;
            }

        private:
            consteval static auto deference_type() {
                if constexpr (std::same_as<ContainerValue, char>) {
                    return std::string_view {};
                } else if constexpr (Utily::Concepts::IsContiguousRange<Container>) {
                    return std::span<const ContainerValue> {};
                } else {
                    return std::ranges::subrange<
                    std::ranges::iterator_t<Container>, 
                    std::ranges::iterator_t<Container>> { };
                }
            }
            using DerefenceType = decltype(deference_type());

        public:
            [[nodiscard]] constexpr auto
            operator*() const noexcept {
                if constexpr (std::same_as<ContainerValue, char>) {
                    return std::string_view { current_begin, current_end };
                } else if constexpr (Utily::Concepts::IsContiguousRange<Container>) {
                    return std::span<const ContainerValue> { current_begin, current_end };
                } else {
                    return std::ranges::subrange { current_begin, current_end };
                }
            }

            using difference_type = std::ptrdiff_t;
            using value_type = DerefenceType;
            using reference = std::add_lvalue_reference_t<value_type>;
            using pointer = std::add_pointer_t<value_type>;

            [[nodiscard]] constexpr auto operator==(const Iterator& other) const noexcept {
                return this->current_begin == other.current_begin && this->current_end == other.current_end && this->end == other.end && this->delim == other.delim;
            }
            [[nodiscard]] constexpr auto operator!=(const Iterator& other) const noexcept {
                return !(*this == other);
            }
        };

        using const_iterator = Iterator;

        [[nodiscard]] constexpr auto begin() const noexcept {
            return ++Iterator {
                .current_begin = _container.cbegin(),
                .current_end = _container.cbegin(),
                .end = _container.cend(),
                .delim = _delim
            };
        }
        [[nodiscard]] constexpr auto end() const noexcept {
            return Iterator {
                .current_begin = _container.cend(),
                .current_end = _container.cend(),
                .end = _container.cend(),
                .delim = _delim
            };
        }
        [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
        [[nodiscard]] constexpr auto cend() const noexcept { return end(); }

        [[nodiscard]] constexpr auto evaluate() const noexcept {
            using SplitType = decltype(*this->begin());
            std::vector<SplitType> evaluated;
            std::copy(this->cbegin(), this->cend(), std::back_inserter(evaluated));
            return evaluated;
        }
    };

    template <std::ranges::range Container>
    class SplitByElements
    {
    };

    template <std::ranges::range Container, typename... Args>
    auto split(const Container& container, Args&&... args) {
        using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;

        constexpr static bool use_split_by_element = sizeof...(Args) == 1 && std::same_as<std::ranges::range_value_t<Container>, FirstArg>;
        constexpr static bool use_split_by_elements = std::same_as<std::ranges::range_value_t<Container>, FirstArg>;

        if constexpr (use_split_by_element) {
            return SplitByElement(container, args...);
        }
        static_assert(use_split_by_element || use_split_by_elements, "Obsure split operation");
    }

}