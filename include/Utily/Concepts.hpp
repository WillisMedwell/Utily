#pragma once

#include <list>
#include <memory>
#include <string>

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
        concept IsContiguousRange = requires(T t) {
            {
                t.begin()
            };
            {
                t.end()
            };
            {
                t.data()
            };
        };
        static_assert(IsContiguousRange<std::string>);
        static_assert(!IsContiguousRange<std::list<int>>);

        // template<typename , typename

        template <typename T, typename Param>
        concept IsCallableWith = requires(T t, Param param) {
            {
                t(param)
            };
        };

        template <typename T, typename Param>
        concept IsConstCallableWith = requires(T t, const Param& param) {
            {
                t(param)
            };
        };
    }

} // namespace Utily
