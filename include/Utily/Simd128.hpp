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

#include <emmintrin.h> // SSE2
#include <pmmintrin.h> // SSE3
#include <smmintrin.h> // SSE4.1
#include <xmmintrin.h> // SSE

#ifndef UTY_ALWAYS_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define UTY_ALWAYS_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define UTY_ALWAYS_INLINE __forceinline
#else
#define UTY_ALWAYS_INLINE inline
#endif
#endif // UTY_ALWAYS_INLINE

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
    UTY_ALWAYS_INLINE auto find_first_of(const char* src_begin, const size_t src_size, const char* val_begin) noexcept -> std::ptrdiff_t {
        constexpr static size_t chars_per_vec = 128 / 8;

        const size_t max_i_clamped = src_size - (src_size % chars_per_vec);

        __m128i vs[ValSize];
        for (size_t i = 0; i < ValSize; ++i) {
            vs[i] = _mm_set1_epi8(val_begin[i]);
        }

        for (size_t i = 0; i < max_i_clamped; i += chars_per_vec) {
            const __m128i c = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_begin + i));
            __m128i eqs[ValSize];
            for (size_t ii = 0; ii < ValSize; ++ii) {
                eqs[ii] = _mm_cmpeq_epi8(c, vs[ii]);
            }
            __m128i eq = _mm_setzero_si128();
            for (size_t ii = 0; ii < ValSize; ++ii) {
                eq = _mm_or_si128(eq, eqs[ii]);
            }
            uint32_t eq_bits = std::bit_cast<uint32_t>(_mm_movemask_epi8(eq));
            if (eq_bits != 0) {
                return static_cast<std::ptrdiff_t>(i) + std::countr_zero(eq_bits);
            }
        }

        __m128i c = vs[0];

        memcpy(reinterpret_cast<void*>(&c), (src_begin + max_i_clamped), src_size - static_cast<size_t>(max_i_clamped));

        __m128i eqs[ValSize];
        for (size_t i = 0; i < ValSize; ++i) {
            eqs[i] = _mm_cmpeq_epi8(c, vs[i]);
        }
        __m128i eq = _mm_setzero_si128();
        for (size_t i = 0; i < ValSize; ++i) {
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

        _mm_prefetch(src_begin, _MM_HINT_T0);
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