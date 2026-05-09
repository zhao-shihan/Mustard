// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Tuple.h++"

#include "fmt/format.h"

#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

// Limitation notice (ROOT 6.36):
// - TTree does not support std::int8_t.
// - TTree does not support std::array<std::string, N>.
// - TTree does not support some map/set families types, but key_type=std::string/int and
//     mapped_type=int/float/double/std::string types are generally safe.
// - as well as some other types... you should test before applying to TTree.
// - RNTuple support to map/set families are broken.

struct TestingModelA
    : Mustard::Data::Model<
          Mustard::Data::Value<std::int32_t, "EvtID">,
          Mustard::Data::Value<std::int32_t, "i32">,
          Mustard::Data::Value<double, "f64">,
          Mustard::Data::Value<std::string, "str">,
          Mustard::Data::Value<bool, "b">,
          Mustard::Data::Value<char, "ch">,
          Mustard::Data::Value<std::int64_t, "i64">,
          Mustard::Data::Value<std::uint32_t, "u32">,
          Mustard::Data::Value<float, "f32">,
          Mustard::Data::Value<std::vector<std::int32_t>, "vi32">> {};

struct TestingModelB
    : Mustard::Data::Model<
          Mustard::Data::Value<std::int32_t, "EvtID">,
          Mustard::Data::Value<float, "f32">,
          Mustard::Data::Value<bool, "b">,
          Mustard::Data::Value<std::vector<int>, "vi">,
          Mustard::Data::Value<std::vector<double>, "vf64">,
          Mustard::Data::Value<std::vector<std::string>, "vstr">,
          Mustard::Data::Value<std::uint8_t, "u8">,
          Mustard::Data::Value<std::int16_t, "i16">,
          Mustard::Data::Value<std::pair<float, double>, "pf32f64">,
          Mustard::Data::Value<std::tuple<std::uint16_t, bool, std::string>, "tu16bstr">> {};

struct TestingModelC
    : Mustard::Data::Model<
          Mustard::Data::Value<std::int32_t, "EvtID">,
          Mustard::Data::Value<std::array<double, 3>, "a3f64">,
          Mustard::Data::Value<std::pair<int, int>, "p11">,
          Mustard::Data::Value<std::array<std::uint32_t, 4>, "a4u32">,
          Mustard::Data::Value<std::string, "str">,
          Mustard::Data::Value<std::int64_t, "i64">,
          Mustard::Data::Value<double, "f64">,
          Mustard::Data::Value<std::tuple<bool, char, std::int32_t>, "tbchi32">> {};

auto MakeEntryA(gsl::index evtID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelA> {
    auto entry{Mustard::Data::MakeArcTuple<TestingModelA>()};
    Get<"EvtID">(*entry) = static_cast<std::int32_t>(evtID);
    const auto i{evtID * 10 + entryIdx};
    Get<"i32">(*entry) = -1000 - i;
    Get<"f64">(*entry) = 2.5 + i;
    Get<"str">(*entry) = fmt::format("modelA-{}", i);
    Get<"b">(*entry) = (i % 2 == 0);
    Get<"ch">(*entry) = 'A' + (i % 26);
    Get<"i64">(*entry) = -10000 - i;
    Get<"u32">(*entry) = 1000 + i;
    Get<"f32">(*entry) = 1.25f + i;
    Get<"vi32">(*entry) = {std::int32_t(i), std::int32_t(i + 1), std::int32_t(i + 2)};
    return entry;
}

auto MakeEntryB(gsl::index evtID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelB> {
    auto entry{Mustard::Data::MakeArcTuple<TestingModelB>()};
    Get<"EvtID">(*entry) = static_cast<std::int32_t>(evtID);
    const auto i{evtID * 10 + entryIdx};
    Get<"f32">(*entry) = 0.5f + i;
    Get<"b">(*entry) = (i % 3 == 0);
    Get<"vi">(*entry) = {int(i), int(i * 2), int(i * 3)};
    Get<"vf64">(*entry) = {0.1 + i, 0.2 + i, 0.3 + i};
    Get<"vstr">(*entry) = {fmt::format("b{}", i), fmt::format("c{}", i)};
    Get<"u8">(*entry) = i % 256;
    Get<"i16">(*entry) = -500 - i;
    Get<"pf32f64">(*entry) = {1.0f + i, 2.0 + i};
    Get<"tu16bstr">(*entry) = {std::uint16_t(100 + i), (i % 4 == 0), fmt::format("tuple{}", i)};
    return entry;
}

auto MakeEntryC(gsl::index evtID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelC> {
    auto entry{Mustard::Data::MakeArcTuple<TestingModelC>()};
    Get<"EvtID">(*entry) = static_cast<std::int32_t>(evtID);
    const auto i{evtID * 10 + entryIdx};
    Get<"a3f64">(*entry) = {1.0 + i, 2.0 + i, 3.0 + i};
    Get<"p11">(*entry) = {i, -i};
    Get<"a4u32">(*entry) = {std::uint32_t(10 + i), std::uint32_t(20 + i), std::uint32_t(30 + i), std::uint32_t(40 + i)};
    Get<"str">(*entry) = fmt::format("modelC-{}", i);
    Get<"i64">(*entry) = 10000 + i;
    Get<"f64">(*entry) = 3.14159 + i;
    Get<"tbchi32">(*entry) = {(i % 2 == 0), 'Z' - (i % 26), static_cast<std::int32_t>(i * 10)};
    return entry;
}
