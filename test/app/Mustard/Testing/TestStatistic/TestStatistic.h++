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

#include "Mustard/Application/Subprogram.h++"

namespace Mustard::Testing {

#define MUSTARD_TESTING_TEST_STATISTIC(N)                        \
    class TestStatistic##N : public Application::Subprogram {    \
    public:                                                      \
        TestStatistic##N();                                      \
        auto Main(int argc, char* argv[]) const -> int override; \
    };

MUSTARD_TESTING_TEST_STATISTIC(0)
MUSTARD_TESTING_TEST_STATISTIC(1)
MUSTARD_TESTING_TEST_STATISTIC(2)
MUSTARD_TESTING_TEST_STATISTIC(3)
MUSTARD_TESTING_TEST_STATISTIC(4)
MUSTARD_TESTING_TEST_STATISTIC(5)
MUSTARD_TESTING_TEST_STATISTIC(6)
MUSTARD_TESTING_TEST_STATISTIC(7)
MUSTARD_TESTING_TEST_STATISTIC(8)
MUSTARD_TESTING_TEST_STATISTIC(9)
MUSTARD_TESTING_TEST_STATISTIC(10)

#undef MUSTARD_TESTING_TEST_STATISTIC

} // namespace Mustard::Testing
