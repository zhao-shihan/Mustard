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
          Mustard::Data::Field<std::int32_t, "EvtID">,
          Mustard::Data::Field<bool, "b">,
          Mustard::Data::Field<char, "ch">,
          Mustard::Data::Field<std::uint8_t, "u8">,
          Mustard::Data::Field<std::int16_t, "i16">,
          Mustard::Data::Field<std::uint16_t, "u16">,
          Mustard::Data::Field<std::int32_t, "i32">,
          Mustard::Data::Field<std::uint32_t, "u32">,
          Mustard::Data::Field<std::int64_t, "i64">,
          Mustard::Data::Field<std::uint64_t, "u64">,
          Mustard::Data::Field<float, "f32">,
          Mustard::Data::Field<double, "f64">,
          Mustard::Data::Field<std::string, "str">,
          Mustard::Data::Field<std::vector<std::int32_t>, "vi32">,
          Mustard::Data::Field<std::vector<double>, "vf64">,
          Mustard::Data::Field<std::vector<std::string>, "vstr">,
          Mustard::Data::Field<std::array<std::uint16_t, 3>, "au16">,
          Mustard::Data::Field<std::pair<std::int32_t, float>, "pi32f">,
          Mustard::Data::Field<std::tuple<std::uint8_t, double, std::string>, "tu8str">> {};

auto MakeEntry(gsl::index evtID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModel>;
