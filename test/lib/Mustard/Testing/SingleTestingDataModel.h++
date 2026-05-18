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

#include "muc/array"

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

namespace Mustard::Testing {

struct TestingModel
    : Mustard::Data::Model<
          Mustard::Data::Field<int, "EvtID", std::int32_t>,
          Mustard::Data::Field<bool, "b">,
          Mustard::Data::Field<char, "ch">,
          Mustard::Data::Field<unsigned, "u8", std::uint8_t>,
          Mustard::Data::Field<int, "i16", std::int16_t>,
          Mustard::Data::Field<unsigned, "u16", std::uint16_t>,
          Mustard::Data::Field<int, "i32", std::int32_t>,
          Mustard::Data::Field<unsigned, "u32", std::uint32_t>,
          Mustard::Data::Field<long long, "i64", std::int64_t>,
          Mustard::Data::Field<unsigned long long, "u64", std::uint64_t>,
          Mustard::Data::Field<double, "f32", float>,
          Mustard::Data::Field<double, "f64">,
          Mustard::Data::Field<std::string, "str">,
          Mustard::Data::Field<std::vector<int>, "vi32", std::vector<std::int32_t>>,
          Mustard::Data::Field<std::vector<double>, "vf64">,
          Mustard::Data::Field<std::vector<std::string>, "vstr">,
          Mustard::Data::Field<muc::array3u, "a3u16", muc::array3u16>,
          Mustard::Data::Field<std::pair<int, double>, "pi32f32", std::pair<std::int32_t, float>>,
          Mustard::Data::Field<std::tuple<unsigned, double, std::string>, "tu8str", std::tuple<std::uint8_t, double, std::string>>> {};

auto MakeEntry(int eventID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModel>;

} // namespace Mustard::Testing
