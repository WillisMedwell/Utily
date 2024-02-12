#pragma once
#include "Utily/Reflection.hpp"
#include "Utily/Simd.hpp"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#ifdef _MSC_VER
#include <malloc.h>
#endif

#ifndef UTY_ALWAYS_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define UTY_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define UTY_ALWAYS_INLINE __forceinline
#else
#define UTY_ALWAYS_INLINE inline
#endif
#endif // UTY_ALWAYS_INLINE

namespace Utily {
    class TypeErasedVector
    {
    private:
        // type related.
        std::string_view _type_name;
        size_t _type_alignment;
        size_t _type_size;
        // data related.
        std::ptrdiff_t _data_size;
        std::ptrdiff_t _data_capacity;
        void* _data;
        constexpr static size_t first_element_capacity = 8;

    private:
        UTY_ALWAYS_INLINE void ensure_pushable_capacity() {
            assert(_type_size != 0);
            // doesnt need a realloc
            if (_data_size < _data_capacity) {
                return;
            }
            // allocated data, realloc it.
            if (_data != nullptr) {
#ifdef _MSC_VER
                void* new_data = _aligned_realloc(
                    _data,
                    _type_size * (static_cast<size_t>(_data_capacity) * 2),
                    _type_alignment);
#else
                void* new_data = std::realloc(
                    _data,
                    _type_size * (static_cast<size_t>(_data_capacity) * 2));
#endif
                if (new_data != nullptr) {
                    _data = new_data;
                    _data_capacity = _data_capacity * 2;
                    return;
                }
                throw std::bad_alloc();
            }
            // no allocated data.
#ifdef _MSC_VER
            _data = _aligned_malloc(_type_size * first_element_capacity, _type_alignment);
#else
            _data = std::aligned_alloc(_type_alignment, _type_size * first_element_capacity);
#endif

            _data_capacity = first_element_capacity;
            if (_data == nullptr) {
                throw std::bad_alloc();
            }
        }

    public:
        UTY_ALWAYS_INLINE constexpr TypeErasedVector() noexcept
            : _type_name("")
            , _type_alignment(0)
            , _type_size(0)
            , _data_size(0)
            , _data_capacity(0)
            , _data(nullptr) { }

        template <typename T>
        UTY_ALWAYS_INLINE constexpr TypeErasedVector(T t [[maybe_unused]]) noexcept
            : _type_name(Utily::Reflection::get_type_name<T>())
            , _type_alignment(alignof(T))
            , _type_size(sizeof(T))
            , _data_size(0)
            , _data_capacity(0)
            , _data(nullptr) {
            static_assert(std::is_trivially_destructible_v<T>, "The type must not have a destuctor.");
        }

        template <typename T>
        UTY_ALWAYS_INLINE constexpr TypeErasedVector(T t [[maybe_unused]], size_t n)
            : _type_name(Utily::Reflection::get_type_name<T>())
            , _type_alignment(alignof(T))
            , _type_size(sizeof(T))
            , _data_size(0)
            , _data_capacity(0)
            , _data(nullptr) {
            static_assert(std::is_trivially_destructible_v<T>, "The type must not have a destuctor.");
            resize(n);
        }

        TypeErasedVector(const TypeErasedVector&) = delete;

        constexpr TypeErasedVector(TypeErasedVector&& other) noexcept
            : _type_name(std::exchange(other._type_name, ""))
            , _type_alignment(std::exchange(other._type_alignment, 0))
            , _type_size(std::exchange(other._type_size, 0))
            , _data_size(std::exchange(other._data_size, 0))
            , _data_capacity(std::exchange(other._data_capacity, 0))
            , _data(std::exchange(other._data, nullptr)) { }

        UTY_ALWAYS_INLINE ~TypeErasedVector() {
            if (_data != nullptr) {
#ifdef _MSC_VER
                _aligned_free(_data);
#else
                free(_data);
#endif
                _data = nullptr;
            }
        }

