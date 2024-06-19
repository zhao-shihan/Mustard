// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/TupleModel.h++"

#include "muc/concepts"
#include "muc/tuple"

#include "fmt/format.h"

#include <algorithm>
#include <concepts>
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace Mustard::Data::internal {

template<typename I>
concept IteratorOfTupleLike =
    requires {
        requires std::input_or_output_iterator<I>;
        requires TupleLike<std::decay_t<std::iter_reference_t<I>>>;
    };

template<typename I, typename T>
concept OutputIteratorOfTupleLike =
    requires {
        requires IteratorOfTupleLike<I>;
        requires std::output_iterator<I, T>;
    };

template<typename I>
concept InputIteratorOfTupleLike =
    requires {
        requires IteratorOfTupleLike<I>;
        requires std::input_iterator<I>;
    };

template<typename I>
concept ForwardIteratorOfTupleLike =
    requires {
        requires IteratorOfTupleLike<I>;
        requires std::forward_iterator<I>;
    };

template<typename R>
concept RangeOfTupleLike =
    requires {
        requires std::ranges::range<R>;
        requires IteratorOfTupleLike<std::ranges::iterator_t<R>>;
    };

template<typename R>
concept InputRangeOfTupleLike =
    requires {
        requires RangeOfTupleLike<R>;
        requires std::ranges::input_range<R>;
    };

template<typename R>
concept ForwardRangeOfTupleLike =
    requires {
        requires RangeOfTupleLike<R>;
        requires std::ranges::forward_range<R>;
    };

namespace internal {

template<typename IOrR, muc::ceta_string AName>
auto ValueTypeHelper() -> auto {
    if constexpr (IteratorOfTupleLike<IOrR>) {
        return typename std::decay_t<std::iter_reference_t<IOrR>>::Model::template ValueOf<AName>{};
    } else {
        return typename std::decay_t<std::ranges::range_reference_t<IOrR>>::Model::template ValueOf<AName>{};
    }
}

} // namespace internal

template<typename IOrR, muc::ceta_string AName>
    requires IteratorOfTupleLike<IOrR> or RangeOfTupleLike<IOrR>
using ValueType = decltype(internal::ValueTypeHelper<IOrR, AName>());

template<typename IOrR, muc::ceta_string AName>
    requires IteratorOfTupleLike<IOrR> or RangeOfTupleLike<IOrR>
using UnderlyingType = typename ValueType<IOrR, AName>::Type;

template<typename IOrR, muc::ceta_string... ANames>
    requires IteratorOfTupleLike<IOrR> or RangeOfTupleLike<IOrR>
using TupleType = Tuple<ValueType<IOrR, ANames>...>;

template<typename F, typename IOrR, muc::ceta_string... ANames>
concept InvocableByName =
    requires {
        requires IteratorOfTupleLike<IOrR> or RangeOfTupleLike<IOrR>;
        requires sizeof...(ANames) >= 1;
        requires((sizeof...(ANames) == 1 and std::invocable<F, UnderlyingType<IOrR, ANames>...>) or
                 (sizeof...(ANames) >= 2 and std::invocable<F, ValueType<IOrR, ANames>...>));
    };

template<typename F, typename S, muc::ceta_string... ANames>
    requires InvocableByName<F, S, ANames...>
using InvokeByNameResult = std::invoke_result_t<F, std::conditional_t<sizeof...(ANames) == 1,
                                                                      UnderlyingType<S, ANames>,
                                                                      ValueType<S, ANames>>...>;

template<typename I>
concept SheetIterator =
    requires(I i) {
        requires ForwardIteratorOfTupleLike<I>;
        { i.TheSheet() };
        { i.TheSheet().begin() } -> std::common_reference_with<I>;
    };

template<typename S, typename I>
concept SheetSentinelFor =
    requires(S s, I i) {
        requires SheetIterator<S>;
        requires SheetIterator<I>;
        requires std::sentinel_for<S, I>;
        { s.TheSheet().begin() } -> std::common_reference_with<I>;
        { i.TheSheet().begin() } -> std::common_reference_with<S>;
    };

template<typename S>
concept SheetRange =
    requires {
        requires ForwardRangeOfTupleLike<S>;
        requires SheetIterator<std::ranges::iterator_t<S>>;
    };

namespace {

template<SheetIterator I, SheetSentinelFor<I> S>
auto CheckAndGetSheetFromFirstLast(I first, S last) -> const auto& {
    if (&first.TheSheet() == &last.TheSheet()) { return first.TheSheet(); }
    throw std::invalid_argument{fmt::format("Algorithm on Mustard::Data::Sheet: first and last not reference to the same sheet (but {} and {})",
                                            static_cast<const void*>(&first.TheSheet()), static_cast<const void*>(&last.TheSheet()))};
}

} // namespace

template<muc::ceta_string AName,
         SheetIterator I, SheetSentinelFor<I> S,
         InvocableByName<I, AName> P,
         std::predicate<InvokeByNameResult<P, I, AName>> F>
auto AllAnyNoneOfCountFindIfOrNot(auto&& all_any_none_of_count_find_if_or_not, I first, S last, F&& Pred, P&& Proj) -> decltype(auto) {
    return CheckAndGetSheetFromFirstLast(first, last)
        .template DoWith<AName>([&] {
            return all_any_none_of_count_find_if_or_not(
                first, last,
                [&Pred](auto&& v) {
                    return std::forward<F>(Pred)(std::forward<decltype(v)>(v));
                },
                [&Proj](auto&& entry) {
                    return std::forward<P>(Proj)(*std::move(Get<AName>(entry)));
                });
        });
}

template<muc::ceta_string... ANames,
         SheetIterator I, SheetSentinelFor<I> S,
         InvocableByName<I, ANames...> P,
         muc::predicate_applicable_on<InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto AllAnyNoneOfCountFindIfOrNot(auto&& all_any_none_of_count_find_if_or_not, I first, S last, F&& Pred, P&& Proj) -> decltype(auto) {
    return CheckAndGetSheetFromFirstLast(first, last)
        .template DoWith<ANames...>([&] {
            return all_any_none_of_count_find_if_or_not(
                first, last,
                [&Pred](auto&& t) {
                    return muc::apply(std::forward<F>(Pred), std::forward<decltype(t)>(t));
                },
                [&Proj](auto&& entry) {
                    return std::forward<P>(Proj)(std::move(Get<ANames>(entry))...);
                });
        });
}

template<muc::ceta_string AName,
         SheetIterator I, SheetSentinelFor<I> S,
         InvocableByName<I, AName> P,
         std::invocable<InvokeByNameResult<P, I, AName>> F>
auto ForEach(auto&& for_each, I first, S last, F&& Func, P&& Proj) -> decltype(auto) {
    return CheckAndGetSheetFromFirstLast(first, last)
        .template DoWith<AName>([&] {
            return for_each(
                first, last,
                [&Func](auto&& v) {
                    std::forward<F>(Func)(std::forward<decltype(v)>(v));
                },
                [&Proj](auto&& entry) {
                    return std::forward<P>(Proj)(*std::move(Get<AName>(entry)));
                });
        });
}

template<muc::ceta_string... ANames,
         SheetIterator I, SheetSentinelFor<I> S,
         InvocableByName<I, ANames...> P,
         muc::applicable_on<InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto ForEach(auto&& for_each, I first, S last, F&& Func, P&& Proj) -> decltype(auto) {
    return CheckAndGetSheetFromFirstLast(first, last)
        .template DoWith<ANames...>([&] {
            return for_each(
                first, last,
                [&Func](auto&& t) {
                    muc::apply(std::forward<F>(Func), std::forward<decltype(t)>(t));
                },
                [&Proj](auto&& entry) {
                    return std::forward<P>(Proj)(std::move(Get<ANames>(entry))...);
                });
        });
}

template<muc::ceta_string AName,
         SheetIterator I, SheetSentinelFor<I> S,
         InvocableByName<I, AName> P>
auto CountFind(auto&& count_find, I first, S last, const UnderlyingType<I, AName>& value, P&& Proj) -> decltype(auto) {
    return CheckAndGetSheetFromFirstLast(first, last)
        .template DoWith<AName>([&] {
            return count_find(
                first, last,
                value,
                [&Proj](auto&& entry) {
                    return std::forward<P>(Proj)(*std::move(Get<AName>(entry)));
                });
        });
}

template<muc::ceta_string... ANames,
         SheetIterator I, SheetSentinelFor<I> S,
         EquivalentTuple<TupleType<I, ANames...>> ATuple,
         InvocableByName<I, ANames...> P>
auto CountFind(auto&& count_find, I first, S last, const ATuple& tuple, P&& Proj) -> decltype(auto) {
    return CheckAndGetSheetFromFirstLast(first, last)
        .template DoWith<ANames...>([&] {
            return count_find(
                first, last,
                tuple,
                [&Proj](auto&& entry) {
                    return std::forward<P>(Proj)(std::move(Get<ANames>(entry))...);
                });
        });
}

// template<muc::ceta_string AName,
//          SheetIterator I, SheetSentinelFor<I> S,
//          std::input_iterator I2, std::sentinel_for<I2> S2,
//          InvocableByName<I, AName> P1,
//          std::invocable<std::iter_reference_t<I2>> P2,
//          std::predicate<InvokeByNameResult<P1, I, AName>, std::invoke_result_t<P2, std::iter_reference_t<I2>>> F>
// auto Mismatch(auto&& mismatch_find_end_first_of, I first, S last, R&& r, F&& Pred, P1&& Proj1, P2&& Proj2) -> decltype(auto) {
//     return CheckAndGetSheetFromFirstLast(first, last)
//         .template DoWith<AName>([&] {
//             return mismatch_find_end_first_of(
//                 first, last,
//                 std::forward<R>(r),
//                 std::forward<F>(Pred),
//                 [&Proj1](auto&& entry) {
//                     return std::forward<P1>(Proj1)(*std::move(Get<AName>(entry)));
//                 },
//                 [&Proj2](auto&& v) {
//                     return std::forward<P2>(Proj2)(std::forward<decltype(v)>(v));
//                 });
//         });
// }

// template<muc::ceta_string... ANames,
//          SheetIterator I, SheetSentinelFor<I> S,
//          std::input_iterator I2, std::sentinel_for<I2> S2,
//          typename F,
//          InvocableByName<I, ANames...> P1,
//          typename P2>
//     requires EquivalentTuple<std::decay_t<std::iter_value_t<I2>>, TupleType<I, ANames...>> and
//              std::invocable<P2, typename std::decay_t<std::iter_value_t<I2>>::Model::template ValueOf<ANames>...> and
//              std::predicate<F, std::invoke_result_t<P1, typename I::Model::template ValueOf<ANames>...>,
//                             std::invoke_result_t<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...>>
// auto Mismatch(auto&& mismatch_find_end_first_of, I first1, S last1, I2 first2, S2 last2, R&& r, F&& Pred, P1&& Proj1, P2&& Proj2) -> decltype(auto) {
//     return CheckAndGetSheetFromFirstLast(first, last)
//         .template DoWith<ANames...>([&] {
//             return mismatch_find_end_first_of(
//                 first1, last1,
//                 first2, last2,
//                 std::forward<F>(Pred),
//                 [&Proj1](auto&& entry) {
//                     return std::forward<P1>(Proj1)(std::move(Get<ANames>(entry))...);
//                 },
//                 [&Proj2](auto&& t) {
//                     return std::forward<P2>(Proj2)(
//                         std::forward<decltype(Get<ANames>(std::forward<decltype(t)>(t)))>(
//                              Get<ANames>(std::forward<decltype(t)>(t)))...);
//                 });
//         });
// }

} // namespace Mustard::Data::internal
