#pragma once

#include <concepts>
#include <ranges>

#include <algorithm>
#include <bit>
#include <bitset>
#include <type_traits>

#if defined(__GNUC__) || defined(__clang__)
#define UTY_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define UTY_ALWAYS_INLINE __forceinline
#else
#define UTY_ALWAYS_INLINE inline
#endif

#include <array>
#include <cassert>
#include <cstring>
#include <iostream>

// Check for SSE support
#if defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__)
#define UTY_SUPPORTS_128 1
#include <emmintrin.h>
#include <xmmintrin.h>
#else
#define UTY_SUPPORTS_128 0
#endif

// Check for AVX support
#if defined(__AVX__) && !defined(EMSCRIPTEN)
#define UTY_SUPPORTS_256 1
#include <immintrin.h>
#else
#define UTY_SUPPORTS_256 0
#endif

// Check for AVX-512 support
#if defined(__AVX512F__) && !defined(EMSCRIPTEN)
#define UTY_SUPPORTS_512 1
#include <immintrin.h>
#else
#define UTY_SUPPORTS_512 0
#endif

#if !defined(UTY_USE_SIMD_128) && !defined(UTY_USE_SIMD_256) && !defined(UTY_USE_SIMD_512) && !defined(UTY_NO_SIMD)
#if UTY_SUPPORTS_512
#define UTY_USE_SIMD_512
#elif UTY_SUPPORTS_256
#define UTY_USE_SIMD_256
#elif UTY_SUPPORTS_128
#define UTY_USE_SIMD_128
#endif
#endif

namespace Utily::Simd::Details {
    /*
        find -> char
    */
#if UTY_SUPPORTS_512 || defined(UTY_USE_SIMD_512)
    UTY_ALWAYS_INLINE auto find_512(const char* src_begin, const size_t src_size, char value) -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 64;
        const size_t max_i_clamped = src_size - (src_size % chars_per_vec);

        const __m512i v = _mm512_set1_epi8(value);

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m512i c = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src_begin + i));
            uint64_t eq_bits = std::bit_cast<uint64_t>(_mm512_cmpeq_epi8_mask(c, v));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }

        __m512i c = v;
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - max_i_clamped);
        uint64_t eq_bits = std::bit_cast<uint64_t>(_mm512_cmpeq_epi8_mask(c, v));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
#if UTY_SUPPORTS_256 || defined(UTY_USE_SIMD_256)
    UTY_ALWAYS_INLINE auto find_256(const char* src_begin, const size_t src_size, char value) -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 32;

        const size_t max_i_clamped = src_size - (src_size % chars_per_vec);

        const __m256i v = _mm256_set1_epi8(value);

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m256i c = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src_begin + i));
            const __m256i eq = _mm256_cmpeq_epi8(c, v);
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm256_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }

        __m256i c = v;
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - max_i_clamped);
        const __m256i eq = _mm256_cmpeq_epi8(c, v);
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm256_movemask_epi8(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
#if UTY_SUPPORTS_128 || defined(UTY_USE_SIMD_128)
    UTY_ALWAYS_INLINE auto find_128(const char* src_begin, const size_t src_size, char value) -> std::ptrdiff_t {

        constexpr static size_t chars_per_vec = 128 / 8;

        const size_t max_i_clamped = src_size - (src_size % chars_per_vec);

        const __m128i v = _mm_set1_epi8(value);

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m128i c = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i));
            const __m128i eq = _mm_cmpeq_epi8(c, v);
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }

        __m128i c = v;
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - max_i_clamped);
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(c, v)));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
    UTY_ALWAYS_INLINE auto find(const char* src_begin, const size_t src_size, char value) noexcept -> std::ptrdiff_t {
#if defined(UTY_USE_SIMD_512) || defined(UTY_USE_SIMD_512)
        return find_512(src_begin, src_size, value);
#elif defined(UTY_USE_SIMD_256) || defined(UTY_USE_SIMD_256)
        return find_256(src_begin, src_size, value);
#elif defined(UTY_USE_SIMD_128) || defined(UTY_USE_SIMD_128)
        return find_128(src_begin, src_size, value);
#elif defined(UTY_NO_SIMD)
        return std::distance(src_begin, std::find(src_begin, src_begin + src_size, value));
#else
        return std::distance(src_begin, std::find(src_begin, src_begin + src_size, value));
#endif
    }

    /*
        find -> int32_t
    */
#if UTY_SUPPORTS_512 || defined(UTY_USE_SIMD_512)
    UTY_ALWAYS_INLINE auto find_512(const int32_t* src_begin, size_t src_size, int32_t value) -> std::ptrdiff_t {
        using Vec = __m512i;

        constexpr static size_t ints_per_vec = 512 / 32;
        const size_t max_i_clamped = src_size - (src_size % ints_per_vec);

        const Vec v = _mm512_set1_epi32(value);

        for (size_t i = 0; i < max_i_clamped; i += ints_per_vec) {
            const Vec c = _mm512_loadu_si512(reinterpret_cast<const Vec*>(src_begin + i));
            uint16_t eq_bits = std::bit_cast<uint16_t>(_mm512_cmpeq_epi32_mask(c, v));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }
        Vec c = v;
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - max_i_clamped);
        uint16_t eq_bits = std::bit_cast<uint16_t>(_mm512_cmpeq_epi32_mask(c, v));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
