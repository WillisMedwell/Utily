#pragma once

#include "Utily/Concepts.hpp"

#include <optional>
#include <type_traits>
#include <variant>

namespace Utily {
    template <typename Value, typename Error>
        requires(!std::same_as<Value, Error>)
    class Result
    {
    private:
        using InternalResult = std::variant<Value, Error>;
        InternalResult _result;

    public:
        constexpr Result() = delete;

        template <typename Arg>
            requires(!std::is_reference_v<Arg>)
        constexpr Result(Arg&& arg) {
            /*
            The constructors have a lot of noise but it means that a type
            without either a move or copy constructor can still be compiled like std::unique_ptr.
            */
            if constexpr (std::same_as<Arg, Value>) {
                static_assert(Utily::Concepts::HasMoveConstructor<Value>, "Result Value type has no move constructor");
                std::construct_at(&_result, std::forward<Value>(arg));
            } else if constexpr (std::same_as<Arg, Error>) {
                static_assert(Utily::Concepts::HasMoveConstructor<Error>, "Result Error type has no move constructor");
                std::construct_at(&_result, std::forward<Error>(arg));
            } else if constexpr (std::is_constructible_v<Value, Arg&&> && std::is_constructible_v<Error, Arg&&>) {
                static_assert(std::is_constructible_v<Value, Arg&&> && std::is_constructible_v<Error, Arg&&>, "Obscure move constructor.");
            } else if constexpr (std::is_constructible_v<Value, Arg&&>) {
                std::construct_at(&_result, static_cast<Value>(arg));
            } else if constexpr (std::is_constructible_v<Error, Arg&&>) {
                std::construct_at(&_result, static_cast<Error>(arg));
            } 
        }

        template <typename Arg>
            requires(!std::is_reference_v<Arg>)
        constexpr Result(const Arg& arg) {
            if constexpr (std::same_as<Arg, Value>) {
                static_assert(Utily::Concepts::HasCopyConstructor<Value>, "Result Value type has no copy constructor");
                std::construct_at(&_result, arg);
            } else if constexpr (std::same_as<Arg, Error>) {
                static_assert(Utily::Concepts::HasCopyConstructor<Error>, "Result Error type has no copy constructor");
                std::construct_at(&_result, arg);
            } else if constexpr (std::is_constructible_v<Value, const Arg&> && std::is_constructible_v<Error, const Arg&>) {
                static_assert(std::is_constructible_v<Value, const Arg&> && std::is_constructible_v<Error, const Arg&>, "Obscure copy constructor.");
            } else if constexpr (std::is_constructible_v<Value, const Arg&>) {
                std::construct_at(&_result, Value{arg});
            } else if constexpr (std::is_constructible_v<Error, const Arg&>) {
                std::construct_at(&_result, Error{arg});
            }
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return std::holds_alternative<Value>(_result);
        }
        [[nodiscard]] constexpr bool has_error() const noexcept {
            return std::holds_alternative<Error>(_result);
        }

        [[nodiscard]] constexpr auto value() -> Value& {
            return std::get<Value>(_result);
        }
        [[nodiscard]] constexpr auto value() const -> const Value& {
            return std::get<Value>(_result);
        }
        [[nodiscard]] constexpr auto error() -> Error& {
            return std::get<Error>(_result);
        }
        [[nodiscard]] constexpr auto error() const -> const Error& {
            return std::get<Error>(_result);
        }

        template <typename Pred>
            requires Utily::Concepts::IsConstCallableWith<Pred, Value>
        constexpr auto on_value(Pred pred) const noexcept -> const Result& {
            if (has_value()) {
                pred(std::get<Value>(_result));
            }
            return *this;
        }

        template <typename Pred>
            requires Utily::Concepts::IsCallableWith<Pred, Value>
        constexpr auto on_value(Pred pred) noexcept -> Result& {
            if (has_value()) {
                pred(std::get<Value>(_result));
            }
            return *this;
        }

        template <typename Pred>
            requires Utily::Concepts::IsConstCallableWith<Pred, Error>
        constexpr auto on_error(Pred pred) const noexcept -> const Result& {
            if (has_error()) {
                pred(std::get<Error>(_result));
            }
            return *this;
        }

        template <typename Pred>
            requires Utily::Concepts::IsCallableWith<Pred, Error>
        constexpr auto on_error(Pred pred) noexcept -> Result& {
            if (has_error()) {
                pred(std::get<Error>(_result));
            }
            return *this;
        }

        template <typename ValuePred, typename ErrorPred>
            requires Utily::Concepts::IsCallableWith<ValuePred, Value>
            && Utily::Concepts::IsCallableWith<ErrorPred, Error>
        constexpr auto on_either(ValuePred value_pred, ErrorPred error_pred) -> Result& {
            if (std::holds_alternative<Value>(_result)) {
                value_pred(std::get<Value>(_result));
            } else {
                error_pred(std::get<Error>(_result));
            }
            return *this;
        }

        template <typename ValuePred, typename ErrorPred>
            requires Utily::Concepts::IsConstCallableWith<ValuePred, Value>
            && Utily::Concepts::IsConstCallableWith<ErrorPred, Error>
        constexpr auto on_either(ValuePred value_pred, ErrorPred error_pred) const noexcept -> const Result& {
            if (std::holds_alternative<Value>(_result)) {
                value_pred(std::get<Value>(_result));
            } else {
                error_pred(std::get<Error>(_result));
            }
            return *this;
        }
    };

    template <typename Error>
        requires(Utily::Concepts::HasCopyConstructor<Error> && Utily::Concepts::HasMoveConstructor<Error>)
    class Result<void, Error>
    {
    private:
        std::optional<Error> _result;

    public:
        constexpr Result()
            : _result(std::nullopt) { }

        template <typename Arg>
            requires(!std::is_reference_v<Arg>)
        constexpr Result(const Arg& arg) {
            if constexpr (std::same_as<Arg, Error>) {
                static_assert(Utily::Concepts::HasCopyConstructor<Error>, "Result Error type has no copy constructor");
                std::construct_at(&_result, arg);
            } else {
                static_assert(std::same_as<Arg, Error>, "Obscure copy constructor");
            }
        }

        template <typename Arg>
            requires(!std::is_reference_v<Arg>)
        constexpr Result(Arg&& arg) {
            if constexpr (std::same_as<Arg, Error>) {
                static_assert(Utily::Concepts::HasMoveConstructor<Error>, "Result Error type has no move constructor");
                std::construct_at(&_result, std::forward<Error>(arg));
            } else {
                static_assert(std::same_as<Arg, Error>, "Obscure move constructor");
            }
        }

        [[nodiscard]] constexpr bool has_value() const noexcept {
            return !_result.has_value();
        }
        [[nodiscard]] constexpr bool has_error() const noexcept {
            return _result.has_value();
        }

        [[nodiscard]] constexpr auto error() -> Error& {
            return _result.value();
        }
        [[nodiscard]] constexpr auto error() const -> const Error& {
            return _result.value();
        }

        template <typename Pred>
            requires Utily::Concepts::IsConstCallableWith<Pred, Error>
        constexpr auto on_error(Pred pred) const noexcept -> const Result& {
            if (has_error()) {
                pred(error());
            }
            return *this;
        }

        template <typename Pred>
            requires Utily::Concepts::IsCallableWith<Pred, Error>
        constexpr auto on_error(Pred pred) noexcept -> Result& {
            if (has_error()) {
                pred(error());
            }
            return *this;
        }
    };
}