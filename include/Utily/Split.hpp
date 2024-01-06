#pragma once

#include <algorithm>
#include <array>
#include <execution>
#include <iterator>
#include <numeric>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include <Utily/Concepts.hpp>

namespace Utily {

    namespace Split {

        template <std::ranges::range Container, typename Delim = std::ranges::range_value_t<Container>>
            requires std::equality_comparable_with<Delim, std::ranges::range_value_t<Container>> && (!std::is_reference_v<Container>)
        class ByElement
        {
            using ContainerIter = Container::const_iterator;
            using ContainerValue = std::ranges::range_value_t<Container>;

            const Container& _container;
            const Delim _delim;

        public:
            constexpr ByElement(const Container& container, const Delim& delim)
                : _container(container)
                , _delim(delim) { }

            ByElement(Container&& container, const Delim& delim) = delete;

            struct Iterator {
            public:
                ContainerIter current_begin;
                ContainerIter current_end;
                ContainerIter end;
                Delim delim;

                constexpr auto operator++() noexcept -> Iterator& {
                    if (current_end != end) {
                        current_begin = std::find_if_not(current_end, end, [&](auto element) { return element == delim; });
                        current_end = std::find(current_begin, end, delim);
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
                // purely for type deduction
                consteval static auto dereference_type() {
                    if constexpr (std::same_as<ContainerValue, char>) {
                        return std::string_view {};
                    } else if constexpr (Utily::Concepts::IsContiguousRange<Container>) {
                        return std::span<const ContainerValue> {};
                    } else {
                        return std::ranges::subrange<ContainerIter, ContainerIter> {};
                    }
                }
                using DereferenceType = std::decay_t<decltype(dereference_type())>;

            public:
                [[nodiscard]] constexpr auto
                operator*() const noexcept {
                    return DereferenceType { current_begin, current_end };
                }

                using difference_type = std::ptrdiff_t;
                using value_type = DereferenceType;
                using reference = std::add_lvalue_reference_t<value_type>;
                using pointer = std::add_pointer_t<value_type>;
                using iterator_category = std::forward_iterator_tag;

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

        template <std::ranges::range Container, size_t S, typename Delim = std::ranges::range_value_t<Container>>
            requires(!std::is_reference_v<Container>)
        class ByElements
        {
        private:
            using ContainerIter = Container::const_iterator;
            using ContainerValue = std::ranges::range_value_t<Container>;
            using Delims = std::array<Delim, S>;
            const Container& _container;
            const Delims _delims;

        public:
            constexpr ByElements(const Container& container, const Delims& delims)
                : _container(container)
                , _delims(delims) { }

            struct Iterator {
            public:
                ContainerIter current_begin;
                ContainerIter current_end;
                ContainerIter end;
                const Delims& delims;

                constexpr auto operator++() noexcept -> Iterator& {
                    auto is_delimiter = [&](const auto& element) {
                        return std::ranges::find(delims, element) != delims.end();
                    };
                    if (current_end != end) {
                        current_begin = std::ranges::find_if_not(current_end, end, is_delimiter);
                        current_end = std::ranges::find_if(current_begin, end, is_delimiter);
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
                // purely for type deduction
                consteval static auto deference_type() {
                    if constexpr (std::same_as<ContainerValue, char>) {
                        return std::string_view {};
                    } else if constexpr (Utily::Concepts::IsContiguousRange<Container>) {
                        return std::span<const ContainerValue> {};
                    } else {
                        return std::ranges::subrange<ContainerIter, ContainerIter> {};
                    }
                }
                using DerefenceType = decltype(deference_type());

            public:
                [[nodiscard]] constexpr auto operator*() const noexcept {
                    return DerefenceType { current_begin, current_end };
                }

                using difference_type = std::ptrdiff_t;
                using value_type = DerefenceType;
                using reference = std::add_lvalue_reference_t<value_type>;
                using pointer = std::add_pointer_t<value_type>;
                using iterator_category = std::forward_iterator_tag;

                [[nodiscard]] constexpr auto operator==(const Iterator& other) const noexcept {
                    return this->current_begin == other.current_begin && this->current_end == other.current_end && this->end == other.end && this->delims == other.delims;
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
                    .delims = _delims
                };
            }
            [[nodiscard]] constexpr auto end() const noexcept {
                return Iterator {
                    .current_begin = _container.cend(),
                    .current_end = _container.cend(),
                    .end = _container.cend(),
                    .delims = _delims
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

    }

    template <std::ranges::range Container, typename... Args>
        requires(!std::is_reference_v<Container>)
    auto split(const Container& container, Args&&... args) {
        using FirstArg = std::tuple_element_t<0, std::tuple<Args...>>;

        constexpr static bool use_split_by_element = sizeof...(Args) == 1 && std::equality_comparable_with<std::ranges::range_value_t<Container>, FirstArg>;
        constexpr static bool use_split_by_elements = std::equality_comparable_with<std::ranges::range_value_t<Container>, FirstArg>;

        if constexpr (use_split_by_element) {
            return Split::ByElement(container, args...);
        } else if constexpr (use_split_by_elements) {
            return Split::ByElements(container, std::to_array({ args... }));
        }
        static_assert(use_split_by_element || use_split_by_elements, "Obsure split operation");
    }

    template <std::ranges::range Container, typename... Args>
        requires(!std::is_reference_v<Container>)
    auto split(Container&& container, Args&&... args) = delete;
}