        template <typename T>
        UTY_ALWAYS_INLINE constexpr void set_underlying_type() {
            static_assert(std::is_trivially_destructible_v<T>, "The type must not have a destuctor.");
            assert(_type_alignment == 0 && _type_size == 0 && _type_name == "");
            assert(_data == nullptr && _data_size == 0 && _data_capacity == 0);

            _type_name = Utily::Reflection::get_type_name<T>();
            _type_alignment = alignof(T);
            _type_size = sizeof(T);
        }

        template <typename T>
        UTY_ALWAYS_INLINE void push_back(T&& t) {
            static_assert(!std::is_same_v<T, void>, "Cannot push back void type");
            assert(Utily::Reflection::get_type_name<T>() == _type_name);
            ensure_pushable_capacity();
            std::construct_at(
                reinterpret_cast<T*>(static_cast<std::byte*>(_data) + (_data_size * static_cast<std::ptrdiff_t>(_type_size))),
                std::forward<T>(t));
            ++_data_size;
        }

        template <typename T, typename... Args>
        UTY_ALWAYS_INLINE void emplace_back(Args&&... args) {
            static_assert(!std::is_same_v<T, void>, "Cannot emplace back void type");
            assert(Utily::Reflection::get_type_name<T>() == _type_name);
            ensure_pushable_capacity();
            std::construct_at(
                reinterpret_cast<T*>(static_cast<std::byte*>(_data) + (_data_size * static_cast<std::ptrdiff_t>(_type_size))),
                std::forward<Args>(args)...);
            ++_data_size;
        }

        UTY_ALWAYS_INLINE constexpr void pop_back() {
            assert(_data_size > 0);
            --_data_size;
        }

        [[nodiscard]] UTY_ALWAYS_INLINE auto operator[](std::ptrdiff_t index) -> void* {
            assert(_data != nullptr);
            assert(index < _data_size);

            return static_cast<void*>(
                static_cast<int8_t*>(_data) + (index * static_cast<std::ptrdiff_t>(_type_size)));
        }

        template <typename T>
        [[nodiscard]] UTY_ALWAYS_INLINE auto at(std::ptrdiff_t index) -> T& {
            assert(Utily::Reflection::get_type_name<T>() == _type_name);
            assert(_data != nullptr);
            assert(index < _data_size);
            return *reinterpret_cast<T*>(static_cast<int8_t*>(_data) + (index * _type_size));
        }

        [[nodiscard]] UTY_ALWAYS_INLINE auto size() const noexcept { return static_cast<size_t>(_data_size); }
        [[nodiscard]] UTY_ALWAYS_INLINE auto size_bytes() const noexcept { return static_cast<size_t>(_data_size) * _type_size; }
        [[nodiscard]] UTY_ALWAYS_INLINE auto capacity() const noexcept { return static_cast<size_t>(_data_capacity); }

        template <typename T>
        [[nodiscard]] auto UTY_ALWAYS_INLINE as_span() -> std::span<T> {
            assert(Utily::Reflection::get_type_name<T>() == _type_name);
            return std::span<T>(reinterpret_cast<T*>(_data), reinterpret_cast<T*>(_data) + _data_size);
        }

        UTY_ALWAYS_INLINE void resize(size_t n) {
            assert(_type_size != 0);

            if (n <= static_cast<size_t>(_data_capacity)) {
                _data_size = static_cast<std::ptrdiff_t>(n);
                return;
            }

            if (_data != nullptr) { // allocated data, realloc it.
#ifdef _MSC_VER
                void* new_data = _aligned_realloc(
                    _data,
                    _type_size * n,
                    _type_alignment);
#else
                void* new_data = std::realloc(
                    _data,
                    _type_size * n);
#endif
                if (new_data == nullptr) {
                    throw std::bad_alloc();
                }
                _data = new_data;
            } else { //  un-allocated data, alloc it.
#ifdef _MSC_VER
                _data = _aligned_malloc(_type_size * n, _type_alignment);
#else
                _data = std::aligned_alloc(_type_alignment, _type_size * n);
#endif
                if (_data == nullptr) {
                    throw std::bad_alloc();
                }
            }
            _data_capacity = static_cast<std::ptrdiff_t>(n);
            _data_size = static_cast<std::ptrdiff_t>(n);
            return;
        }
    };

}