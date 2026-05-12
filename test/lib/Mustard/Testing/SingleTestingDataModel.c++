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
#include "Mustard/Testing/SingleTestingDataModel.h++"

#include "fmt/format.h"

namespace Mustard::Testing {

auto MakeEntry(gsl::index evtID, gsl::index entryIdx) -> Mustard::Data::ArcTuple<TestingModel> {
    auto event{Mustard::Data::MakeArcTuple<TestingModel>()};
    F<"EvtID">(*event) = static_cast<std::int32_t>(evtID);
    const auto i{evtID * 10 + entryIdx};
    F<"b">(*event) = (i % 2 == 0);
    F<"ch">(*event) = 'A' + i;
    F<"u8">(*event) = 2 + i;
    F<"i16">(*event) = -100 - i;
    F<"u16">(*event) = 100 + i;
    F<"i32">(*event) = -1000 - i;
    F<"u32">(*event) = 1000 + i;
    F<"i64">(*event) = -10000 - i;
    F<"u64">(*event) = 10000 + i;
    F<"f32">(*event) = 1.25f + i;
    F<"f64">(*event) = 2.5 + i;
    F<"str">(*event) = fmt::format("row-{}", i % 16);
    F<"vi32">(*event) = {std::int32_t(i), std::int32_t(i + 1), std::int32_t(i + 2)};
    F<"vf64">(*event) = {0.5 + i, 1.5 + i};
    F<"vstr">(*event) = {fmt::format("x{}", i % 16), fmt::format("y{}", i % 16)};
    F<"au16">(*event) = {std::uint16_t(10 + i), std::uint16_t(20 + i), std::uint16_t(30 + i)};
    F<"pi32f">(*event) = {std::int32_t(7 + i), 3.0f + i};
    F<"tu8str">(*event) = {std::uint8_t(9 + i), 6.25 + i, fmt::format("t{}", i)};
    return event;
}

} // namespace Mustard::Testing
