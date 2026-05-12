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

#include "Mustard/CLHEPX/Random/MersenneTwister.h++"
#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/MT1993732.h++"
#include "Mustard/Testing/TestCLHEPXMT1993732/TestCLHEPXMT1993732.h++"

#include "Eigen/Core"

#include "muc/chrono"

#include <numeric>
#include <random>

namespace Mustard::Testing {

TestCLHEPXMT1993732::TestCLHEPXMT1993732() :
    Subprogram{"TestCLHEPXMT1993732", "Test Mustard::CLHEPX::Random::MT1993732 and benchmark."} {}

auto TestCLHEPXMT1993732::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    Random::MT1993732 mt32{0x123456};
    CLHEPX::Random::MT1993732 mt32x{0x123456};

    PrintLn("Simply generate 10 million integers:");

    unsigned int r;
    for (int i = 0; i < 1000; ++i) {
        r = mt32();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i = 0; i < 10'000'000; ++i) {
        r = mt32();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    PrintLn("    CLHEPX::...::MT1993732Engine : {} ms (last integer: {})", time, r);

    for (int i = 0; i < 1000; ++i) {
        r = (unsigned int)(mt32x);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        r = (unsigned int)(mt32x);
    }
    time = stopwatch.read();
    PrintLn("       Math::...::MT1993732 : {} ms (last integer: {})", time, r);

    PrintLn("2D random walk, 10 million steps:");
    Eigen::RowVector2d v2d = {0, 0};
    Eigen::RowVector2d delta2d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("    CLHEPX::...::MT1993732Engine : {} ms (last displacement: {})", time, v2d);

    v2d = {0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {mt32x.flat(),
                   mt32x.flat()};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {mt32x.flat(),
                   mt32x.flat()};
        v2d += delta2d;
    }
    time = stopwatch.read();
    PrintLn("       Math::...::MT1993732 : {} ms (last displacement: {})", time, v2d);

    PrintLn("3D random walk, 10 million steps:");
    Eigen::RowVector3d v3d = {0, 0, 0};
    Eigen::RowVector3d delta3d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("    CLHEPX::...::MT1993732Engine : {} ms (last displacement: {})", time, v3d);

    v3d = {0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {mt32x.flat(),
                   mt32x.flat(),
                   mt32x.flat()};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {mt32x.flat(),
                   mt32x.flat(),
                   mt32x.flat()};
        v3d += delta3d;
    }
    time = stopwatch.read();
    PrintLn("       Math::...::MT1993732 : {} ms (last displacement: {})", time, v3d);

    PrintLn("4D random walk, 10 million steps:");
    Eigen::RowVector4d v4d = {0, 0, 0, 0};
    Eigen::RowVector4d delta4d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32),
                   Random::Uniform<double>()(mt32)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("    CLHEPX::...::MT1993732Engine : {} ms (last displacement: {})", time, v4d);

    v4d = {0, 0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {mt32x.flat(),
                   mt32x.flat(),
                   mt32x.flat(),
                   mt32x.flat()};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {mt32x.flat(),
                   mt32x.flat(),
                   mt32x.flat(),
                   mt32x.flat()};
        v4d += delta4d;
    }
    time = stopwatch.read();
    PrintLn("       Math::...::MT1993732 : {} ms (last displacement: {})", time, v4d);

    return 0;
}

} // namespace Mustard::Testing
