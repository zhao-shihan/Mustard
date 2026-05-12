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

#include "Mustard/CLHEPX/Random/Xoshiro.h++"
#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512StarStar.h++"
#include "Mustard/Testing/TestCLHEPXXoshiro512StarStar/TestCLHEPXXoshiro512StarStar.h++"

#include "Eigen/Core"

#include "muc/chrono"

#include <numeric>
#include <random>

namespace Mustard::Testing {

TestCLHEPXXoshiro512StarStar::TestCLHEPXXoshiro512StarStar() :
    Subprogram{"TestCLHEPXXoshiro512StarStar", "Test Mustard::CLHEPX::Random::Xoshiro512StarStar and benchmark."} {}

auto TestCLHEPXXoshiro512StarStar::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    Random::Xoshiro512StarStar xoshiro512SS(0x123456);
    CLHEPX::Random::Wrap<Random::Xoshiro512StarStar> xoshiro512SSX(0x123456);

    PrintLn("Simply generate 10 million integers:");

    unsigned int r;
    for (int i = 0; i < 1000; ++i) {
        r = xoshiro512SS();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i = 0; i < 10'000'000; ++i) {
        r = xoshiro512SS();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    PrintLn("    CLHEPX::...::Xoshiro512StarStarEngine : {} ms (last integer: {})", time, r);

    for (int i = 0; i < 1000; ++i) {
        r = (unsigned int)(xoshiro512SSX);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        r = (unsigned int)(xoshiro512SSX);
    }
    time = stopwatch.read();
    PrintLn("     Math::...::Xoshiro512StarStar : {} ms (last integer: {})", time, r);

    PrintLn("2D random walk, 10 million steps:");
    Eigen::RowVector2d v2d = {0, 0};
    Eigen::RowVector2d delta2d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("    CLHEPX::...::Xoshiro512StarStarEngine : {} ms (last displacement: {})", time, v2d);

    v2d = {0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {xoshiro512SSX.flat(),
                   xoshiro512SSX.flat()};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {xoshiro512SSX.flat(),
                   xoshiro512SSX.flat()};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("     Math::...::Xoshiro512StarStar : {} ms (last displacement: {})", time, v2d);

    PrintLn("3D random walk, 10 million steps:");
    Eigen::RowVector3d v3d = {0, 0, 0};
    Eigen::RowVector3d delta3d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("    CLHEPX::...::Xoshiro512StarStarEngine : {} ms (last displacement: {})", time, v3d);

    v3d = {0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {xoshiro512SSX.flat(),
                   xoshiro512SSX.flat(),
                   xoshiro512SSX.flat()};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {xoshiro512SSX.flat(),
                   xoshiro512SSX.flat(),
                   xoshiro512SSX.flat()};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("     Math::...::Xoshiro512StarStar : {} ms (last displacement: {})", time, v3d);

    PrintLn("4D random walk, 10 million steps:");
    Eigen::RowVector4d v4d = {0, 0, 0, 0};
    Eigen::RowVector4d delta4d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS),
                   Random::Uniform<double>()(xoshiro512SS)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("    CLHEPX::...::Xoshiro512StarStarEngine : {} ms (last displacement: {})", time, v4d);

    v4d = {0, 0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {xoshiro512SSX.flat(),
                   xoshiro512SSX.flat(),
                   xoshiro512SSX.flat(),
                   xoshiro512SSX.flat()};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {xoshiro512SSX.flat(),
                   xoshiro512SSX.flat(),
                   xoshiro512SSX.flat(),
                   xoshiro512SSX.flat()};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("     Math::...::Xoshiro512StarStar : {} ms (last displacement: {})", time, v4d);

    return 0;
}

} // namespace Mustard::Testing