#if UTY_SUPPORTS_128 || defined(UTY_USE_SIMD_128)
    UTY_ALWAYS_INLINE auto find_128(const int32_t* src_begin, const size_t src_size, int32_t value) -> std::ptrdiff_t {
        using Vec = __m128i;

        constexpr static size_t ints_per_vec = 128 / 32;
        const size_t max_i_clamped = src_size - (src_size % ints_per_vec);

        const Vec v = _mm_set1_epi32(value);

        for (size_t i = 0; i < max_i_clamped; i += ints_per_vec) {
            const Vec c = _mm_lddqu_si128(reinterpret_cast<const Vec*>(src_begin + i));
            auto eqi = _mm_cmpeq_epi32(c, v);
            auto eq = _mm_castsi128_ps(eqi);
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_ps(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }

        Vec c = v;
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - max_i_clamped);
        auto eqi = _mm_cmpeq_epi32(c, v);
        auto eq = _mm_castsi128_ps(eqi);
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_ps(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif // SUPPORTS_XXX
    UTY_ALWAYS_INLINE auto find(const int32_t* src_begin, const size_t src_size, int32_t value) noexcept -> std::ptrdiff_t {
#if defined(UTY_USE_SIMD_512) || defined(UTY_USE_SIMD_512)
        return find_512(src_begin, src_size, value);
#elif defined(UTY_USE_SIMD_256) || defined(UTY_USE_SIMD_256)
        // TODO
        return find_128(src_begin, src_size, value);
#elif defined(UTY_USE_SIMD_128) || defined(UTY_USE_SIMD_128)
        return find_128(src_begin, src_size, value);
#elif defined(UTY_NO_SIMD)
        return std::distance(src_begin, std::find(src_begin, src_begin + src_size, value));
#else
        return std::distance(src_begin, std::find(src_begin, src_begin + src_size, value));
#endif
    }

    /*
        find_first_of -> char
    */
#if UTY_SUPPORTS_128 || defined(UTY_USE_SIMD_128)
    UTY_ALWAYS_INLINE auto find_first_of_128(const char* src_begin, const size_t src_size, const char* value_begin, size_t value_size) -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 128 / 8;

        constexpr static size_t max_values = 16;
        assert(value_size < max_values && "Exceeded values capacity, change Utily/Simd.cpp max_values for more.");

        const size_t max_i_clamped = src_size - (src_size % chars_per_vec);

        __m128i vs[max_values];
        for (size_t i = 0; i < value_size; ++i) {
            vs[i] = _mm_set1_epi8(value_begin[i]);
        }

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m128i c = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i));
            __m128i eqs[max_values];
            for (size_t ii = 0; ii < value_size; ++ii) {
                eqs[ii] = _mm_cmpeq_epi8(c, vs[ii]);
            }
            __m128i eq = _mm_setzero_si128();
            for (size_t ii = 0; ii < value_size; ++ii) {
                eq = _mm_or_si128(eq, eqs[ii]);
            }
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }

        __m128i c = vs[0];

        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - max_i_clamped);

        __m128i eqs[max_values];
        for (size_t i = 0; i < value_size; ++i) {
            eqs[i] = _mm_cmpeq_epi8(c, vs[i]);
        }
        __m128i eq = _mm_setzero_si128();
        for (size_t i = 0; i < value_size; ++i) {
            eq = _mm_or_si128(eq, eqs[i]);
        }
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
    UTY_ALWAYS_INLINE auto find_first_of(const char* src_begin, const size_t src_size, const char* value_begin, size_t value_size) noexcept -> std::ptrdiff_t {
#if defined(UTY_NO_SIMD)
        return std::distance(src_begin, std::find_first_of(src_begin, src_begin + src_size, value_begin, value_begin + value_size));
#elif UTY_SUPPORTS_128 || defined(UTY_USE_SIMD_128)
        return find_first_of_128(src_begin, src_size, value_begin, value_size);
#else
        return std::distance(src_begin, std::find_first_of(src_begin, src_begin + src_size, value_begin, value_begin + value_size));
#endif
    }
}

namespace Utily::Simd {
    template <std::contiguous_iterator Iter, typename Value>
    UTY_ALWAYS_INLINE auto find(Iter begin, Iter end, Value value) noexcept -> Iter {
        if constexpr (sizeof(std::iter_value_t<Iter>) == 1 && sizeof(Value) == 1) {
            auto data = reinterpret_cast<const char*>(&(*begin));
            auto& val = *reinterpret_cast<const char*>(&value);
            return begin + Utily::Simd::Details::find(data, static_cast<size_t>(std::distance(begin, end)), val);
        } else if constexpr (sizeof(std::iter_value_t<Iter>) == 4 && sizeof(Value) == 4) {
            auto data = reinterpret_cast<const int32_t*>(&(*begin));
            auto& val = *reinterpret_cast<const int32_t*>(&value);
            return begin + Utily::Simd::Details::find(data, static_cast<size_t>(std::distance(begin, end)), val);
        } else {
            return std::find(begin, end, value);
        }
    }

    template <std::contiguous_iterator SrcIter, std::contiguous_iterator ValIter>
    UTY_ALWAYS_INLINE auto find_first_of(SrcIter src_begin, SrcIter src_end, ValIter value_begin, ValIter value_end) noexcept -> SrcIter {
        if constexpr (sizeof(std::iter_value_t<SrcIter>) == 1 && sizeof(std::iter_value_t<ValIter>) == 1) {
            auto src = reinterpret_cast<const char*>(&(*src_begin));
            auto src_size = static_cast<size_t>(std::distance(src_begin, src_end));

            auto val = reinterpret_cast<const char*>(&(*value_begin));
            auto val_size = static_cast<size_t>(std::distance(value_begin, value_end));

            return src_begin + Utily::Simd::Details::find_first_of(src, src_size, val, val_size);
        } else {
            return std::find_first_of(src_begin, src_end, value_begin, value_end);
        }
    }
}