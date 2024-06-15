#include "Mustard/Extension/CLHEPX/Random/Xoshiro.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256SS.h++"

#include "Eigen/Core"

#include "muc/time"

#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>

using namespace Mustard;

int main() {
    Math::Random::Xoshiro256SS xoshiro256SS(0x123456);
    CLHEPX::Random::Wrap<Math::Random::Xoshiro256SS> xoshiro256SSX(0x123456);

    std::cout << "Simply generate 10 million integers:" << std::endl;

    unsigned int r;
    for (int i = 0; i < 1000; ++i) { r = xoshiro256SS(); }
    muc::wall_time_stopwatch<> stopWatch;
    for (int i = 0; i < 10'000'000; ++i) { r = xoshiro256SS(); }
    auto time = stopWatch.ms_elapsed();
    std::cout << "    CLHEPX::...::Xoshiro256SSEngine : " << time << " ms (last integer: " << r << ')' << std::endl;

    for (int i = 0; i < 1000; ++i) { r = (unsigned int)(xoshiro256SSX); }
    stopWatch = {};
    for (int i = 0; i < 10'000'000; ++i) { r = (unsigned int)(xoshiro256SSX); }
    time = stopWatch.ms_elapsed();
    std::cout << "     Math::...::Xoshiro256SS : " << time << " ms (last integer: " << r << ')' << std::endl;

    std::cout << "2D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector2d v2d = {0, 0};
    Eigen::RowVector2d delta2d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS)};
        v2d += delta2d;
    }
    stopWatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS)};
        v2d += delta2d;
    }
    time = stopWatch.ms_elapsed();
    std::cout << "    CLHEPX::...::Xoshiro256SSEngine : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

    v2d = {0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta2d = {xoshiro256SSX.flat(),
                   xoshiro256SSX.flat()};
        v2d += delta2d;
    }
    stopWatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta2d = {xoshiro256SSX.flat(),
                   xoshiro256SSX.flat()};
        v2d += delta2d;
    }
    time = stopWatch.ms_elapsed();
    std::cout << "     Math::...::Xoshiro256SS : " << time << " ms (last displacement: " << std::setprecision(18) << v2d << std::setprecision(6) << ')' << std::endl;

    std::cout << "3D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector3d v3d = {0, 0, 0};
    Eigen::RowVector3d delta3d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS)};
        v3d += delta3d;
    }
    stopWatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS)};
        v3d += delta3d;
    }
    time = stopWatch.ms_elapsed();
    std::cout << "    CLHEPX::...::Xoshiro256SSEngine : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

    v3d = {0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta3d = {xoshiro256SSX.flat(),
                   xoshiro256SSX.flat(),
                   xoshiro256SSX.flat()};
        v3d += delta3d;
    }
    stopWatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta3d = {xoshiro256SSX.flat(),
                   xoshiro256SSX.flat(),
                   xoshiro256SSX.flat()};
        v3d += delta3d;
    }
    time = stopWatch.ms_elapsed();
    std::cout << "     Math::...::Xoshiro256SS : " << time << " ms (last displacement: " << std::setprecision(18) << v3d << std::setprecision(6) << ')' << std::endl;

    std::cout << "4D random walk, 10 million steps:" << std::endl;
    Eigen::RowVector4d v4d = {0, 0, 0, 0};
    Eigen::RowVector4d delta4d;

    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS)};
        v4d += delta4d;
    }
    stopWatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS),
                   Math::Random::Uniform<double>()(xoshiro256SS)};
        v4d += delta4d;
    }
    time = stopWatch.ms_elapsed();
    std::cout << "    CLHEPX::...::Xoshiro256SSEngine : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

    v4d = {0, 0, 0, 0};
    for (int i = 0; i < 1'000'000; ++i) {
        delta4d = {xoshiro256SSX.flat(),
                   xoshiro256SSX.flat(),
                   xoshiro256SSX.flat(),
                   xoshiro256SSX.flat()};
        v4d += delta4d;
    }
    stopWatch = {};
    for (int i = 0; i < 10'000'000; ++i) {
        delta4d = {xoshiro256SSX.flat(),
                   xoshiro256SSX.flat(),
                   xoshiro256SSX.flat(),
                   xoshiro256SSX.flat()};
        v4d += delta4d;
    }
    time = stopWatch.ms_elapsed();
    std::cout << "     Math::...::Xoshiro256SS : " << time << " ms (last displacement: " << std::setprecision(18) << v4d << std::setprecision(6) << ')' << std::endl;

    return 0;
}
