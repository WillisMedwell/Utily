#pragma once

#include "Utily/Concepts.hpp"

namespace Utily {
    namespace TupleAlgo {
        template <typename Tuple, typename Pred, size_t Index = 0>
            requires(!std::is_reference_v<Tuple>)
        constexpr auto for_each(Tuple& tuple, Pred pred) -> void {
            if constexpr (Index == std::tuple_size_v<Tuple>) {
                return;
            } else if constexpr (Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>) {
                pred(std::get<Index>(tuple));
                Utily::TupleAlgo::for_each<Tuple, Pred, Index + 1>(tuple, pred);
            } else {
                static_assert(Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>, "Predicate must be callable with all tuple element types");
            }
        }

        template <typename Tuple, typename Pred, size_t Index = 0>
            requires(!std::is_reference_v<Tuple>)
        constexpr auto for_each(Tuple&& tuple, Pred pred) -> void {
            if constexpr (Index == std::tuple_size_v<Tuple>) {
                return;
            } else if constexpr (Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>) {
                pred(std::get<Index>(tuple));
                Utily::TupleAlgo::for_each<Tuple, Pred, Index + 1>(tuple, pred);
            } else {
                static_assert(Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>, "Predicate must be callable with all tuple element types");
            }
        }

        template <typename Tuple, typename Iter, size_t Index = 0>
            requires(!std::is_reference_v<Tuple>)
        constexpr auto copy(const Tuple& tuple, Iter iter) -> void {
            if constexpr (Index == std::tuple_size_v<Tuple>) {
                return;
            } else {
                using TupleElement = std::tuple_element_t<Index, Tuple>;
                if constexpr (std::is_rvalue_reference_v<TupleElement>) {
                    *iter = std::forward<std::remove_reference_t<TupleElement>>(std::get<Index>(tuple));
                } else {
                    *iter = std::get<Index>(tuple);
                }
                copy<Tuple, Iter, Index + 1>(tuple, ++iter);
            }
        }

        template <typename Tuple, typename Iter, size_t Index = 0>
            requires(!std::is_reference_v<Tuple>)
        constexpr auto copy(Tuple&& tuple, Iter iter) -> void {
            if constexpr (Index == std::tuple_size_v<Tuple>) {
                return;
            } else {
                using TupleElement = std::tuple_element_t<Index, Tuple>;
                if constexpr (std::is_rvalue_reference_v<TupleElement>) {
                    *iter = std::forward<std::remove_reference_t<TupleElement>>(std::get<Index>(tuple));
                } else {
                    *iter = std::get<Index>(tuple);
                }
                copy<Tuple, Iter, Index + 1>(tuple, ++iter);
            }
        }

    }
}