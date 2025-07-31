#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/MT1993732.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PP.h++"

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
    Math::Random::MT1993732 mt1993732;
    Math::Random::Xoshiro256PP xoshiro256PP;

    std::cout << "Simply generate 10 million integers:" << std::endl;

    auto r = mt1993732();
    for (int i = 0; i < 1000; ++i) {
        r = mt1993732();
    }
    muc::chrono::stopwatch stopwatch;
    for (int i = 0; i < 10'000'000; ++i) {
        r = mt1993732();
    }
    muc::chrono::milliseconds<double> time{stopwatch.read()};
    std::cout << "      MT19937-32 : " << time << " ms (last integer: " << r << ')' << std::endl;

    for (int i = 0; i < 1000; ++i) {
        r = xoshiro256PP();
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        r = xoshiro256PP();
    }
    time = stopwatch.read();
    std::cout << "    xoshiro256++ : " << time << " ms (last integer: " << r << ')' << std::endl;

    std::cout << "Shuffle a std::array<double, 16> 1 million times:" << std::endl;
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
    std::cout << "      MT19937-32 : " << time << " ms (first element: " << arr16.front() << ')' << std::endl;

    for (int i = 0; i < 1000; ++i) {
        std::ranges::shuffle(arr16, xoshiro256PP);
    }
    stopwatch = {};
    for (int i = 0; i < 1'000'000; ++i) {
        std::ranges::shuffle(arr16, xoshiro256PP);
    }
    time = stopwatch.read();
    std::cout << "    xoshiro256++ : " << time << " ms (first element: " << arr16.front() << ')' << std::endl;

    std::cout << "Shuffle a std::array<double, 4096> 10k times:" << std::endl;
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
    std::cout << "      MT19937-32 : " << time << " ms (first element: " << arr4096.front() << ')' << std::endl;

    for (int i = 0; i < 100; ++i) {
        std::ranges::shuffle(arr4096, xoshiro256PP);
    }
    stopwatch = {};
    for (int i = 0; i < 10'000; ++i) {
        std::ranges::shuffle(arr4096, xoshiro256PP);
    }
    time = stopwatch.read();
    std::cout << "    xoshiro256++ : " << time << " ms (first element: " << arr4096.front() << ')' << std::endl;

    std::cout << "2D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector2d v2d = {0, 0};
    Eigen::RowVector2d delta2d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    std::cout << "      MT19937-32 : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

    v2d = {0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP)};
        v2d += delta2d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP)};
        v2d += delta2d;
    }
    time = stopwatch.read();
    std::cout << "    xoshiro256++ : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

    std::cout << "3D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector3d v3d = {0, 0, 0};
    Eigen::RowVector3d delta3d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    std::cout << "      MT19937-32 : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

    v3d = {0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP)};
        v3d += delta3d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP)};
        v3d += delta3d;
    }
    time = stopwatch.read();
    std::cout << "    xoshiro256++ : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

    std::cout << "4D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector4d v4d = {0, 0, 0, 0};
    Eigen::RowVector4d delta4d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732),
                   Math::Random::Uniform<double>()(mt1993732)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    std::cout << "      MT19937-32 : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

    v4d = {0, 0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP)};
        v4d += delta4d;
    }
    stopwatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP),
                   Math::Random::Uniform<double>()(xoshiro256PP)};
        v4d += delta4d;
    }
    time = stopwatch.read();
    std::cout << "    xoshiro256++ : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

    return 0;
}
