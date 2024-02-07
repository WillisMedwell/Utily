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

#if defined(__GNUC__) || defined(__clang__)
#define UTY_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define UTY_ALWAYS_INLINE __forceinline
#else
#define UTY_ALWAYS_INLINE inline
#endif

#if defined(_MSC_VER) || (defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSE4_1__))

#include <emmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

namespace Utily::Simd128::Char {
    UTY_ALWAYS_INLINE auto find(const char* src_begin, const size_t src_size, const char val) noexcept -> std::ptrdiff_t {
        constexpr static int64_t chars_per_vec = 128 / 8;

        const __m128i v = _mm_set1_epi8(val);

        const std::ptrdiff_t max_i_clamped = static_cast<std::ptrdiff_t>(src_size - (src_size % chars_per_vec));
        const std::ptrdiff_t max_4_i_count = static_cast<std::ptrdiff_t>(src_size - (src_size % (chars_per_vec * 4)));

        // This loop is here to make the most of throughput
        for (std::ptrdiff_t i = 0; i < max_4_i_count; i += (chars_per_vec * std::ptrdiff_t { 4 })) {
            // 6 + (0.33 * 4) ~= 7.33 for 4 loads instead of 6.33 for 1 load.
            const __m128i c0 = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i + (chars_per_vec * 0)));
            const __m128i c1 = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i + (chars_per_vec * 1)));
            const __m128i c2 = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i + (chars_per_vec * 2)));
            const __m128i c3 = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i + (chars_per_vec * 3)));

            // 1 + (0.5 * 4) ~= 3 for 4 compares instead of 1.5 for 1 cmp.
            const __m128i eq0 = _mm_cmpeq_epi8(c0, v);
            const __m128i eq1 = _mm_cmpeq_epi8(c1, v);
            const __m128i eq2 = _mm_cmpeq_epi8(c2, v);
            const __m128i eq3 = _mm_cmpeq_epi8(c3, v);

            // 1 + (0.33 * 3) +
            int and_eq_bits = _mm_movemask_epi8(
                _mm_or_si128(
                    _mm_or_si128(eq0, eq1),
                    _mm_or_si128(eq2, eq3)));

            if (and_eq_bits) {
                int s_eq_b0 = _mm_movemask_epi8(eq0);
                int s_eq_b1 = _mm_movemask_epi8(eq1);
                int s_eq_b2 = _mm_movemask_epi8(eq2);
                int s_eq_b3 = _mm_movemask_epi8(eq3);
                std::ptrdiff_t eq_bits[4] = {
                    static_cast<std::ptrdiff_t>(i) + std::countr_zero(std::bit_cast<uint32_t>(s_eq_b0)) + (static_cast<int32_t>(chars_per_vec * 0)),
                    static_cast<std::ptrdiff_t>(i) + std::countr_zero(std::bit_cast<uint32_t>(s_eq_b1)) + (static_cast<int32_t>(chars_per_vec * 1)),
                    static_cast<std::ptrdiff_t>(i) + std::countr_zero(std::bit_cast<uint32_t>(s_eq_b2)) + (static_cast<int32_t>(chars_per_vec * 2)),
                    static_cast<std::ptrdiff_t>(i) + std::countr_zero(std::bit_cast<uint32_t>(s_eq_b3)) + (static_cast<int32_t>(chars_per_vec * 3))
                };

                if (s_eq_b0 != 0) {
                    return eq_bits[0];
                } else if (s_eq_b1 != 0) {
                    return eq_bits[1];
                } else if (s_eq_b2 != 0) {
                    return eq_bits[2];
                } else {
                    return eq_bits[3];
                }
            }
        }

        for (std::ptrdiff_t i = max_4_i_count; i < max_i_clamped; i += chars_per_vec) {
            const __m128i c = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i));
            const __m128i eq = _mm_cmpeq_epi8(c, v);
            int signed_eq_bits = _mm_movemask_epi8(eq);

            if (signed_eq_bits != 0) {
                uint32_t eq_bits = std::bit_cast<uint32_t>(signed_eq_bits);
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }
        __m128i c = v;
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(c, v)));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
    UTY_ALWAYS_INLINE auto find_first_of(const char* src_begin, const size_t src_size, const char* val_begin, const size_t val_size) noexcept -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 128 / 8;
        constexpr static size_t max_values = 16;

        assert(val_size < max_values && "Exceeded values capacity, change Utily/Simd.cpp max_values for more.");

        const size_t max_i_clamped = src_size - (src_size % chars_per_vec);

        __m128i vs[max_values];
        for (size_t i = 0; i < val_size; ++i) {
            vs[i] = _mm_set1_epi8(val_begin[i]);
        }

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m128i c = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i));
            __m128i eqs[max_values];
            for (size_t ii = 0; ii < val_size; ++ii) {
                eqs[ii] = _mm_cmpeq_epi8(c, vs[ii]);
            }
            __m128i eq = _mm_setzero_si128();
            for (size_t ii = 0; ii < val_size; ++ii) {
                eq = _mm_or_si128(eq, eqs[ii]);
            }
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }

        __m128i c = vs[0];

        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));

        __m128i eqs[max_values];
        for (size_t i = 0; i < val_size; ++i) {
            eqs[i] = _mm_cmpeq_epi8(c, vs[i]);
        }
        __m128i eq = _mm_setzero_si128();
        for (size_t i = 0; i < val_size; ++i) {
            eq = _mm_or_si128(eq, eqs[i]);
        }
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }

    template <size_t ValSize>
    UTY_ALWAYS_INLINE auto search(const char* src_begin [[maybe_unused]], const size_t src_size [[maybe_unused]], const char* val_begin [[maybe_unused]]) noexcept -> std::ptrdiff_t {
        // static_assert(false, "Not implemented");
        assert(false && "Not implemeted");
        return std::ptrdiff_t { 0 };
    }
    template <>
    UTY_ALWAYS_INLINE auto search<4>(const char* src_begin, const size_t src_size, const char* val_begin) noexcept -> std::ptrdiff_t {
        constexpr static uint64_t chars_per_vec = 128 / 8;

        const int64_t ssrc_size = static_cast<int64_t>(src_size);
        const int64_t lhs = ssrc_size - (ssrc_size % static_cast<int64_t>(chars_per_vec)) - 3;
        const int64_t num_vectorised_loops = std::max({ lhs, static_cast<int64_t>(0) });
        const uint64_t num_vectorised_loops_u = static_cast<uint64_t>(num_vectorised_loops);

        int32_t val_i32 = *reinterpret_cast<const int32_t*>(val_begin);

        const __m128i v = _mm_set1_epi32(val_i32);

        for (uint64_t i = 3; i < num_vectorised_loops_u; i += chars_per_vec) {

            const __m128i d[4] = {
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 3)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 2)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 1)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 0))
            };

            const __m128i cmpeq[4] = {
                _mm_cmpeq_epi32(d[0], v),
                _mm_cmpeq_epi32(d[1], v),
                _mm_cmpeq_epi32(d[2], v),
                _mm_cmpeq_epi32(d[3], v)
            };

            const int32_t any_eq = _mm_movemask_ps(_mm_castsi128_ps(
                _mm_or_si128(
                    _mm_or_si128(cmpeq[0], cmpeq[1]),
                    _mm_or_si128(cmpeq[2], cmpeq[3]))));

            if (any_eq) {
                const int32_t mm[4] = {
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[0])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[1])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[2])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[3]))
                };

                const int32_t cz[4] = {
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[0])) * 4) - 3,
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[1])) * 4) - 2,
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[2])) * 4) - 1,
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[3])) * 4) - 0
                };
                std::ptrdiff_t res = static_cast<std::ptrdiff_t>(i) + std::min({ cz[0], cz[1], cz[2], cz[3] });
                return std::min(res, static_cast<std::ptrdiff_t>(ssrc_size));
            }
        }

        {
            __m128i d[4] = {
                _mm_setzero_si128(),
                _mm_setzero_si128(),
                _mm_setzero_si128(),
                _mm_setzero_si128()
            };

            memcpy(&d[0], (src_begin + num_vectorised_loops + 0), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 0 }));
            memcpy(&d[1], (src_begin + num_vectorised_loops + 1), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 1 }));
            memcpy(&d[2], (src_begin + num_vectorised_loops + 2), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 2 }));
            memcpy(&d[3], (src_begin + num_vectorised_loops + 3), static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 3 }));

            const __m128i cmpeq[4] = {
                _mm_cmpeq_epi32(d[0], v),
                _mm_cmpeq_epi32(d[1], v),
                _mm_cmpeq_epi32(d[2], v),
                _mm_cmpeq_epi32(d[3], v)
            };

            const int32_t any_eq = _mm_movemask_ps(_mm_castsi128_ps(
                _mm_or_si128(
                    _mm_or_si128(cmpeq[0], cmpeq[1]),
                    _mm_or_si128(cmpeq[2], cmpeq[3]))));

            if (any_eq) {
                const int32_t mm[4] = {
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[0])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[1])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[2])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[3]))
                };
                const int32_t cz[4] = {
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[0])) * 4) + 0,
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[1])) * 4) + 1,
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[2])) * 4) + 2,
                    (std::countr_zero(std::bit_cast<uint32_t>(mm[3])) * 4) + 3
                };
                const auto min = std::min({ cz[0], cz[1], cz[2], cz[3] });
                const auto res = static_cast<std::ptrdiff_t>(num_vectorised_loops) + min;
                return std::min(res, static_cast<std::ptrdiff_t>(src_size));
            }
            return static_cast<std::ptrdiff_t>(src_size);
        }
    }
    template <>
    UTY_ALWAYS_INLINE auto search<8>(const char* src_begin, const size_t src_size, const char* val_begin) noexcept -> std::ptrdiff_t {
        assert(src_begin != nullptr);

        constexpr static uint64_t chars_per_vec = 128 / 8;
        const int64_t ssrc_size = static_cast<int64_t>(src_size);
        const int64_t lhs = ssrc_size - (ssrc_size % static_cast<int64_t>(chars_per_vec)) - 7;
        const int64_t num_vectorised_loops = std::max({ lhs, static_cast<int64_t>(0) });
        const uint64_t num_vectorised_loops_u = static_cast<uint64_t>(num_vectorised_loops);

        int64_t val_i64 = *reinterpret_cast<const int64_t*>(val_begin);
        const __m128i v = _mm_set1_epi64x(val_i64);

        for (uint64_t i = 7; i < num_vectorised_loops_u; i += chars_per_vec) {
            const __m128i d[8] = {
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 7)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 6)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 5)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 4)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 3)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 2)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 1)),
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i - 0))
            };

            const __m128i cmpeq[8] = {
                _mm_cmpeq_epi64(d[0], v),
                _mm_cmpeq_epi64(d[1], v),
                _mm_cmpeq_epi64(d[2], v),
                _mm_cmpeq_epi64(d[3], v),
                _mm_cmpeq_epi64(d[4], v),
                _mm_cmpeq_epi64(d[5], v),
                _mm_cmpeq_epi64(d[6], v),
                _mm_cmpeq_epi64(d[7], v)
            };

            const int32_t any_eq =
                _mm_movemask_ps(_mm_castsi128_ps(
                    _mm_or_si128(
                        _mm_or_si128(
                            _mm_or_si128(cmpeq[0], cmpeq[1]),
                            _mm_or_si128(cmpeq[2], cmpeq[3])),
                        _mm_or_si128(
                            _mm_or_si128(cmpeq[4], cmpeq[5]),
                            _mm_or_si128(cmpeq[6], cmpeq[7])))));

            if (any_eq) {
                const int32_t mm[8] = {
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[0])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[1])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[2])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[3])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[4])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[5])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[6])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[7]))
                };

                const int64_t cz[8] = {
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[0]))) * 4 - 7,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[1]))) * 4 - 6,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[2]))) * 4 - 5,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[3]))) * 4 - 4,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[4]))) * 4 - 3,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[5]))) * 4 - 2,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[6]))) * 4 - 1,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[7]))) * 4 - 0
                };

                std::ptrdiff_t res = static_cast<int64_t>(i) + std::min({ cz[0], cz[1], cz[2], cz[3], cz[4], cz[5], cz[6], cz[7] });
                assert(std::distance(src_begin, std::search(src_begin, src_begin + src_size, val_begin, val_begin + 8)) == res);
                return res;
            }
        }

        {
            __m128i d[8] = {};

            memcpy(&d[0], src_begin + num_vectorised_loops + 0, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 0 }));
            memcpy(&d[1], src_begin + num_vectorised_loops + 1, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 1 }));
            memcpy(&d[2], src_begin + num_vectorised_loops + 2, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 2 }));
            memcpy(&d[3], src_begin + num_vectorised_loops + 3, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 3 }));
            memcpy(&d[4], src_begin + num_vectorised_loops + 4, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 4 }));
            memcpy(&d[5], src_begin + num_vectorised_loops + 5, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 5 }));
            memcpy(&d[6], src_begin + num_vectorised_loops + 6, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 6 }));
            memcpy(&d[7], src_begin + num_vectorised_loops + 7, static_cast<size_t>(ssrc_size - num_vectorised_loops - int64_t { 7 }));

            const __m128i cmpeq[8] = {
                _mm_cmpeq_epi64(d[0], v),
                _mm_cmpeq_epi64(d[1], v),
                _mm_cmpeq_epi64(d[2], v),
                _mm_cmpeq_epi64(d[3], v),
                _mm_cmpeq_epi64(d[4], v),
                _mm_cmpeq_epi64(d[5], v),
                _mm_cmpeq_epi64(d[6], v),
                _mm_cmpeq_epi64(d[7], v),
            };

            const int any_eq =
                _mm_movemask_ps(_mm_castsi128_ps(
                    _mm_or_si128(
                        _mm_or_si128(
                            _mm_or_si128(cmpeq[0], cmpeq[1]),
                            _mm_or_si128(cmpeq[2], cmpeq[3])),
                        _mm_or_si128(
                            _mm_or_si128(cmpeq[4], cmpeq[5]),
                            _mm_or_si128(cmpeq[6], cmpeq[7])))));

            if (any_eq) {
                const int32_t mm[8] = {
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[0])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[1])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[2])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[3])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[4])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[5])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[6])),
                    _mm_movemask_ps(_mm_castsi128_ps(cmpeq[7]))
                };

                const int64_t cz[8] = {
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[0]))) * 4 + 0,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[1]))) * 4 + 1,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[2]))) * 4 + 2,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[3]))) * 4 + 3,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[4]))) * 4 + 4,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[5]))) * 4 + 5,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[6]))) * 4 + 6,
                    static_cast<int64_t>(std::countr_zero(std::bit_cast<uint32_t>(mm[7]))) * 4 + 7
                };
                auto min = std::min({ cz[0], cz[1], cz[2], cz[3], cz[4], cz[5], cz[6], cz[7], ssrc_size - num_vectorised_loops });
                auto res = static_cast<std::ptrdiff_t>(num_vectorised_loops) + min;
                assert(std::distance(src_begin, std::search(src_begin, src_begin + src_size, val_begin, val_begin + 8)) == res);
                return res;
            }
            return static_cast<std::ptrdiff_t>(src_size);
        }
    }

    UTY_ALWAYS_INLINE auto search(const char* src_begin, const size_t src_size, const char* val_begin, const size_t val_size) noexcept -> std::ptrdiff_t {
        assert(src_begin != nullptr);
        if (src_size < val_size) {
            return static_cast<std::ptrdiff_t>(src_size);
        } else if (val_size == 1) {
            return Simd128::Char::find(src_begin, src_size, *val_begin);
        } else if (val_size == 4) {
            return Simd128::Char::search<4>(src_begin, src_size, val_begin);
        } else if (val_size == 8) {
            return Simd128::Char::search<8>(src_begin, src_size, val_begin);
        }
        assert(false && "not implemented");
        return std::distance(src_begin, std::search(src_begin, src_begin + src_size, val_begin, val_begin + val_size));
    }
}

