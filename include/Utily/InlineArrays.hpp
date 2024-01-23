
#pragma once

#include "Utily/TupleAlgo.hpp"
#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <span>

namespace Utily {

    class InlineArrays
    {
        using Owner = std::unique_ptr<std::byte[]>;

    public:
        template <typename T>
        static auto is_aligned(auto* ptr) {
            return reinterpret_cast<std::uintptr_t>(ptr) % alignof(T) == 0;
        }

        template <typename... Types, std::integral... Size>
            requires(sizeof...(Size) == sizeof...(Types))
        static auto alloc_uninit(Size... array_size) {
            constexpr static size_t padding_for_alignment = (alignof(Types) + ...);

            const auto size = padding_for_alignment + ((sizeof(Types) * static_cast<size_t>(array_size)) + ...);
            Owner data = std::make_unique_for_overwrite<std::byte[]>(size);

            const auto array_sizes = std::to_array({ (static_cast<std::ptrdiff_t>(array_size))... });

            auto size_iter = array_sizes.begin();
            std::byte* data_ptr = data.get();

            auto set_up_span = [&]<typename T>(std::span<T>& span) {
                while (!is_aligned<T>(data_ptr)) {
                    ++data_ptr;
                }
                T* begin = reinterpret_cast<T*>(data_ptr);
                T* end = std::next(begin, *size_iter);
                span = std::span<T> { begin, end };
                data_ptr = reinterpret_cast<std::byte*>(end);
                ++size_iter;
            };
            using Spans = std::tuple<std::span<Types>...>;
            Spans spans;
            Utily::TupleAlgo::for_each(spans, set_up_span);

            if (data_ptr >= data_ptr + size) {
                throw std::runtime_error("Overflowed buffer");
            }

            return std::tuple_cat(std::tuple<Owner>(std::move(data)), spans);
        }

        template <typename... Types, std::integral... Size>
            requires(std::is_default_constructible_v<Types> && ...)
            && (sizeof...(Size) == sizeof...(Types))
        static auto alloc_dafault(Size... array_size) {
            auto owner_and_spans = alloc_uninit<Types...>(array_size...);


            Utily::TupleAlgo::for_each(owner_and_spans,
                [](auto& span) {
                    using Type = std::decay_t<decltype(span)>;
                    if constexpr (!std::same_as<Owner, Type>) {
                        std::ranges::uninitialized_default_construct(span);
                    }
                });

            return owner_and_spans;
        }

    private:
        template <typename TupleRangeIn, typename TupleDest, size_t Max, size_t Index = 0>
        constexpr static void uninit_copy(TupleRangeIn& in, TupleDest& dest) {
            if constexpr (Index == Max) {
                return;
            } else {
                std::ranges::uninitialized_copy(std::get<Index>(in), std::get<Index + 1>(dest));
                return uninit_copy<TupleRangeIn, TupleDest, Max, Index + 1>(in, dest);
            }
        }

    public:
        template <typename... Types, std::ranges::range... Range>
            requires(sizeof...(Types) == sizeof...(Range))
            && ((!std::is_reference_v<Range>) && ...)
        auto static alloc_copy(const Range&... range) {
            auto owner_and_spans = alloc_uninit<Types...>(range.size()...);
            const auto ranges = std::make_tuple(range...);

            uninit_copy<decltype(ranges), decltype(owner_and_spans), sizeof...(Range), 0>(ranges, owner_and_spans);
            return owner_and_spans;
        }
        template <std::ranges::range... Range>
        auto static alloc_copy(const Range&... range) {
            return alloc_copy<std::ranges::range_value_t<Range>...>(range...);
        }
    };
}