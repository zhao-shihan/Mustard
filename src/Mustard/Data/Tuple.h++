#pragma once

#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/Value.h++"
#include "Mustard/Extension/gslx/index_sequence.h++"

#include "muc/ceta_string"
#include "muc/concepts"

#include "gsl/gsl"

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Mustard::Data {

template<typename T>
concept TupleLike = requires {
    typename T::Model;
    requires muc::instantiated_from<typename T::Model, TupleModel>;
    { T::Size() } -> std::same_as<std::size_t>;
    requires T::Size() >= 0;
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (... and requires(T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name()>(); });
    }(gslx::make_index_sequence<T::Size()>{}));
    requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return (... and requires(const T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name()>(); });
    }(gslx::make_index_sequence<T::Size()>{}));
    requires T::Size() <= 1 or requires {
        requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            return (... and requires(T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name(),
                                                           std::tuple_element_t<Is + 1, typename T::Model::StdTuple>::Name()>(); });
        }(gslx::make_index_sequence<T::Size() - 1>{}));
        requires([]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            return (... and requires(const T t) { t.template Get<std::tuple_element_t<Is, typename T::Model::StdTuple>::Name(),
                                                                 std::tuple_element_t<Is + 1, typename T::Model::StdTuple>::Name()>(); });
        }(gslx::make_index_sequence<T::Size() - 1>{}));
    };
};

template<typename T1, typename T2>
concept EquivalentTuple = TupleLike<T1> and TupleLike<T2> and
                          EquivalentTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept SubTuple = TupleLike<T1> and TupleLike<T2> and
                   SubTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept SuperTuple = TupleLike<T1> and TupleLike<T2> and
                     SuperTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept ProperSubTuple = TupleLike<T1> and TupleLike<T2> and
                         ProperSubTupleModel<typename T1::Model, typename T2::Model>;

template<typename T1, typename T2>
concept ProperSuperTuple = TupleLike<T1> and TupleLike<T2> and
                           ProperSuperTupleModel<typename T1::Model, typename T2::Model>;

} // namespace Mustard::Data

namespace std {

template<typename T>
    requires requires { typename T::Model; } and
             muc::instantiated_from<typename T::Model, Mustard::Data::TupleModel>
struct tuple_size<T>
    : tuple_size<typename T::Model::StdTuple> {};

template<std::size_t I, typename T>
    requires requires { typename T::Model; } and
             muc::instantiated_from<typename T::Model, Mustard::Data::TupleModel>
struct tuple_element<I, T>
    : tuple_element<I, typename T::Model::StdTuple> {};

} // namespace std

