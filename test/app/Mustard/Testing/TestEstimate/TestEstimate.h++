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

#pragma once

#include "Mustard/Application/Subprogram.h++"

namespace Mustard::Testing {

#define MUSTARD_TESTING_TEST_ESTIMATE(N)                         \
    class TestEstimate##N : public Application::Subprogram {     \
    public:                                                      \
        TestEstimate##N();                                       \
        auto Main(int argc, char* argv[]) const -> int override; \
    };

MUSTARD_TESTING_TEST_ESTIMATE(0)
MUSTARD_TESTING_TEST_ESTIMATE(1)
MUSTARD_TESTING_TEST_ESTIMATE(2)
MUSTARD_TESTING_TEST_ESTIMATE(3)
MUSTARD_TESTING_TEST_ESTIMATE(4)
MUSTARD_TESTING_TEST_ESTIMATE(5)
MUSTARD_TESTING_TEST_ESTIMATE(6)
MUSTARD_TESTING_TEST_ESTIMATE(7)
MUSTARD_TESTING_TEST_ESTIMATE(8)
MUSTARD_TESTING_TEST_ESTIMATE(9)
MUSTARD_TESTING_TEST_ESTIMATE(10)
MUSTARD_TESTING_TEST_ESTIMATE(11)
MUSTARD_TESTING_TEST_ESTIMATE(12)
MUSTARD_TESTING_TEST_ESTIMATE(13)
MUSTARD_TESTING_TEST_ESTIMATE(14)
MUSTARD_TESTING_TEST_ESTIMATE(15)
MUSTARD_TESTING_TEST_ESTIMATE(16)
MUSTARD_TESTING_TEST_ESTIMATE(17)
MUSTARD_TESTING_TEST_ESTIMATE(18)
MUSTARD_TESTING_TEST_ESTIMATE(19)
MUSTARD_TESTING_TEST_ESTIMATE(20)
MUSTARD_TESTING_TEST_ESTIMATE(21)
MUSTARD_TESTING_TEST_ESTIMATE(22)
MUSTARD_TESTING_TEST_ESTIMATE(23)
MUSTARD_TESTING_TEST_ESTIMATE(24)
MUSTARD_TESTING_TEST_ESTIMATE(25)
MUSTARD_TESTING_TEST_ESTIMATE(26)
MUSTARD_TESTING_TEST_ESTIMATE(27)
MUSTARD_TESTING_TEST_ESTIMATE(28)
MUSTARD_TESTING_TEST_ESTIMATE(29)
MUSTARD_TESTING_TEST_ESTIMATE(30)
MUSTARD_TESTING_TEST_ESTIMATE(31)
MUSTARD_TESTING_TEST_ESTIMATE(32)
MUSTARD_TESTING_TEST_ESTIMATE(33)
MUSTARD_TESTING_TEST_ESTIMATE(34)
MUSTARD_TESTING_TEST_ESTIMATE(35)
MUSTARD_TESTING_TEST_ESTIMATE(36)
MUSTARD_TESTING_TEST_ESTIMATE(37)

#undef MUSTARD_TESTING_TEST_ESTIMATE

} // namespace Mustard::Testing
