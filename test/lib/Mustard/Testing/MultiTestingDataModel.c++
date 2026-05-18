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

#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Testing/MultiTestingDataModel.h++"

#include "fmt/format.h"

namespace Mustard::Testing {

auto MakeEntryA(int eventID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelA> {
    auto entry{Mustard::Data::MakeArcTuple<TestingModelA>()};
    F<"EvtID">(*entry) = eventID;
    const auto i{eventID * 10 + entryIdx};
    F<"i32">(*entry) = -1000 - i;
    F<"f64">(*entry) = 2.5 + i;
    F<"str">(*entry) = fmt::format("modelA-{}", i);
    F<"b">(*entry) = (i % 2 == 0);
    F<"ch">(*entry) = 'A' + (i % 26);
    F<"i64">(*entry) = -10000 - i;
    F<"u32">(*entry) = 1000 + i;
    F<"f32">(*entry) = 1.25f + i;
    F<"vi32">(*entry) = std::array{i, i + 1, i + 2};
    return entry;
}

auto MakeEntryB(int eventID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelB> {
    auto entry{Mustard::Data::MakeArcTuple<TestingModelB>()};
    F<"EvtID">(*entry) = eventID;
    const auto i{eventID * 10 + entryIdx};
    F<"f32">(*entry) = 0.5f + i;
    F<"b">(*entry) = (i % 3 == 0);
    F<"vi32">(*entry) = std::array{i, i * 2, i * 3};
    F<"vf64">(*entry) = {0.1 + i, 0.2 + i, 0.3 + i};
    F<"vstr">(*entry) = {fmt::format("b{}", i), fmt::format("c{}", i)};
    F<"u8">(*entry) = i % 256;
    F<"i16">(*entry) = -500 - i % 30000;
    F<"pf32f64">(*entry) = {1.0f + i, 2.0 + i};
    F<"tu16bstr">(*entry) = {(100 + i) % 65536, i % 4 == 0, fmt::format("tuple{}", i)};
    return entry;
}

auto MakeEntryC(int eventID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModelC> {
    auto entry{Mustard::Data::MakeArcTuple<TestingModelC>()};
    F<"EvtID">(*entry) = eventID;
    const auto i{eventID * 10 + entryIdx};
    F<"a3f64">(*entry) = {1.0 + i, 2.0 + i, 3.0 + i};
    F<"pi32i32">(*entry) = {i, -i};
    F<"a4u32">(*entry) = std::array{10 + i, 20 + i, 30 + i, 40 + i};
    F<"str">(*entry) = fmt::format("modelC-{}", i);
    F<"i64">(*entry) = 10000 + i;
    F<"f64">(*entry) = 3.14159 + i;
    F<"tbchi32">(*entry) = {(i % 2 == 0), 'Z' - (i % 26), static_cast<int>(i * 10)};
    return entry;
}

} // namespace Mustard::Testing
