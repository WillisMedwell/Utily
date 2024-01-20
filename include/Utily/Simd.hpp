#pragma once

#include <concepts>
#include <ranges>

#include <bit>
#include <bitset>
#include <emmintrin.h>
#include <type_traits>

namespace Utily::Simd {
    template <typename Iter, typename Value>
        requires(
            std::contiguous_iterator<Iter>
            && (sizeof(std::iter_value_t<Iter>) == 1)
            && (sizeof(Value) == 1))
    auto find(Iter begin, Iter end, Value value) {
        using IterValue = std::remove_cvref_t<std::iter_value_t<Iter>>;

        char* b = reinterpret_cast<char*>(const_cast<IterValue*>(&(*begin)));
        auto data = std::span<char> { b, static_cast<size_t>(std::distance(begin, end)) };

        constexpr size_t n = 128 / 8;
        const size_t s = data.size();
        const size_t max_i_clamped = s - (s % n);

        const __m128i v = _mm_set1_epi8(*reinterpret_cast<char*>(&value));
        __m128i c;
        __m128i eq;
        for (size_t i = 0; i < max_i_clamped; i += n) {
            c = _mm_loadu_si128(reinterpret_cast<__m128i*>(data.data() + i));
            eq = _mm_cmpeq_epi8(c, v);
            uint32_t eq_bits = _mm_movemask_epi8(eq);
            if (eq_bits != 0) {
                size_t offset = i + std::countr_zero(eq_bits);
                return begin + offset;
            }
        }

        c = v;
        std::memcpy(&c, reinterpret_cast<__m128i*>(data.data() + max_i_clamped), s - max_i_clamped);
        eq = _mm_cmpeq_epi8(c, v);
        uint32_t eq_bits = _mm_movemask_epi8(eq);
        size_t offset = max_i_clamped + std::countr_zero(eq_bits);
        return begin + offset;
    }
}