#else

#warning "Utily::Simd128 falling back to stl implementation; As SSE/SSE2/SSE3 has not been enabled -> use -mtune=native flag."

namespace Utily::Simd128 {
    namespace Char {
        UTY_ALWAYS_INLINE auto find(const char* src_begin, const size_t src_size, const char val) noexcept -> std::ptrdiff_t {
            return std::distance(src_begin, std::find(src_begin, (src_begin + src_size), val));
        }
        UTY_ALWAYS_INLINE auto find_first_of(const char* src_begin, const size_t src_size, const char* val_begin, const size_t val_size) noexcept -> std::ptrdiff_t {
            return std::distance(
                src_begin,
                std::find_first_of(src_begin, (src_begin + src_size), val_begin, (val_begin + val_size)));
        }
        UTY_ALWAYS_INLINE auto search(const char* src_begin, const size_t src_size, const char* val_begin, const size_t val_size) noexcept -> std::ptrdiff_t {
            return std::distance(
                src_begin,
                std::search(src_begin, (src_begin + src_size), val_begin, (val_begin + val_size)));
        }
    }
}

#endif // defined(_MSC_VER) || (defined(__SSE__) && defined(__SSE2__) && defined(__SSE3__) && defined(__SSE4_1__))

#if defined(__AVX512BW__)

