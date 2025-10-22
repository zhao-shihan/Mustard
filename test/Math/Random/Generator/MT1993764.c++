// Copyright (C) 2020-2025  Mustard developers
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

#include "Mustard/Math/Random/Generator/MT1993764.h++"

#include "Eigen/Core"

#include "muc/chrono"
#include "muc/numeric"

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>

using namespace Mustard;

int main() {
    std::mt19937_64 stdMT1993764;
    Math::Random::MT1993764 mt1993764;

    std::cout << "Simply generate 10 million integers:" << std::endl;

    uintmax_t r;
    for (int i = 0; i < 1'000'000; ++i) {
        r = stdMT1993764();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i = 0; i < 10'000'000; ++i) {
        r = stdMT1993764();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    std::cout << "    std::mt19937_64 : " << time << " ms (last integer: " << r << ')' << std::endl;

    for (int i = 0; i < 1'000'000; ++i) {
        r = mt1993764();
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        r = mt1993764();
    }
    time = stopwatch.read();
    std::cout << "    Mustard MT19937-64 : " << time << " ms (last integer: " << r << ')' << std::endl;

    std::cout << "Shuffle a std::array<double, 16> 1 million times:" << std::endl;
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
    std::cout << "    std::mt19937_64 : " << time << " ms (first element: " << arr16.front() << ')' << std::endl;

    muc::ranges::iota(arr16, 0);
    for (int i = 0; i < 100'000; ++i) {
        std::ranges::shuffle(arr16, mt1993764);
    }
    stopwatch = {};
    for (int i = 0; i < 1'000'000; ++i) {
        std::ranges::shuffle(arr16, mt1993764);
    }
    time = stopwatch.read();
    std::cout << "    Mustard MT19937-64 : " << time << " ms (first element: " << arr16.front() << ')' << std::endl;

    std::cout << "Shuffle a std::array<double, 4096> 10k times:" << std::endl;
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
    std::cout << "    std::mt19937_64 : " << time << " ms (first element: " << arr4096.front() << ')' << std::endl;

    muc::ranges::iota(arr4096, 0);
    for (int i = 0; i < 1'000; ++i) {
        std::ranges::shuffle(arr4096, mt1993764);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000; ++i) {
        std::ranges::shuffle(arr4096, mt1993764);
    }
    time = stopwatch.read();
    std::cout << "    Mustard MT19937-64 : " << time << " ms (first element: " << arr4096.front() << ')' << std::endl;

    std::cout << "2D random walk, 10 million steps:" << std::endl;
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
    std::cout << "    std::mt19937_64 : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

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
    std::cout << "    Mustard MT19937-64 : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

    std::cout << "3D random walk, 10 million steps:" << std::endl;
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
    std::cout << "    std::mt19937_64 : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

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
    std::cout << "    Mustard MT19937-64 : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

    std::cout << "4D random walk, 10 million steps:" << std::endl;
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
    std::cout << "    std::mt19937_64 : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

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
    std::cout << "    Mustard MT19937-64 : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

    return 0;
}
