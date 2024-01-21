#pragma once

#include <concepts>
#include <ranges>

#include <bit>
#include <bitset>
#include <emmintrin.h>
#include <immintrin.h>
#include <type_traits>
#include <xmmintrin.h>

// #define UTILY_SIMD_USE_MEMCPY

namespace Utily::Simd {

    inline auto find_contiguous_char_128(const char* begin, const char* end, char value) -> std::ptrdiff_t {

        constexpr static size_t chars_per_vec = 128 / 8;
        const size_t s = static_cast<size_t>(end - begin);
        const size_t max_i_clamped = s - (s % chars_per_vec);

        const __m128i v = _mm_set1_epi8(value);

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m128i c = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(begin + i));
            const __m128i eq = _mm_cmpeq_epi8(c, v);
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i + std::countr_zero(eq_bits));
            }
        }

        __m128i c = v;
#ifndef UTILY_SIMD_USE_MEMCPY
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(s - max_i_clamped); ++i) {
            *(reinterpret_cast<char*>(&c) + i) = *(begin + max_i_clamped + i);
        }
#else
        std::memcpy(&c, (begin + max_i_clamped), s - max_i_clamped);
#endif
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(c, v)));
        return static_cast<std::ptrdiff_t>(max_i_clamped + std::countr_zero(eq_bits));
    }

    template <size_t S>
    inline auto find_contiguous_chars_128(const char* begin, const char* end, std::array<char, S> values) -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 128 / 8;
        const size_t s = static_cast<size_t>(end - begin);
        const size_t max_i_clamped = s - (s % chars_per_vec);

        // this should get unrolled.
        std::array<__m128i, S> vs;
        for (size_t i = 0; i < S; ++i) {
            vs[i] = _mm_set1_epi8(values[i]);
        }

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m128i c = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(begin + i));
            std::array<__m128i, S> eqs;
            for (size_t i = 0; i < S; ++i) {
                eqs[i] = _mm_cmpeq_epi8(c, vs[i]);
            }
            __m128i eq = _mm_setzero_ps();
            for (size_t i = 0; i < S; ++i) {
                eq = _mm_or_si128(eq, eqs[i]);
            }
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i + std::countr_zero(eq_bits));
            }
        }

        __m128i c = vs[0];
#ifndef UTILY_SIMD_USE_MEMCPY
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(s - max_i_clamped); ++i) {
            *(reinterpret_cast<char*>(&c) + i) = *(begin + max_i_clamped + i);
        }
#else
        std::memcpy(&c, (begin + max_i_clamped), s - max_i_clamped);
#endif
        std::array<__m128i, S> eqs;
        for (size_t i = 0; i < S; ++i) {
            eqs[i] = _mm_cmpeq_epi8(c, vs[i]);
        }
        __m128i eq = _mm_setzero_ps();
        for (size_t i = 0; i < S; ++i) {
            eq = _mm_or_si128(eq, eqs[i]);
        }
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped + std::countr_zero(eq_bits));
    }

    inline auto find_contiguous_char_256(const char* begin, const char* end, char value) -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 32;
        const size_t s = static_cast<size_t>(end - begin);
        const size_t max_i_clamped = s - (s % chars_per_vec);

        const __m256i v = _mm256_set1_epi8(value);

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m256i c = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(begin + i));
            const __m256i eq = _mm256_cmpeq_epi8(c, v);
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm256_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i + std::countr_zero(eq_bits));
            }
        }

        __m256i c = v;
#ifndef UTILY_SIMD_USE_MEMCPY
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(s - max_i_clamped); ++i) {
            *(reinterpret_cast<char*>(&c) + i) = *(begin + max_i_clamped + i);
        }
#else
        std::memcpy(&c, (begin + max_i_clamped), s - max_i_clamped);
#endif

        const __m256i eq = _mm256_cmpeq_epi8(c, v);
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm256_movemask_epi8(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped + std::countr_zero(eq_bits));
    }

    inline auto find_contiguous_char_512(const char* begin, const char* end, char value) -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 64;
        const size_t s = static_cast<size_t>(end - begin);
        const size_t max_i_clamped = s - (s % chars_per_vec);

        const __m512i v = _mm512_set1_epi8(value);

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m512i c = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(begin + i));
            uint64_t eq_bits = std::bit_cast<uint64_t>(_mm512_cmpeq_epi8_mask(c, v));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i + std::countr_zero(eq_bits));
            }
        }

        __m512i c = v;
