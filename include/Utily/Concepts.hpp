#pragma once

#include <list>
#include <memory>
#include <string>
#include <ranges>

namespace Utily {
    namespace Concepts {
        template <typename T>
        concept HasMoveConstructor = requires(T&& t) {
            {
                T { std::forward<T>(t) }
            } -> std::same_as<T>;
        };
        static_assert(HasMoveConstructor<std::string>);

        template <typename T>
        concept HasMoveOperator = requires(T t, T&& move) {
            {
                t = std::forward<T>(move)
            } -> std::same_as<T&>;
        };
        static_assert(HasMoveOperator<std::string>);

        template <typename T>
        concept HasCopyConstructor = requires(const T& t) {
            {
                T { t }
            } -> std::same_as<T>;
        };
        static_assert(HasCopyConstructor<std::string>);
        static_assert(!HasCopyConstructor<std::unique_ptr<int>>);

        template <typename T>
        concept HasCopyOperator = requires(T t, const T& copy) {
            {
                t = copy
            } -> std::same_as<T&>;
        };
        static_assert(HasCopyOperator<std::string>);

        template <typename T>
        concept IsContiguousRange =
            std::ranges::contiguous_range<T> && std::ranges::sized_range<T> && requires(T a) {
                { std::ranges::data(a) } -> std::contiguous_iterator;
            };

        static_assert(IsContiguousRange<std::string>);
        static_assert(!IsContiguousRange<std::list<int>>);

        // template<typename , typename

        template <typename T, typename... Param>
        concept IsCallableWith = requires(T t, Param... param) {
            {
                t(param...)
            };
        };

        template <typename T, typename... Param>
        concept IsConstCallableWith = requires(T t, const Param&... param) {
            {
                t(param...)
            };
        };

        template <typename Iter>
        concept SubrangeCompatible = std::contiguous_iterator<Iter> && requires(Iter a, Iter b) {
            { std::ranges::subrange(a, b) } -> std::convertible_to<std::ranges::subrange<Iter>>;
        };
    }

} // namespace Utily
