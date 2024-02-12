#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cassert>
#include <concepts>
#include <cstring>
#include <iostream>
#include <ranges>
#include <type_traits>
#include <vector>

#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <pmmintrin.h> // SSE3
#include <smmintrin.h> // SSE4.1
#include <immintrin.h> // AVX

#ifndef UTY_ALWAYS_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define UTY_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define UTY_ALWAYS_INLINE __forceinline
#else
#define UTY_ALWAYS_INLINE inline
#endif
#endif // UTY_ALWAYS_INLINE

namespace Utily::Simd512::Char {
    UTY_ALWAYS_INLINE auto find(const char* src_begin, const size_t src_size, const char val) noexcept -> std::ptrdiff_t {
        constexpr static int64_t chars_per_vec = 512 / 8;

        const __m512i v = _mm512_set1_epi8(val);

        const std::ptrdiff_t max_i_clamped = static_cast<std::ptrdiff_t>(src_size - (src_size % chars_per_vec));

        for (std::ptrdiff_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m512i c = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src_begin + i));
            const uint64_t eq = _mm512_cmpeq_epi8_mask(c, v);

            if (eq != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq);
            }
        }
        __m512i c = v;
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));
        uint64_t eq = _mm512_cmpeq_epi8_mask(c, v);
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq);
    }

    template <size_t ValSize>
    UTY_ALWAYS_INLINE auto search(const char* src_begin [[maybe_unused]], const size_t src_size [[maybe_unused]], const char* val_begin [[maybe_unused]]) noexcept -> std::ptrdiff_t {
        // static_assert(false, "Not implemented");
        assert(false && "Not implemeted");
        return std::ptrdiff_t { 0 };
    }
    template <>
    UTY_ALWAYS_INLINE auto search<4>(const char* src_begin, const size_t src_size, const char* val_begin) noexcept -> std::ptrdiff_t {
        constexpr static uint64_t chars_per_vec = 512 / 8;

        const int64_t ssrc_size = static_cast<int64_t>(src_size);
        const int64_t lhs = ssrc_size - (ssrc_size % static_cast<int64_t>(chars_per_vec)) - 3;
        const int64_t num_vectorised_loops = std::max({ lhs, static_cast<int64_t>(0) });
        const uint64_t num_vectorised_loops_u = static_cast<uint64_t>(num_vectorised_loops);

        int32_t val_i32 = *reinterpret_cast<const int32_t*>(val_begin);

        const __m512i v = _mm512_set1_epi32(val_i32);

        for (uint64_t i = 3; i < num_vectorised_loops_u; i += chars_per_vec) {
            _mm_prefetch(src_begin + i - 3, _MM_HINT_T0);
            const __m512i d[4] = {
                _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src_begin + i - 3)),
                _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src_begin + i - 2)),
                _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src_begin + i - 1)),
                _mm512_loadu_si512(reinterpret_cast<const __m512i*>(src_begin + i - 0))
            };

            const uint16_t cmpeq[4] = {
                _mm512_cmpeq_epi32_mask(d[0], v),
                _mm512_cmpeq_epi32_mask(d[1], v),
                _mm512_cmpeq_epi32_mask(d[2], v),
                _mm512_cmpeq_epi32_mask(d[3], v)
            };

            const uint16_t any_eq = cmpeq[0] | cmpeq[1] | cmpeq[2] | cmpeq[3];

            if (any_eq) {

                const int32_t cz[4] = {
                    (std::countr_zero(cmpeq[0]) * 4) - 3,
                    (std::countr_zero(cmpeq[1]) * 4) - 2,
                    (std::countr_zero(cmpeq[2]) * 4) - 1,
                    (std::countr_zero(cmpeq[3]) * 4) - 0
                };
                std::ptrdiff_t res = static_cast<std::ptrdiff_t>(i) + std::min({ cz[0], cz[1], cz[2], cz[3] });
                return std::min(res, static_cast<std::ptrdiff_t>(ssrc_size));
            }
        }

        {
            __m512i d[4] = {
                _mm512_setzero_epi32(),
                _mm512_setzero_epi32(),
                _mm512_setzero_epi32(),
                _mm512_setzero_epi32()
            };

            memcpy(&d[0], (src_begin + num_vectorised_loops + 0), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 0 }));
            memcpy(&d[1], (src_begin + num_vectorised_loops + 1), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 1 }));
            memcpy(&d[2], (src_begin + num_vectorised_loops + 2), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 2 }));
            memcpy(&d[3], (src_begin + num_vectorised_loops + 3), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 3 }));

            const unsigned short cmpeq[4] = {
                _mm512_cmpeq_epi32_mask(d[0], v),
                _mm512_cmpeq_epi32_mask(d[1], v),
                _mm512_cmpeq_epi32_mask(d[2], v),
                _mm512_cmpeq_epi32_mask(d[3], v)
            };

            const bool any_eq = cmpeq[0] || cmpeq[1] || cmpeq[2] || cmpeq[3];

            if (any_eq) {
                const int32_t cz[4] = {
                    (std::countr_zero(cmpeq[0]) * 4) - 3,
                    (std::countr_zero(cmpeq[1]) * 4) - 2,
                    (std::countr_zero(cmpeq[2]) * 4) - 1,
                    (std::countr_zero(cmpeq[3]) * 4) - 0
                };
                std::ptrdiff_t res = static_cast<std::ptrdiff_t>(num_vectorised_loops) + std::min({ cz[0], cz[1], cz[2], cz[3] });
                return std::min(res, static_cast<std::ptrdiff_t>(ssrc_size));
            }
            return static_cast<std::ptrdiff_t>(src_size);
        }
    }

    UTY_ALWAYS_INLINE auto search(const char* src_begin, const size_t src_size, const char* val_begin, const size_t val_size) noexcept -> std::ptrdiff_t {
        assert(src_begin != nullptr);
        if (src_size < val_size) {
            return static_cast<std::ptrdiff_t>(src_size);
        } else if (val_size == 4) {
            return Simd128::Char::search<4>(src_begin, src_size, val_begin);
        }
        assert(false && "not implemented");
        return std::distance(src_begin, std::search(src_begin, src_begin + src_size, val_begin, val_begin + val_size));
    }
}