// Copyright (C) 2020-2025  The Mustard development team
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
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"

#include "Eigen/Core"

#include "muc/chrono"

#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>

using namespace Mustard;

int main() {
    Math::Random::MT1993764 mt64{0x123456};
    Mustard::CLHEPX::Random::MT1993764 mt64x{0x123456};

    std::cout << "Simply generate 10 million integers:" << std::endl;

    unsigned int r;
    for (int i = 0; i < 1000; ++i) {
        r = mt64();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i = 0; i < 10'000'000; ++i) {
        r = mt64();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    std::cout << "    CLHEPX::...::MT1993764Engine : " << time << " ms (last integer: " << r << ')' << std::endl;

    for (int i = 0; i < 1000; ++i) {
        r = (unsigned int)(mt64x);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        r = (unsigned int)(mt64x);
    }
    time = stopwatch.read();
    std::cout << "       Math::...::MT1993764 : " << time << " ms (last integer: " << r << ')' << std::endl;

    std::cout << "2D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector2d v2d = {0, 0};
    Eigen::RowVector2d delta2d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    std::cout << "    CLHEPX::...::MT1993764Engine : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

    v2d = {0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {mt64x.flat(),
                   mt64x.flat()};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {mt64x.flat(),
                   mt64x.flat()};
        v2d += delta2d;
    }
    time = stopwatch.read();
    std::cout << "       Math::...::MT1993764 : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

    std::cout << "3D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector3d v3d = {0, 0, 0};
    Eigen::RowVector3d delta3d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    std::cout << "    CLHEPX::...::MT1993764Engine : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

    v3d = {0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {mt64x.flat(),
                   mt64x.flat(),
                   mt64x.flat()};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {mt64x.flat(),
                   mt64x.flat(),
                   mt64x.flat()};
        v3d += delta3d;
    }
    time = stopwatch.read();
    std::cout << "       Math::...::MT1993764 : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

    std::cout << "4D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector4d v4d = {0, 0, 0, 0};
    Eigen::RowVector4d delta4d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64),
                   Math::Random::Uniform<double>()(mt64)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    std::cout << "    CLHEPX::...::MT1993764Engine : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

    v4d = {0, 0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {mt64x.flat(),
                   mt64x.flat(),
                   mt64x.flat(),
                   mt64x.flat()};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {mt64x.flat(),
                   mt64x.flat(),
                   mt64x.flat(),
                   mt64x.flat()};
        v4d += delta4d;
    }
    time = stopwatch.read();
    std::cout << "       Math::...::MT1993764 : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

    return 0;
}