#ifndef UTILY_SIMD_USE_MEMCPY
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(s - max_i_clamped); ++i) {
            *(reinterpret_cast<char*>(&c) + i) = *(begin + max_i_clamped + i);
        }
#else
        std::memcpy(&c, (begin + max_i_clamped), s - max_i_clamped);
#endif
        uint64_t eq_bits = std::bit_cast<uint64_t>(_mm512_cmpeq_epi8_mask(c, v));
        return static_cast<std::ptrdiff_t>(max_i_clamped + std::countr_zero(eq_bits));
    }

    inline auto find_contiguous_int32_128(const int32_t* begin, const int32_t* end, int32_t value) -> std::ptrdiff_t {
        using Vec = __m128i;
        using Type = int32_t;

        constexpr static size_t ints_per_vec = 128 / 32;
        const size_t s = static_cast<size_t>(end - begin);
        const size_t max_i_clamped = s - (s % ints_per_vec);

        const Vec v = _mm_set1_epi32(value);

        for (size_t i = 0; i < max_i_clamped; i += ints_per_vec) {
            const Vec c = _mm_lddqu_si128(reinterpret_cast<const Vec*>(begin + i));
            auto eqi = _mm_cmpeq_epi32(c, v);
            auto eq = _mm_castsi128_ps(eqi);
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_ps(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i + std::countr_zero(eq_bits));
            }
        }

        Vec c = v;
#ifndef UTILY_SIMD_USE_MEMCPY
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(s - max_i_clamped); ++i) {
            *(reinterpret_cast<Type*>(&c) + i) = *(begin + max_i_clamped + i);
        }
#else
        std::memcpy(&c, (begin + max_i_clamped), s - max_i_clamped);
#endif
        auto eqi = _mm_cmpeq_epi32(c, v);
        auto eq = _mm_castsi128_ps(eqi);
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_ps(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped + std::countr_zero(eq_bits));
    }

    inline auto find_contiguous_int32_512(const int32_t* begin, const int32_t* end, int32_t value) -> std::ptrdiff_t {
        using Vec = __m512i;
        using Type = int32_t;

        constexpr static size_t ints_per_vec = 512 / 32;
        const size_t s = static_cast<size_t>(end - begin);
        const size_t max_i_clamped = s - (s % ints_per_vec);

        const Vec v = _mm512_set1_epi32(value);

        for (size_t i = 0; i < max_i_clamped; i += ints_per_vec) {
            const Vec c = _mm512_loadu_si512(reinterpret_cast<const Vec*>(begin + i));
            uint16_t eq_bits = std::bit_cast<uint16_t>(_mm512_cmpeq_epi32_mask(c, v));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i + std::countr_zero(eq_bits));
            }
        }

        Vec c = v;
#ifndef UTILY_SIMD_USE_MEMCPY
        for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(s - max_i_clamped); ++i) {
            *(reinterpret_cast<Type*>(&c) + i) = *(begin + max_i_clamped + i);
        }
#else
        std::memcpy(&c, (begin + max_i_clamped), s - max_i_clamped);
#endif
        uint16_t eq_bits = std::bit_cast<uint16_t>(_mm512_cmpeq_epi32_mask(c, v));
        return static_cast<std::ptrdiff_t>(max_i_clamped + std::countr_zero(eq_bits));
    }

    // inline auto is_equal_contiguous_char(const char* begin, const char* end, const char* value_begin, const char* value_end) {

    // }

    template <std::contiguous_iterator Iter, typename Value>
    auto find(Iter begin, Iter end, Value value) noexcept {
        if constexpr (sizeof(std::iter_value_t<Iter>) == 1 && sizeof(Value) == 1) {
            auto data = reinterpret_cast<const char*>(&(*begin));
            auto& val = *reinterpret_cast<const char*>(&value);
            return begin + find_contiguous_char_512(data, data + std::distance(begin, end), val);
        } else if constexpr (sizeof(std::iter_value_t<Iter>) == 4 && sizeof(Value) == 4) {
            auto data = reinterpret_cast<const int32_t*>(&(*begin));
            auto& val = *reinterpret_cast<const int32_t*>(&value);
            return begin + find_contiguous_int32_512(data, data + std::distance(begin, end), val);
        } else {
            return std::find(begin, end, value);
        }
    }
}