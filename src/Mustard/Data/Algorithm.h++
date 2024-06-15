#pragma once

#include "Mustard/Data/internal/Algorithm.h++"

#include "muc/tuple"

#include <algorithm>
#include <concepts>
#include <type_traits>

namespace Mustard::Data {

struct TuplifyIdentity {
    template<typename... Us>
        requires(... and TupleModelizable<std::decay_t<Us>>)
    constexpr auto operator()(Us&&... args) const -> auto { return Tuple{std::forward<Us>(args)...}; }
};

// std::ranges::all_of

template<muc::ceta_string AName,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, AName> P = std::identity,
         std::predicate<internal::InvokeByNameResult<P, I, AName>> F>
auto AllOf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<AName>(std::ranges::all_of,
                                                         first, last,
                                                         std::forward<F>(Pred),
                                                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, ANames...> P = TuplifyIdentity,
         muc::predicate_applicable_on<internal::InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto AllOf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<ANames...>(std::ranges::all_of,
                                                             first, last,
                                                             std::forward<F>(Pred),
                                                             std::forward<P>(Proj));
}

template<muc::ceta_string AName,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity,
         std::predicate<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>> F>
auto AllOf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return AllOf<AName>(std::ranges::begin(sheet),
                        std::ranges::end(sheet),
                        std::forward<F>(Pred),
                        std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
         muc::predicate_applicable_on<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<ANames>...>> F>
    requires(sizeof...(ANames) >= 2)
auto AllOf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return AllOf<ANames...>(std::ranges::begin(sheet),
                            std::ranges::end(sheet),
                            std::forward<F>(Pred),
                            std::forward<P>(Proj));
}

// std::ranges::any_of

template<muc::ceta_string AName,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, AName> P = std::identity,
         std::predicate<internal::InvokeByNameResult<P, I, AName>> F>
auto AnyOf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<AName>(std::ranges::any_of,
                                                         first, last,
                                                         std::forward<F>(Pred),
                                                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, ANames...> P = TuplifyIdentity,
         muc::predicate_applicable_on<internal::InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto AnyOf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<ANames...>(std::ranges::any_of,
                                                             first, last,
                                                             std::forward<F>(Pred),
                                                             std::forward<P>(Proj));
}

template<muc::ceta_string AName,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity,
         std::predicate<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>> F>
auto AnyOf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return AnyOf<AName>(std::ranges::begin(sheet),
                        std::ranges::end(sheet),
                        std::forward<F>(Pred),
                        std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
         muc::predicate_applicable_on<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<ANames>...>> F>
    requires(sizeof...(ANames) >= 2)
auto AnyOf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return AnyOf<ANames...>(std::ranges::begin(sheet),
                            std::ranges::end(sheet),
                            std::forward<F>(Pred),
                            std::forward<P>(Proj));
}

// std::ranges::none_of

template<muc::ceta_string AName,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, AName> P = std::identity,
         std::predicate<internal::InvokeByNameResult<P, I, AName>> F>
auto NoneOf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<AName>(std::ranges::none_of,
                                                         first, last,
                                                         std::forward<F>(Pred),
                                                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, ANames...> P = TuplifyIdentity,
         muc::predicate_applicable_on<internal::InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto NoneOf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<ANames...>(std::ranges::none_of,
                                                             first, last,
                                                             std::forward<F>(Pred),
                                                             std::forward<P>(Proj));
}

template<muc::ceta_string AName,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity,
         std::predicate<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>> F>
auto NoneOf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return NoneOf<AName>(std::ranges::begin(sheet),
                         std::ranges::end(sheet),
                         std::forward<F>(Pred),
                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
         muc::predicate_applicable_on<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<ANames>...>> F>
    requires(sizeof...(ANames) >= 2)
auto NoneOf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return NoneOf<ANames...>(std::ranges::begin(sheet),
                             std::ranges::end(sheet),
                             std::forward<F>(Pred),
                             std::forward<P>(Proj));
}

// // std::ranges::for_each

// template<muc::ceta_string AName,
//          internal::SheetIterator I, internal::SheetSentinelFor<I> S,
//          std::invocable<typename I::Model::template ValueOf<AName>::Type> P = std::identity,
//          std::invocable<std::invoke_result_t<P, typename I::Model::template ValueOf<AName>::Type>> F>
// auto ForEach(I first, S last, F&& Func, P&& Proj = {}) -> decltype(auto) {
//     return internal::ForEach<AName>(std::ranges::for_each,
//                                     first, last,
//                                     std::forward<F>(Func),
//                                     std::forward<P>(Proj));
// }

// template<muc::ceta_string... ANames,
//          internal::SheetIterator I, internal::SheetSentinelFor<I> S,
//          std::invocable<typename I::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
//          muc::applicable<std::invoke_result_t<P, typename I::Model::template ValueOf<ANames>...>> F>
//     requires(sizeof...(ANames) >= 2)
// auto ForEach(I first, S last, F&& Func, P&& Proj = {}) -> decltype(auto) {
//     return internal::ForEach<ANames...>(std::ranges::for_each,
//                                         first, last,
//                                         std::forward<F>(Func),
//                                         std::forward<P>(Proj));
// }

// template<muc::ceta_string AName,
//          internal::SheetRange S,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity,
//          std::invocable<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>> F>
// auto ForEach(S&& sheet, F&& Func, P&& Proj = {}) -> decltype(auto) {
//     return ForEach<AName>(std::ranges::begin(sheet),
//                           std::ranges::end(sheet),
//                           std::forward<F>(Func),
//                           std::forward<P>(Proj));
// }

// template<muc::ceta_string... ANames,
//          internal::SheetRange S,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
//          muc::applicable<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<ANames>...>> F>
//     requires(sizeof...(ANames) >= 2)
// auto ForEach(S&& sheet, F&& Func, P&& Proj = {}) -> decltype(auto) {
//     return ForEach<ANames...>(std::ranges::begin(sheet),
//                               std::ranges::end(sheet),
//                               std::forward<F>(Func),
//                               std::forward<P>(Proj));
// }

// // std::ranges::count

// template<muc::ceta_string AName,
//          internal::SheetIterator I, internal::SheetSentinelFor<I> S,
//          std::invocable<typename I::Model::template ValueOf<AName>::Type> P = std::identity>
// auto Count(I first, S last, const typename I::Model::template ValueOf<AName>::Type& value, P&& Proj = {}) -> decltype(auto) {
//     return internal::CountFind<AName>(std::ranges::count,
//                                       first, last,
//                                       value,
//                                       std::forward<P>(Proj));
// }

// template<muc::ceta_string... ANames,
//          internal::SheetIterator I, internal::SheetSentinelFor<I> S,
//          EquivalentTuple<Tuple<typename I::Model::template ValueOf<ANames>...>> ATuple = Tuple<typename I::Model::template ValueOf<ANames>...>,
//          std::invocable<typename I::Model::template ValueOf<ANames>...> P = TuplifyIdentity>
// auto Count(I first, S last, const ATuple& tuple, P&& Proj = {}) -> decltype(auto) {
//     return internal::CountFind<ANames...>(std::ranges::count,
//                                           first, last,
//                                           tuple,
//                                           std::forward<P>(Proj));
// }

// template<muc::ceta_string AName,
//          internal::SheetRange S,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity>
// auto Count(S&& sheet, const typename std::decay_t<S>::Model::template ValueOf<AName>::Type& value, P&& Proj = {}) -> decltype(auto) {
//     return Count<AName>(std::ranges::begin(sheet),
//                         std::ranges::end(sheet),
//                         value,
//                         std::forward<P>(Proj));
// }

// template<muc::ceta_string... ANames,
//          internal::SheetRange S,
//          EquivalentTuple<Tuple<typename std::decay_t<S>::Model::template ValueOf<ANames>...>> ATuple = Tuple<typename std::decay_t<S>::Model::template ValueOf<ANames>...>,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity>
// auto Count(S&& sheet, const ATuple& tuple, P&& Proj = {}) -> decltype(auto) {
//     return Count<ANames...>(std::ranges::begin(sheet),
//                             std::ranges::end(sheet),
//                             tuple,
//                             std::forward<P>(Proj));
// }

// std::ranges::count_if

template<muc::ceta_string AName,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, AName> P = std::identity,
         std::predicate<internal::InvokeByNameResult<P, I, AName>> F>
auto CountIf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<AName>(std::ranges::count_if,
                                                         first, last,
                                                         std::forward<F>(Pred),
                                                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, ANames...> P = TuplifyIdentity,
         muc::predicate_applicable_on<internal::InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto CountIf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<ANames...>(std::ranges::count_if,
                                                             first, last,
                                                             std::forward<F>(Pred),
                                                             std::forward<P>(Proj));
}

template<muc::ceta_string AName,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity,
         std::predicate<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>> F>
auto CountIf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return CountIf<AName>(std::ranges::begin(sheet),
                          std::ranges::end(sheet),
                          std::forward<F>(Pred),
                          std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
         muc::predicate_applicable_on<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<ANames>...>> F>
    requires(sizeof...(ANames) >= 2)
auto CountIf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return CountIf<ANames...>(std::ranges::begin(sheet),
                              std::ranges::end(sheet),
                              std::forward<F>(Pred),
                              std::forward<P>(Proj));
}

// // std::ranges::mismatch

// template<muc::ceta_string AName,
//          SheetIterator I, SheetSentinelFor<I> S,
//          std::ranges::input_range R = std::initializer_list<typename I::Model::template ValueOf<AName>::Type>,
//          std::invocable<typename I::Model::template ValueOf<AName>::Type> P1 = std::identity,
//          std::invocable<std::ranges::range_value_t<R>> P2 = std::identity,
//          std::predicate<std::invoke_result_t<P1, typename I::Model::template ValueOf<AName>::Type>, std::invoke_result_t<P2, std::ranges::range_value_t<R>>> F = std::equal_to<>>
// auto Mismatch(I first, S last, R&& r, F&& Pred = {}, P1&& Proj1 = {}, P2&& Proj2 = {}) -> decltype(auto) {
//     return internal::Mismatch<AName>(std::ranges::mismatch,
//                                      first, last,
//                                      std::forward<R>(r),
//                                      std::forward<F>(Pred),
//                                      std::forward<P1>(Proj1),
//                                      std::forward<P2>(Proj2));
// }

// template<muc::ceta_string... ANames,
//          SheetIterator I, SheetSentinelFor<I> S,
//          std::ranges::input_range R = std::initializer_list<Tuple<typename I::Model::template ValueOf<ANames>...>>,
//          typename F = std::equal_to<>,
//          std::invocable<typename I::Model::template ValueOf<ANames>...> P1 = TuplifyIdentity,
//          typename P2 = TuplifyIdentity>
//     requires EquivalentTuple<std::decay_t<std::ranges::range_value_t<R>>, Tuple<typename I::Model::template ValueOf<ANames>...>> and
//              std::invocable<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...> and
//              std::predicate<F, std::invoke_result_t<P1, typename I::Model::template ValueOf<ANames>...>,
//                             std::invoke_result_t<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...>>
// auto Mismatch(I first, S last, R&& r, F&& Pred, P1&& Proj1, P2&& Proj2) -> decltype(auto) {
//     return internal::Mismatch<ANames...>(std::ranges::mismatch,
//                                          first, last,
//                                          std::forward<R>(r),
//                                          std::forward<F>(Pred),
//                                          std::forward<P1>(Proj1),
//                                          std::forward<P2>(Proj2));
// }

// template<muc::ceta_string AName,
//          internal::SheetRange S,
//          std::ranges::input_range R = std::initializer_list<typename std::decay_t<S>::Model::template ValueOf<AName>::Type>,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P1 = std::identity,
//          std::invocable<std::ranges::range_value_t<R>> P2 = std::identity,
//          std::predicate<std::invoke_result_t<P1, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>, std::invoke_result_t<P2, std::ranges::range_value_t<R>>> F = std::equal_to<>>
// auto Mismatch(S&& sheet, R&& r, F&& Pred = {}, P1&& Proj1 = {}, P2&& Proj2 = {}) -> decltype(auto) {
//     return Mismatch<AName>(std::ranges::begin(sheet),
//                            std::ranges::end(sheet),
//                            std::forward<R>(r),
//                            std::forward<F>(Pred),
//                            std::forward<P1>(Proj1),
//                            std::forward<P2>(Proj2));
// }

// template<muc::ceta_string... ANames,
//          internal::SheetRange S,
//          std::ranges::input_range R = std::initializer_list<Tuple<typename std::decay_t<S>::Model::template ValueOf<ANames>...>>,
//          typename F = std::equal_to<>,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P1 = TuplifyIdentity,
//          typename P2 = TuplifyIdentity>
//     requires EquivalentTuple<std::decay_t<std::ranges::range_value_t<R>>, Tuple<typename std::decay_t<S>::Model::template ValueOf<ANames>...>> and
//              std::invocable<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...> and
//              std::predicate<F, std::invoke_result_t<P1, typename std::decay_t<S>::Model::template ValueOf<ANames>...>,
//                             std::invoke_result_t<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...>>
// auto Mismatch(S&& sheet, R&& r, F&& Pred, P1&& Proj1, P2&& Proj2) -> decltype(auto) {
//     return Mismatch<ANames...>(std::ranges::begin(sheet),
//                                std::ranges::end(sheet),
//                                std::forward<R>(r),
//                                std::forward<F>(Pred),
//                                std::forward<P1>(Proj1),
//                                std::forward<P2>(Proj2));
// }

// // std::ranges::find

// template<muc::ceta_string AName,
//          internal::SheetIterator I, internal::SheetSentinelFor<I> S,
//          std::invocable<typename I::Model::template ValueOf<AName>::Type> P = std::identity>
// auto Find(I first, S last, const typename I::Model::template ValueOf<AName>::Type& value, P&& Proj = {}) -> decltype(auto) {
//     return internal::CountFind<AName>(std::ranges::find,
//                                       first, last,
//                                       value,
//                                       std::forward<P>(Proj));
// }

// template<muc::ceta_string... ANames,
//          internal::SheetIterator I, internal::SheetSentinelFor<I> S,
//          EquivalentTuple<Tuple<typename I::Model::template ValueOf<ANames>...>> ATuple = Tuple<typename I::Model::template ValueOf<ANames>...>,
//          std::invocable<typename I::Model::template ValueOf<ANames>...> P = TuplifyIdentity>
// auto Find(I first, S last, const ATuple& tuple, P&& Proj = {}) -> decltype(auto) {
//     return internal::CountFind<ANames...>(std::ranges::find,
//                                           first, last,
//                                           tuple,
//                                           std::forward<P>(Proj));
// }

// template<muc::ceta_string AName,
//          internal::SheetRange S,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity>
// auto Find(S&& sheet, const typename std::decay_t<S>::Model::template ValueOf<AName>::Type& value, P&& Proj = {}) -> decltype(auto) {
//     return Find<AName>(std::ranges::begin(sheet),
//                        std::ranges::end(sheet),
//                        value,
//                        std::forward<P>(Proj));
// }

// template<muc::ceta_string... ANames,
//          internal::SheetRange S,
//          EquivalentTuple<Tuple<typename std::decay_t<S>::Model::template ValueOf<ANames>...>> ATuple = Tuple<typename std::decay_t<S>::Model::template ValueOf<ANames>...>,
//          std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity>
// auto Find(S&& sheet, const ATuple& tuple, P&& Proj = {}) -> decltype(auto) {
//     return Find<ANames...>(std::ranges::begin(sheet),
//                            std::ranges::end(sheet),
//                            tuple,
//                            std::forward<P>(Proj));
// }

// std::ranges::find_if

template<muc::ceta_string AName,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, AName> P = std::identity,
         std::predicate<internal::InvokeByNameResult<P, I, AName>> F>
auto FindIf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<AName>(std::ranges::find_if,
                                                         first, last,
                                                         std::forward<F>(Pred),
                                                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, ANames...> P = TuplifyIdentity,
         muc::predicate_applicable_on<internal::InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto FindIf(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<ANames...>(std::ranges::find_if,
                                                             first, last,
                                                             std::forward<F>(Pred),
                                                             std::forward<P>(Proj));
}

template<muc::ceta_string AName,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity,
         std::predicate<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>> F>
auto FindIf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return FindIf<AName>(std::ranges::begin(sheet),
                         std::ranges::end(sheet),
                         std::forward<F>(Pred),
                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
         muc::predicate_applicable_on<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<ANames>...>> F>
    requires(sizeof...(ANames) >= 2)
auto FindIf(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return FindIf<ANames...>(std::ranges::begin(sheet),
                             std::ranges::end(sheet),
                             std::forward<F>(Pred),
                             std::forward<P>(Proj));
}

// std::ranges::find_if_not

template<muc::ceta_string AName,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, AName> P = std::identity,
         std::predicate<internal::InvokeByNameResult<P, I, AName>> F>
auto FindIfNot(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<AName>(std::ranges::find_if_not,
                                                         first, last,
                                                         std::forward<F>(Pred),
                                                         std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetIterator I, internal::SheetSentinelFor<I> S,
         internal::InvocableByName<I, ANames...> P = TuplifyIdentity,
         muc::predicate_applicable_on<internal::InvokeByNameResult<P, I, ANames...>> F>
    requires(sizeof...(ANames) >= 2)
auto FindIfNot(I first, S last, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return internal::AllAnyNoneOfCountFindIfOrNot<ANames...>(std::ranges::find_if_not,
                                                             first, last,
                                                             std::forward<F>(Pred),
                                                             std::forward<P>(Proj));
}

template<muc::ceta_string AName,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<AName>::Type> P = std::identity,
         std::predicate<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<AName>::Type>> F>
auto FindIfNot(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return FindIfNot<AName>(std::ranges::begin(sheet),
                            std::ranges::end(sheet),
                            std::forward<F>(Pred),
                            std::forward<P>(Proj));
}

template<muc::ceta_string... ANames,
         internal::SheetRange S,
         std::invocable<typename std::decay_t<S>::Model::template ValueOf<ANames>...> P = TuplifyIdentity,
         muc::predicate_applicable_on<std::invoke_result_t<P, typename std::decay_t<S>::Model::template ValueOf<ANames>...>> F>
    requires(sizeof...(ANames) >= 2)
auto FindIfNot(S&& sheet, F&& Pred, P&& Proj = {}) -> decltype(auto) {
    return FindIfNot<ANames...>(std::ranges::begin(sheet),
                                std::ranges::end(sheet),
                                std::forward<F>(Pred),
                                std::forward<P>(Proj));
}

// // std::ranges::find_end

// template<muc::ceta_string AName,
//          std::ranges::input_range R = std::initializer_list<typename I::Model::template ValueOf<AName>::Type>,
//          std::invocable<typename I::Model::template ValueOf<AName>::Type> P1 = std::identity,
//          std::invocable<std::ranges::range_value_t<R>> P2 = std::identity,
//          std::predicate<std::invoke_result_t<P1, typename I::Model::template ValueOf<AName>::Type>, std::invoke_result_t<P2, std::ranges::range_value_t<R>>> F = std::equal_to<>>
// auto FindEnd(R&& r, F&& Pred = {}, P1&& Proj1 = {}, P2&& Proj2 = {}) -> decltype(auto) {
//     return internal::FindEndFirstOf<AName>(std::ranges::mismatch, std::forward<R>(r), std::forward<F>(Pred), std::forward<P1>(Proj1), std::forward<P2>(Proj2));
// }
// template<muc::ceta_string... ANames,
//          std::ranges::input_range R = std::initializer_list<Tuple<typename I::Model::template ValueOf<ANames>...>>,
//          typename F = std::equal_to<>,
//          std::invocable<typename I::Model::template ValueOf<ANames>...> P1 = TuplifyIdentity,
//          typename P2 = TuplifyIdentity>
//     requires TupleLike<std::decay_t<std::ranges::range_value_t<R>>> and
//              EquivalentTuple<std::decay_t<std::ranges::range_value_t<R>>, Tuple<typename I::Model::template ValueOf<ANames>...>> and
//              std::invocable<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...> and
//              std::predicate<F, std::invoke_result_t<P1, typename I::Model::template ValueOf<ANames>...>,
//                             std::invoke_result_t<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...>>
// auto FindEnd(R&& r, F&& Pred = {}, P1&& Proj1 = {}, P2&& Proj2 = {}) -> decltype(auto) {
//     return internal::FindEndFirstOf<ANames...>(std::ranges::mismatch, std::forward<R>(r), std::forward<F>(Pred), std::forward<P1>(Proj1), std::forward<P2>(Proj2));
// }

// // std::ranges::find_first_of

// template<muc::ceta_string AName,
//          std::ranges::input_range R = std::initializer_list<typename I::Model::template ValueOf<AName>::Type>,
//          std::invocable<typename I::Model::template ValueOf<AName>::Type> P1 = std::identity,
//          std::invocable<std::ranges::range_value_t<R>> P2 = std::identity,
//          std::predicate<std::invoke_result_t<P1, typename I::Model::template ValueOf<AName>::Type>, std::invoke_result_t<P2, std::ranges::range_value_t<R>>> F = std::equal_to<>>
// auto FindFirstOf(R&& r, F&& Pred = {}, P1&& Proj1 = {}, P2&& Proj2 = {}) -> decltype(auto) {
//     return internal::FindEndFirstOf<AName>(std::ranges::mismatch, std::forward<R>(r), std::forward<F>(Pred), std::forward<P1>(Proj1), std::forward<P2>(Proj2));
// }
// template<muc::ceta_string... ANames,
//          std::ranges::input_range R = std::initializer_list<Tuple<typename I::Model::template ValueOf<ANames>...>>,
//          typename F = std::equal_to<>,
//          std::invocable<typename I::Model::template ValueOf<ANames>...> P1 = TuplifyIdentity,
//          typename P2 = TuplifyIdentity>
//     requires EquivalentTuple<std::decay_t<std::ranges::range_value_t<R>>, Tuple<typename I::Model::template ValueOf<ANames>...>> and
//              std::invocable<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...> and
//              std::predicate<F, std::invoke_result_t<P1, typename I::Model::template ValueOf<ANames>...>,
//                             std::invoke_result_t<P2, typename std::decay_t<std::ranges::range_value_t<R>>::Model::template ValueOf<ANames>...>>
// auto FindFirstOf(R&& r, F&& Pred = {}, P1&& Proj1 = {}, P2&& Proj2 = {}) -> decltype(auto) {
//     return internal::FindEndFirstOf<ANames...>(std::ranges::mismatch, std::forward<R>(r), std::forward<F>(Pred), std::forward<P1>(Proj1), std::forward<P2>(Proj2));
// }

} // namespace Mustard::Data
