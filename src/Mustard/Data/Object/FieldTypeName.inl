// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

namespace Mustard::Data::inline Object {

template<FieldAcceptable T>
auto FieldTypeName() -> std::string {
    if constexpr (std::same_as<T, bool>) {
        return "b";
    } else if constexpr (std::same_as<T, char>) {
        return "ch";
    } else if constexpr (std::same_as<T, std::int8_t>) {
        return "i8";
    } else if constexpr (std::same_as<T, std::uint8_t>) {
        return "u8";
    } else if constexpr (std::same_as<T, std::int16_t>) {
        return "i16";
    } else if constexpr (std::same_as<T, std::uint16_t>) {
        return "u16";
    } else if constexpr (std::same_as<T, std::int32_t>) {
        return "i32";
    } else if constexpr (std::same_as<T, std::uint32_t>) {
        return "u32";
    } else if constexpr (std::same_as<T, std::int64_t>) {
        return "i64";
    } else if constexpr (std::same_as<T, std::uint64_t>) {
        return "u64";
    } else if constexpr (std::same_as<T, float>) {
        return "f32";
    } else if constexpr (std::same_as<T, double>) {
        return "f64";
    } else if constexpr (std::same_as<T, std::string>) {
        return "str";
    } else if constexpr (impl::IsStdBitset<T>{}) {
        return fmt::format("bset<{}>", T{}.size());
    } else if constexpr (impl::IsStdPair<T>{}) {
        const auto firstName{FieldTypeName<typename T::first_type>()};
        const auto secondName{FieldTypeName<typename T::second_type>()};
        return fmt::format("pair<{},{}>", std::move(firstName), std::move(secondName));
    } else if constexpr (impl::IsStdTuple<T>{}) {
        std::string result{"tup<"};
        [&result]<gsl::index... Is>(gslx::index_sequence<Is...>) {
            (..., (result += FieldTypeName<std::tuple_element_t<Is, T>>() + (Is < sizeof...(Is) - 1 ? "," : "")));
        }(gslx::make_index_sequence<std::tuple_size_v<T>>{});
        result += ">";
        return result;
    } else if constexpr (impl::IsStdVector<T>{}) {
        return fmt::format("vec<{}>", FieldTypeName<typename T::value_type>());
    } else if constexpr (impl::IsStdArray<T>{}) {
        return fmt::format("arr<{},{}>", FieldTypeName<typename T::value_type>(), std::tuple_size_v<T>);
    } else if constexpr (impl::IsStdOptional<T>{}) {
        return fmt::format("opt<{}>", FieldTypeName<typename T::value_type>());
    } else if constexpr (impl::IsStdSet<T>{}) {
        return fmt::format("set<{}>", FieldTypeName<typename T::key_type>());
    } else if constexpr (impl::IsStdUnorderedSet<T>{}) {
        return fmt::format("uset<{}>", FieldTypeName<typename T::key_type>());
    } else if constexpr (impl::IsStdMultiSet<T>{}) {
        return fmt::format("mset<{}>", FieldTypeName<typename T::key_type>());
    } else if constexpr (impl::IsStdUnorderedMultiSet<T>{}) {
        return fmt::format("umset<{}>", FieldTypeName<typename T::key_type>());
    } else if constexpr (impl::IsStdMap<T>{}) {
        auto keyName{FieldTypeName<typename T::key_type>()};
        auto mappedName{FieldTypeName<typename T::mapped_type>()};
        return fmt::format("map<{},{}>", std::move(keyName), std::move(mappedName));
    } else if constexpr (impl::IsStdUnorderedMap<T>{}) {
        auto keyName{FieldTypeName<typename T::key_type>()};
        auto mappedName{FieldTypeName<typename T::mapped_type>()};
        return fmt::format("umap<{},{}>", std::move(keyName), std::move(mappedName));
    } else if constexpr (impl::IsStdMultiMap<T>{}) {
        auto keyName{FieldTypeName<typename T::key_type>()};
        auto mappedName{FieldTypeName<typename T::mapped_type>()};
        return fmt::format("mmap<{},{}>", std::move(keyName), std::move(mappedName));
    } else if constexpr (impl::IsStdUnorderedMultiMap<T>{}) {
        auto keyName{FieldTypeName<typename T::key_type>()};
        auto mappedName{FieldTypeName<typename T::mapped_type>()};
        return fmt::format("ummap<{},{}>", std::move(keyName), std::move(mappedName));
    } else {
        static_assert(muc::dependent_false<T>, "Unsupported field type");
    }
}

} // namespace Mustard::Data::inline Object