#include <immintrin.h>

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

#else

namespace Utily::Simd512::Char {
    UTY_ALWAYS_INLINE static auto find(const char* src_begin, const size_t src_size, const char val) noexcept -> std::ptrdiff_t {
        return Utily::Simd128::Char::find(src_begin, src_size, val);
    }

    UTY_ALWAYS_INLINE auto search(const char* src_begin, const size_t src_size, const char* val_begin, const size_t val_size) noexcept -> std::ptrdiff_t {
        return Utily::Simd128::Char::search(src_begin, src_size, val_begin, val_size);
    }

}

#endif // defined(_MSV_VER) || defined(__AVX2__)

/*
namespace Utily::Simd::Details {

#ifdef UTY_USE_SIMD_512
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
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));
        uint64_t eq_bits = std::bit_cast<uint64_t>(_mm512_cmpeq_epi8_mask(c, v));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
#ifdef UTY_USE_SIMD_256
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
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));
        const __m256i eq = _mm256_cmpeq_epi8(c, v);
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm256_movemask_epi8(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
#ifdef UTY_USE_SIMD_128
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
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(c, v)));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
    UTY_ALWAYS_INLINE auto find(const char* src_begin, const size_t src_size, char value) noexcept -> std::ptrdiff_t {
#ifdef UTY_USE_SIMD_512
        return find_512(src_begin, src_size, value);
#elif defined(UTY_USE_SIMD_512)
        return find_256(src_begin, src_size, value);
#elif defined(UTY_USE_SIMD_128)
        return find_128(src_begin, src_size, value);
#else
        return std::distance(src_begin, std::find(src_begin, src_begin + src_size, value));
#endif
    }


#if defined(UTY_USE_SIMD_512)
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
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));
        uint16_t eq_bits = std::bit_cast<uint16_t>(_mm512_cmpeq_epi32_mask(c, v));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif
#if defined(UTY_USE_SIMD_128)
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
        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));
        auto eqi = _mm_cmpeq_epi32(c, v);
        auto eq = _mm_castsi128_ps(eqi);
        uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_ps(eq));
        return static_cast<std::ptrdiff_t>(max_i_clamped) + std::countr_zero(eq_bits);
    }
#endif // SUPPORTS_XXX
    UTY_ALWAYS_INLINE auto find(const int32_t* src_begin, const size_t src_size, int32_t value) noexcept -> std::ptrdiff_t {
#if defined(UTY_USE_SIMD_512)
        return find_512(src_begin, src_size, value);
#elif defined(UTY_USE_SIMD_128)
        return find_128(src_begin, src_size, value);
#else
        return std::distance(src_begin, std::find(src_begin, src_begin + src_size, value));
#endif
    }

#if defined(UTY_USE_SIMD_128)
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

        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));

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
#if defined(UTY_USE_SIMD_128)
        return find_first_of_128(src_begin, src_size, value_begin, value_size);
#else
        return std::distance(src_begin, std::find_first_of(src_begin, src_begin + src_size, value_begin, value_begin + value_size));
#endif
    }
#if defined(UTY_USE_SIMD_128)
    UTY_ALWAYS_INLINE auto find_subrange_128_4(const char* src_begin, const size_t src_size, const char* val_begin) -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 128 / 8;
        const size_t num_vectorised_loops = src_size - (src_size % chars_per_vec);

        int32_t val_i32 = *reinterpret_cast<const int32_t*>(val_begin);

        const __m128i v = _mm_set1_epi32(val_i32);

        for (size_t i = 0; i < num_vectorised_loops; i += chars_per_vec) {
            const __m128i a =
                _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i));
            const __m128i b = _mm_lddqu_si128(
                reinterpret_cast<const __m128i*>(src_begin + i + 1));
            const __m128i c = _mm_lddqu_si128(
                reinterpret_cast<const __m128i*>(src_begin + i + 2));
            const __m128i d = _mm_lddqu_si128(
                reinterpret_cast<const __m128i*>(src_begin + i + 3));

            const auto av = _mm_cmpeq_epi32(a, v);
            const auto bv = _mm_cmpeq_epi32(b, v);
            const auto cv = _mm_cmpeq_epi32(c, v);
            const auto dv = _mm_cmpeq_epi32(d, v);

            const auto abcdv =
                _mm_and_si128(_mm_and_si128(av, bv), _mm_and_si128(cv, dv));
            const uint32_t mask =
                std::bit_cast<uint32_t>(_mm_movemask_ps(_mm_castsi128_ps(abcdv)));
            if (mask != 0) {
                const int32_t mav = std::countr_zero(
                    std::bit_cast<uint32_t>(_mm_movemask_ps(_mm_castsi128_ps(av))));
                const int32_t mbv = std::countr_zero(std::bit_cast<uint32_t>(
                                        _mm_movemask_ps(_mm_castsi128_ps(av))))
                    + 1;
                const int32_t mcv = std::countr_zero(std::bit_cast<uint32_t>(
                                        _mm_movemask_ps(_mm_castsi128_ps(av))))
                    + 2;
                const int32_t mdv = std::countr_zero(std::bit_cast<uint32_t>(
                                        _mm_movemask_ps(_mm_castsi128_ps(av))))
                    + 3;
                return static_cast<std::ptrdiff_t>(i) + std::min({ mav, mbv, mcv, mdv });
            }
        }

        __m128i a = v;
        __m128i b = v;
        __m128i c = v;
        __m128i d = v;

        memcpy(reinterpret_cast<void*>(&a), (src_begin + num_vectorised_loops), src_size - num_vectorised_loops);
        memcpy(reinterpret_cast<void*>(&b), (src_begin + num_vectorised_loops + 1), src_size - num_vectorised_loops - 1);
        memcpy(reinterpret_cast<void*>(&c), (src_begin + num_vectorised_loops + 2), src_size - num_vectorised_loops - 2);
        memcpy(reinterpret_cast<void*>(&d), (src_begin + num_vectorised_loops + 3), src_size - num_vectorised_loops - 3);

        const auto av = _mm_cmpeq_epi32(a, v);
        const auto bv = _mm_cmpeq_epi32(b, v);
        const auto cv = _mm_cmpeq_epi32(c, v);
        const auto dv = _mm_cmpeq_epi32(d, v);

        const auto abcdv =
            _mm_or_si128(_mm_or_si128(av, bv), _mm_or_si128(cv, dv));
        const uint32_t mask =
            std::bit_cast<uint32_t>(_mm_movemask_ps(_mm_castsi128_ps(abcdv)));

        if (mask != 0) {
            const int32_t mav = std::countr_zero(
                std::bit_cast<uint32_t>(_mm_movemask_ps(_mm_castsi128_ps(av))));
            const int32_t mbv = std::countr_zero(std::bit_cast<uint32_t>(
                                    _mm_movemask_ps(_mm_castsi128_ps(av))))
                + 1;
            const int32_t mcv = std::countr_zero(std::bit_cast<uint32_t>(
                                    _mm_movemask_ps(_mm_castsi128_ps(av))))
                + 2;
            const int32_t mdv = std::countr_zero(std::bit_cast<uint32_t>(
                                    _mm_movemask_ps(_mm_castsi128_ps(av))))
                + 3;
            return static_cast<std::ptrdiff_t>(num_vectorised_loops) + std::min({ mav, mbv, mcv, mdv });
        } else {
            return static_cast<std::ptrdiff_t>(src_size);
        }
    }
#endif

    UTY_ALWAYS_INLINE auto find_subrange(const char* src_begin, const size_t src_size, const char* val_begin, const size_t val_size) -> std::ptrdiff_t {
        if (val_size == 4) {
#if defined(UTY_USE_SIMD_128)
            return find_subrange_128_4(src_begin, src_size, val_begin);
#endif
        }
        return std::distance(src_begin, std::search(src_begin, src_begin + src_size, val_begin, val_begin + val_size));
    }

}

namespace Utily::Simd {
    template <std::contiguous_iterator Iter, typename Value>
    UTY_ALWAYS_INLINE auto find(Iter begin, Iter end, Value value) noexcept -> Iter {
        if constexpr (sizeof(std::iter_value_t<Iter>) == 1 && sizeof(Value) == 1) {
            if (begin == end) {
                return begin;
            }
            auto data = reinterpret_cast<const char*>(&(*begin));
            auto& val = *reinterpret_cast<const char*>(&value);
            return begin + Utily::Simd::Details::find(data, static_cast<size_t>(std::distance(begin, end)), val);
        } else if constexpr (sizeof(std::iter_value_t<Iter>) == 4 && sizeof(Value) == 4) {
            if (begin == end) {
                return begin;
            }
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

    template <std::contiguous_iterator SrcIter, std::contiguous_iterator ValIter>
    UTY_ALWAYS_INLINE auto find_subrange(SrcIter src_begin, SrcIter src_end, ValIter value_begin, ValIter value_end) noexcept -> SrcIter {
        if constexpr (sizeof(std::iter_value_t<SrcIter>) == 1 && sizeof(std::iter_value_t<ValIter>) == 1) {
            auto src = reinterpret_cast<const char*>(&(*src_begin));
            auto src_size = static_cast<size_t>(std::distance(src_begin, src_end));

            auto val = reinterpret_cast<const char*>(&(*value_begin));
            auto val_size = static_cast<size_t>(std::distance(value_begin, value_end));

            return src_begin + Utily::Simd::Details::find_subrange(src, src_size, val, val_size);
        } else {
            return std::search(src_begin, src_end, value_begin, value_end);
        }
    }
}
*/
