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

#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"
#include "Mustard/Testing/TestMT1993764/TestMT1993764.h++"

#include "Eigen/Core"

#include "muc/chrono"
#include "muc/numeric"

#include <algorithm>
#include <array>
#include <numeric>
#include <random>

namespace Mustard::Testing {

TestMT1993764::TestMT1993764() :
    Subprogram{"TestMT1993764", "Test Mustard::Random::MT1993764 and benchmark."} {}

auto TestMT1993764::Main(int /*argc*/, char* /*argv*/[]) const -> int {
    std::mt19937_64 stdMT1993764;
    Random::MT1993764 mt1993764;

    PrintLn("Simply generate 10 million integers:");

    uintmax_t r;
    for (int i = 0; i < 1'000'000; ++i) {
        r = stdMT1993764();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i = 0; i < 10'000'000; ++i) {
        r = stdMT1993764();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    PrintLn("    std::mt19937_64 : {} ms (last integer: {})", time, r);

    for (int i = 0; i < 1'000'000; ++i) {
        r = mt1993764();
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        r = mt1993764();
    }
    time = stopwatch.read();
    PrintLn("    Mustard MT19937-64 : {} ms (last integer: {})", time, r);

    PrintLn("Shuffle a std::array<double, 16> 1 million times:");
    std::array<double, 16> arr16;

    muc::ranges::iota(arr16, 0);
    for (int i = 0; i < 100'000; ++i) {
        std::ranges::shuffle(arr16, stdMT1993764);
    }
    stopwatch = {};
    for (int i = 0; i < 1'000'000; ++i) {
        std::ranges::shuffle(arr16, stdMT1993764);
    }
    time = stopwatch.read();
    PrintLn("    std::mt19937_64 : {} ms (first element: {})", time, arr16.front());

    muc::ranges::iota(arr16, 0);
    for (int i = 0; i < 100'000; ++i) {
        std::ranges::shuffle(arr16, mt1993764);
    }
    stopwatch = {};
    for (int i = 0; i < 1'000'000; ++i) {
        std::ranges::shuffle(arr16, mt1993764);
    }
    time = stopwatch.read();
    PrintLn("    Mustard MT19937-64 : {} ms (first element: {})", time, arr16.front());

    PrintLn("Shuffle a std::array<double, 4096> 10k times:");
    std::array<double, 4096> arr4096;

    muc::ranges::iota(arr4096, 0);
    for (int i = 0; i < 1'000; ++i) {
        std::ranges::shuffle(arr4096, stdMT1993764);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000; ++i) {
        std::ranges::shuffle(arr4096, stdMT1993764);
    }
    time = stopwatch.read();
    PrintLn("    std::mt19937_64 : {} ms (first element: {})", time, arr4096.front());

    muc::ranges::iota(arr4096, 0);
    for (int i = 0; i < 1'000; ++i) {
        std::ranges::shuffle(arr4096, mt1993764);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000; ++i) {
        std::ranges::shuffle(arr4096, mt1993764);
    }
    time = stopwatch.read();
    PrintLn("    Mustard MT19937-64 : {} ms (first element: {})", time, arr4096.front());

    PrintLn("2D random walk, 10 million steps:");
    Eigen::RowVector2d v2d = {0, 0};
    Eigen::RowVector2d delta2d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("    std::mt19937_64 : {} ms (last displacement: {})", time, v2d);

    v2d = {0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("    Mustard MT19937-64 : {} ms (last displacement: {})", time, v2d);

    PrintLn("3D random walk, 10 million steps:");
    Eigen::RowVector3d v3d = {0, 0, 0};
    Eigen::RowVector3d delta3d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("    std::mt19937_64 : {} ms (last displacement: {})", time, v3d);

    v3d = {0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("    Mustard MT19937-64 : {} ms (last displacement: {})", time, v3d);

    PrintLn("4D random walk, 10 million steps:");
    Eigen::RowVector4d v4d = {0, 0, 0, 0};
    Eigen::RowVector4d delta4d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764),
                   std::uniform_real_distribution()(stdMT1993764)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("    std::mt19937_64 : {} ms (last displacement: {})", time, v4d);

    v4d = {0, 0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764),
                   std::uniform_real_distribution()(mt1993764)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("    Mustard MT19937-64 : {} ms (last displacement: {})", time, v4d);

    return 0;
}

} // namespace Mustard::Testing
