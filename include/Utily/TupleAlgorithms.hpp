#pragma once

#include "Utily/Concepts.hpp"

namespace Utily {
    namespace TupleAlgo {
        template <typename Tuple, typename Pred, size_t Index = 0>
            requires(!std::is_reference_v<Tuple>)
        constexpr auto for_each(const Tuple& tuple, Pred& pred) -> void {
            if constexpr (Index == std::tuple_size_v<Tuple>) {
                return;
            } else if constexpr (Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>) {
                pred(std::get<Index>(tuple));
                tuple_for_each<Tuple, Pred, Index + 1>(tuple, pred);
            } else {
                static_assert(Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>, "Predicate must be callable with all tuple element types");
            }
        }

        template <typename Tuple, typename Pred, size_t Index = 0>
            requires(!std::is_reference_v<Tuple>)
        constexpr auto for_each(Tuple&& tuple, Pred& pred) -> void {
            if constexpr (Index == std::tuple_size_v<Tuple>) {
                return;
            } else if constexpr (Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>) {
                pred(std::get<Index>(tuple));
                tuple_for_each<Tuple, Pred, Index + 1>(tuple, pred);
            } else {
                static_assert(Utily::Concepts::IsCallableWith<Pred, std::tuple_element_t<Index, Tuple>>, "Predicate must be callable with all tuple element types");
            }
        }

        template <typename Tuple, typename Iter, size_t Index = 0>
            requires(!std::is_reference_v<Tuple>)
        constexpr auto copy(const Tuple& tuple, Iter& iter) -> void {
            if constexpr (Index == std::tuple_size_v<Tuple>) {
                return;
            } else {
                *iter = std::get<Index>(tuple);
                tuple_for_each<Tuple, Iter, Index + 1>(tuple, ++iter);
            }
        }

    }
}