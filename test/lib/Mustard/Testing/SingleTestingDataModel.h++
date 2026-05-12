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

struct TestingModel
    : Mustard::Data::Model<
          Mustard::Data::Value<std::int32_t, "EvtID">,
          Mustard::Data::Value<bool, "b">,
          Mustard::Data::Value<char, "ch">,
          Mustard::Data::Value<std::uint8_t, "u8">,
          Mustard::Data::Value<std::int16_t, "i16">,
          Mustard::Data::Value<std::uint16_t, "u16">,
          Mustard::Data::Value<std::int32_t, "i32">,
          Mustard::Data::Value<std::uint32_t, "u32">,
          Mustard::Data::Value<std::int64_t, "i64">,
          Mustard::Data::Value<std::uint64_t, "u64">,
          Mustard::Data::Value<float, "f32">,
          Mustard::Data::Value<double, "f64">,
          Mustard::Data::Value<std::string, "str">,
          Mustard::Data::Value<std::vector<std::int32_t>, "vi32">,
          Mustard::Data::Value<std::vector<double>, "vf64">,
          Mustard::Data::Value<std::vector<std::string>, "vstr">,
          Mustard::Data::Value<std::array<std::uint16_t, 3>, "au16">,
          Mustard::Data::Value<std::pair<std::int32_t, float>, "pi32f">,
          Mustard::Data::Value<std::tuple<std::uint8_t, double, std::string>, "tu8str">> {};

inline auto MakeEntry(gsl::index evtID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModel> {
    auto event{Mustard::Data::MakeArcTuple<TestingModel>()};
    Get<"EvtID">(*event) = static_cast<std::int32_t>(evtID);
    const auto i{evtID * 10 + entryIdx};
    Get<"b">(*event) = (i % 2 == 0);
    Get<"ch">(*event) = 'A' + i;
    Get<"u8">(*event) = 2 + i;
    Get<"i16">(*event) = -100 - i;
    Get<"u16">(*event) = 100 + i;
    Get<"i32">(*event) = -1000 - i;
    Get<"u32">(*event) = 1000 + i;
    Get<"i64">(*event) = -10000 - i;
    Get<"u64">(*event) = 10000 + i;
    Get<"f32">(*event) = 1.25f + i;
    Get<"f64">(*event) = 2.5 + i;
    Get<"str">(*event) = fmt::format("row-{}", i % 16);
    Get<"vi32">(*event) = {std::int32_t(i), std::int32_t(i + 1), std::int32_t(i + 2)};
    Get<"vf64">(*event) = {0.5 + i, 1.5 + i};
    Get<"vstr">(*event) = {fmt::format("x{}", i % 16), fmt::format("y{}", i % 16)};
    Get<"au16">(*event) = {std::uint16_t(10 + i), std::uint16_t(20 + i), std::uint16_t(30 + i)};
    Get<"pi32f">(*event) = {std::int32_t(7 + i), 3.0f + i};
    Get<"tu8str">(*event) = {std::uint8_t(9 + i), 6.25 + i, fmt::format("t{}", i)};
    return event;
}
