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

#define MUSTARD_TESTING_TEST_STATISTICAL_TEST(Test)                \
    class TestStatistical##Test : public Application::Subprogram { \
    public:                                                        \
        TestStatistical##Test();                                   \
        auto Main(int argc, char* argv[]) const -> int override;   \
    };

MUSTARD_TESTING_TEST_STATISTICAL_TEST(TTest)
MUSTARD_TESTING_TEST_STATISTICAL_TEST(ZTest)
MUSTARD_TESTING_TEST_STATISTICAL_TEST(TSquaredTest)
MUSTARD_TESTING_TEST_STATISTICAL_TEST(ChiSquaredTest)

#undef MUSTARD_TESTING_TEST_STATISTICAL_TEST

} // namespace Mustard::Testing