namespace Mustard::Data {

template<typename ADerived>
class EnableGet {
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(const ADerived& t) -> decltype(auto) { return t.template Get<ANames...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(ADerived& t) -> decltype(auto) { return t.template Get<ANames...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(ADerived&& t) -> decltype(auto) { return std::move(t).template Get<ANames...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    friend constexpr auto Get(const ADerived&& t) -> decltype(auto) { return std::move(t).template Get<ANames...>(); }

    template<gsl::index I>
    friend constexpr auto get(const ADerived& t) -> decltype(auto) { return t.template Get<std::tuple_element_t<I, ADerived>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(ADerived& t) -> decltype(auto) { return t.template Get<std::tuple_element_t<I, ADerived>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(ADerived&& t) -> decltype(auto) { return std::move(t).template Get<std::tuple_element_t<I, ADerived>::Name()>(); }
    template<gsl::index I>
    friend constexpr auto get(const ADerived&& t) -> decltype(auto) { return std::move(t).template Get<std::tuple_element_t<I, ADerived>::Name()>(); }

protected:
    constexpr EnableGet();
    constexpr ~EnableGet() = default;
};

template<TupleModelizable... Ts>
class Tuple : public EnableGet<Tuple<Ts...>> {
public:
    using Model = TupleModel<Ts...>;

private:
    struct Dummy {
        using Type = Dummy;
    };

public:
    constexpr Tuple() = default;

    template<TupleModelizable... Us>
        requires std::constructible_from<typename Model::StdTuple, const typename Tuple<Us...>::Model::StdTuple&>
    constexpr Tuple(const Tuple<Us...>& tuple) :
        fTuple{tuple.fTuple} {}

    template<TupleModelizable... Us>
        requires std::constructible_from<typename Model::StdTuple, typename Tuple<Us...>::Model::StdTuple&&>
    constexpr Tuple(Tuple<Us...>&& tuple) :
        fTuple{std::move(tuple.fTuple)} {}

    template<typename... Us>
        requires(sizeof...(Us) == Model::Size() and
                 []<gsl::index... Is>(gslx::index_sequence<Is...>) {
                     return (... and std::constructible_from<std::tuple_element_t<Is, typename Model::StdTuple>,
                                                             std::tuple_element_t<Is, std::tuple<Us...>>&&>);
                 }(gslx::make_index_sequence<sizeof...(Us)>()))
    constexpr explicit(sizeof...(Us) == 1) Tuple(Us&&... values) :
        fTuple{std::forward<Us>(values)...} {}

    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() const& -> decltype(auto) { return GetImpl<Model::template Index<ANames>()...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() & -> decltype(auto) { return GetImpl<Model::template Index<ANames>()...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() && -> decltype(auto) { return std::move(*this).template GetImpl<Model::template Index<ANames>()...>(); }
    template<muc::ceta_string... ANames>
        requires(sizeof...(ANames) >= 1)
    constexpr auto Get() const&& -> decltype(auto) { return std::move(*this).template GetImpl<Model::template Index<ANames>()...>(); }

    template<SubTuple<Tuple<Ts...>> ATuple>
    constexpr auto As() const -> auto { return AsImpl<ATuple>(); }
    template<std::same_as<Tuple<Ts...>> ATuple>
    constexpr auto As() const -> const auto& { return *this; }

    template<TupleLike ATuple>
    constexpr auto operator==(const ATuple& that) const -> auto;
    template<TupleLike ATuple>
    constexpr auto operator<=>(const ATuple&) const -> auto = delete;

    static constexpr auto Size() -> auto { return Model::Size(); }

private:
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() const& -> const std::tuple_element_t<I, Tuple>& { return std::get<I>(fTuple); }
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() & -> std::tuple_element_t<I, Tuple>& { return std::get<I>(fTuple); }
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() && -> std::tuple_element_t<I, Tuple>&& { return std::get<I>(std::move(fTuple)); }
    template<gsl::index I>
        requires(0 <= I and I < Model::Size())
    constexpr auto GetImpl() const&& -> const std::tuple_element_t<I, Tuple>&& { return std::get<I>(std::move(fTuple)); }

    template<gsl::index... Is>
        requires((sizeof...(Is) >= 2) and ... and (0 <= Is and Is < Model::Size()))
    constexpr auto GetImpl() const& -> auto { return Tuple<std::tuple_element_t<Is, Tuple>...>{std::get<Is>(fTuple)...}; }
    template<gsl::index... Is>
        requires((sizeof...(Is) >= 2) and ... and (0 <= Is and Is < Model::Size()))
    constexpr auto GetImpl() && -> auto { return Tuple<std::tuple_element_t<Is, Tuple>...>{std::get<Is>(std::move(fTuple))...}; }
    template<gsl::index... Is>
        requires((sizeof...(Is) >= 2) and ... and (0 <= Is and Is < Model::Size()))
    constexpr auto GetImpl() const&& -> auto { return Tuple<std::tuple_element_t<Is, Tuple>...>{std::get<Is>(std::move(fTuple))...}; }

    template<TupleLike ATuple>
    constexpr auto AsImpl() const -> ATuple;

private:
    typename Model::StdTuple fTuple;
};

template<typename... Ts>
Tuple(Ts...) -> Tuple<Ts...>;

} // namespace Mustard::Data

#include "Mustard/Data/Tuple.inl"
