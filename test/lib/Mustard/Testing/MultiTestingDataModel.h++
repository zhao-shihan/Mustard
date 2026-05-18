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

#include <array>
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

struct TestingModelA
    : Mustard::Data::Model<
          Mustard::Data::Field<int, std::int32_t, "EvtID">,
          Mustard::Data::Field<int, std::int32_t, "i32">,
          Mustard::Data::Field<double, double, "f64">,
          Mustard::Data::Field<std::string, std::string, "str">,
          Mustard::Data::Field<bool, bool, "b">,
          Mustard::Data::Field<char, char, "ch">,
          Mustard::Data::Field<long long, std::int64_t, "i64">,
          Mustard::Data::Field<unsigned, std::uint32_t, "u32">,
          Mustard::Data::Field<double, float, "f32">,
          Mustard::Data::Field<std::vector<int>, std::vector<std::int32_t>, "vi32">> {};

struct TestingModelB
    : Mustard::Data::Model<
          Mustard::Data::Field<int, std::int32_t, "EvtID">,
          Mustard::Data::Field<double, float, "f32">,
          Mustard::Data::Field<bool, bool, "b">,
          Mustard::Data::Field<std::vector<int>, std::vector<std::int32_t>, "vi32">,
          Mustard::Data::Field<std::vector<double>, std::vector<double>, "vf64">,
          Mustard::Data::Field<std::vector<std::string>, std::vector<std::string>, "vstr">,
          Mustard::Data::Field<unsigned, std::uint8_t, "u8">,
          Mustard::Data::Field<int, std::int16_t, "i16">,
          Mustard::Data::Field<std::pair<double, double>, std::pair<float, double>, "pf32f64">,
          Mustard::Data::Field<std::tuple<unsigned, bool, std::string>, std::tuple<std::uint16_t, bool, std::string>, "tu16bstr">> {};

struct TestingModelC
    : Mustard::Data::Model<
          Mustard::Data::Field<int, std::int32_t, "EvtID">,
          Mustard::Data::Field<std::array<double, 3>, std::array<double, 3>, "a3f64">,
          Mustard::Data::Field<std::pair<int, int>, std::pair<std::int32_t, std::int32_t>, "pi32i32">,
          Mustard::Data::Field<std::array<unsigned, 4>, std::array<std::uint32_t, 4>, "a4u32">,
          Mustard::Data::Field<std::string, std::string, "str">,
          Mustard::Data::Field<long long, std::int64_t, "i64">,
          Mustard::Data::Field<double, double, "f64">,
          Mustard::Data::Field<std::tuple<bool, char, int>, std::tuple<bool, char, std::int32_t>, "tbchi32">> {};

auto MakeEntryA(int eventID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelA>;
auto MakeEntryB(int eventID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelB>;
auto MakeEntryC(int eventID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelC>;

} // namespace Mustard::Testing
