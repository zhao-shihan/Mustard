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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/MT1993732.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PlusPlus.h++"
#include "Mustard/Testing/TestXoshiro256PlusPlus/TestXoshiro256PlusPlus.h++"

#include "Eigen/Core"

#include "muc/chrono"
#include "muc/numeric"

#include <algorithm>
#include <array>
#include <numeric>
#include <random>

namespace Mustard::Testing {

TestXoshiro256PlusPlus::TestXoshiro256PlusPlus() :
    Subprogram{"TestXoshiro256PlusPlus", "Test Mustard::Random::Xoshiro256PlusPlus and benchmark."} {}

auto TestXoshiro256PlusPlus::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    Random::MT1993732 mt1993732;
    Random::Xoshiro256PlusPlus xoshiro256PP;

    PrintLn("Simply generate 10 million integers:");

    auto r = mt1993732();
    for (int i = 0; i < 1000; ++i) {
        r = mt1993732();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i = 0; i < 10'000'000; ++i) {
        r = mt1993732();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    PrintLn("      MT19937-32 : {} ms (last integer: {})", time, r);

    for (int i = 0; i < 1000; ++i) {
        r = xoshiro256PP();
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        r = xoshiro256PP();
    }
    time = stopwatch.read();
    PrintLn("    xoshiro256++ : {} ms (last integer: {})", time, r);

    PrintLn("Shuffle a std::array<double, 16> 1 million times:");
    std::array<double, 16> arr16;
    muc::ranges::iota(arr16, 0);

    for (int i = 0; i < 1000; ++i) {
        std::ranges::shuffle(arr16, mt1993732);
    }
    stopwatch = {};
    for (int i = 0; i < 1'000'000; ++i) {
        std::ranges::shuffle(arr16, mt1993732);
    }
    time = stopwatch.read();
    PrintLn("      MT19937-32 : {} ms (first element: {})", time, arr16.front());

    for (int i = 0; i < 1000; ++i) {
        std::ranges::shuffle(arr16, xoshiro256PP);
    }
    stopwatch = {};
    for (int i = 0; i < 1'000'000; ++i) {
        std::ranges::shuffle(arr16, xoshiro256PP);
    }
    time = stopwatch.read();
    PrintLn("    xoshiro256++ : {} ms (first element: {})", time, arr16.front());

    PrintLn("Shuffle a std::array<double, 4096> 10k times:");
    std::array<double, 4096> arr4096;
    muc::ranges::iota(arr4096, 0);

    for (int i = 0; i < 100; ++i) {
        std::ranges::shuffle(arr4096, mt1993732);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000; ++i) {
        std::ranges::shuffle(arr4096, mt1993732);
    }
    time = stopwatch.read();
    PrintLn("      MT19937-32 : {} ms (first element: {})", time, arr4096.front());

    for (int i = 0; i < 100; ++i) {
        std::ranges::shuffle(arr4096, xoshiro256PP);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000; ++i) {
        std::ranges::shuffle(arr4096, xoshiro256PP);
    }
    time = stopwatch.read();
    PrintLn("    xoshiro256++ : {} ms (first element: {})", time, arr4096.front());

    PrintLn("2D random walk, 10 million steps:");
    Eigen::RowVector2d v2d = {0, 0};
    Eigen::RowVector2d delta2d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("      MT19937-32 : {} ms (last displacement: {})", time, v2d);

    v2d = {0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("    xoshiro256++ : {} ms (last displacement: {})", time, v2d);

    PrintLn("3D random walk, 10 million steps:");
    Eigen::RowVector3d v3d = {0, 0, 0};
    Eigen::RowVector3d delta3d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("      MT19937-32 : {} ms (last displacement: {})", time, v3d);

    v3d = {0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("    xoshiro256++ : {} ms (last displacement: {})", time, v3d);

    PrintLn("4D random walk, 10 million steps:");
    Eigen::RowVector4d v4d = {0, 0, 0, 0};
    Eigen::RowVector4d delta4d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732),
                   Random::Uniform<double>()(mt1993732)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("      MT19937-32 : {} ms (last displacement: {})", time, v4d);

    v4d = {0, 0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP),
                   Random::Uniform<double>()(xoshiro256PP)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("    xoshiro256++ : {} ms (last displacement: {})", time, v4d);

    return 0;
}

} // namespace Mustard::Testing
