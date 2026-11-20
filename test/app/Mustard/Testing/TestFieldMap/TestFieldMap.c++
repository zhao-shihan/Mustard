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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Detector/Field/ElectricFieldMap.h++"
#include "Mustard/Detector/Field/ElectromagneticFieldMap.h++"
#include "Mustard/Detector/Field/MagneticFieldMap.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/CreateTemporaryFile.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Testing/TestFieldMap/TestFieldMap.h++"

#include "CLHEP/Units/SystemOfUnits.h"

#include "ROOT/RNTupleModel.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include "TFile.h"

#include "Eigen/Core"

#include "muc/array"
#include "muc/ceta_string"
#include "muc/chrono"
#include "muc/numeric"

#include "fmt/format.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace Mustard::Testing {

TestFieldMap::TestFieldMap() :
    Subprogram{"TestFieldMap", "Test Mustard::Detector::Field field maps."} {}

auto TestFieldMap::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace Mustard::Detector::Field;

    constexpr auto tesla{CLHEP::tesla};
    constexpr auto voltPerM{CLHEP::volt / CLHEP::m};

    // 5x5x5 grid from -2 to 2, step 1.0
    constexpr std::array gridValues{-2.0, -1.0, 0.0, 1.0, 2.0};

    // =========================================================================
    // Helper lambdas
    // =========================================================================

    // Check if two 3D vectors are component-wise close.
    const auto isClose3D{[](const auto& a, const auto& b) -> bool {
        return muc::isclose(a[0], b[0]) and muc::isclose(a[1], b[1]) and muc::isclose(a[2], b[2]);
    }};

    // Throw with diagnostic if 3D vectors are not close.
    const auto throwIfNotClose3D{[&](const auto& actual, const auto& expected,
                                     const std::string& context) {
        if (not isClose3D(actual, expected)) {
            Throw<std::runtime_error>(fmt::format(
                "{}: expected ({}, {}, {}), got ({}, {}, {})",
                context, expected[0], expected[1], expected[2], actual[0], actual[1], actual[2]));
        }
    }};

    // Throw with diagnostic if BE result is not close.
    const auto throwIfNotCloseBE{[&](const auto& be, const auto& expectedB,
                                     const auto& expectedE, const std::string& context) {
        if (not isClose3D(be.B, expectedB) or not isClose3D(be.E, expectedE)) {
            Throw<std::runtime_error>(fmt::format(
                "{}: expected B({}, {}, {}), E({}, {}, {}), got B({}, {}, {}), E({}, {}, {})",
                context,
                expectedB[0], expectedB[1], expectedB[2], expectedE[0], expectedE[1], expectedE[2],
                be.B[0], be.B[1], be.B[2], be.E[0], be.E[1], be.E[2]));
        }
    }};

    // Formula for 3D field raw values (Set A).
    // f(x,y,z) = (x+2, y+3, z+4)
    const auto formula3D{[](double x, double y, double z) -> Point3D {
        return {x + 2, y + 3, z + 4};
    }};

    // Formula for E-part of 6D field raw values (Set B).
    // f_E(x,y,z) = (2x+5, 2y+6, 2z+7)
    const auto formulaE{[](double x, double y, double z) -> Point3D {
        return {2 * x + 5, 2 * y + 6, 2 * z + 7};
    }};

    // Write a 3D vector field RNTuple (Set A: grid + 3-component field).
    const auto writeSetA{[&](const std::filesystem::path& path) {
        Mustard::File<TFile> file{path, "RECREATE"};
        auto model{ROOT::RNTupleModel::Create()};
        const auto gridField{model->MakeField<muc::array3f>("grid")};
        const auto fieldField{model->MakeField<muc::array3f>("field")};
        auto writer{ROOT::RNTupleWriter::Append(std::move(model), "field", *file)};
        for (auto x : gridValues) {
            for (auto y : gridValues) {
                for (auto z : gridValues) {
                    *gridField = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
                    const auto f{formula3D(x, y, z)};
                    *fieldField = {static_cast<float>(f[0]), static_cast<float>(f[1]), static_cast<float>(f[2])};
                    writer->Fill();
                }
            }
        }
    }};

    // Write a 6D vector field RNTuple (Set B: grid + 6-component field).
    // Layout: [Bx, By, Bz, Ex, Ey, Ez]
    const auto writeSetB{[&](const std::filesystem::path& path) {
        Mustard::File<TFile> file{path, "RECREATE"};
        auto model{ROOT::RNTupleModel::Create()};
        const auto gridField{model->MakeField<muc::array3f>("grid")};
        const auto fieldField{model->MakeField<muc::arrayf<6>>("field")};
        auto writer{ROOT::RNTupleWriter::Append(std::move(model), "field", *file)};
        for (auto x : gridValues) {
            for (auto y : gridValues) {
                for (auto z : gridValues) {
                    *gridField = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
                    const auto b{formula3D(x, y, z)};
                    const auto e{formulaE(x, y, z)};
                    *fieldField = {static_cast<float>(b[0]), static_cast<float>(b[1]), static_cast<float>(b[2]),
                                   static_cast<float>(e[0]), static_cast<float>(e[1]), static_cast<float>(e[2])};
                    writer->Fill();
                }
            }
        }
    }};

    // Test 3D field at all grid points.
    // queryField(x,y,z) -> Point3D
    // rawFormula(x,y,z) -> Point3D (raw SI value before unit conversion)
    const auto test3DGridPoints{[&](auto queryField, auto rawFormula, double unit,
                                    const std::string& name) {
        for (auto x : gridValues) {
            for (auto y : gridValues) {
                for (auto z : gridValues) {
                    const auto actual{queryField(x, y, z)};
                    const auto raw{rawFormula(x, y, z)};
                    Point3D expected{raw[0] * unit, raw[1] * unit, raw[2] * unit};
                    throwIfNotClose3D(actual, expected,
                                      fmt::format("{} at ({},{},{})", name, x, y, z));
                }
            }
        }
    }};

    // Test 3D field at a midpoint (trilinear interpolation).
    const auto test3DMidpoint{[&](auto queryField, auto rawFormula, double unit,
                                  const std::string& name) {
        constexpr double mx{-1.5};
        constexpr double my{-1.5};
        constexpr double mz{-1.5};
        const auto actual{queryField(mx, my, mz)};
        const auto raw{rawFormula(mx, my, mz)};
        Point3D expected{raw[0] * unit, raw[1] * unit, raw[2] * unit};
        throwIfNotClose3D(actual, expected,
                          fmt::format("{} at midpoint ({},{},{})", name, mx, my, mz));
    }};

    // Test 3D field at outside-grid point (should return zero).
    const auto test3DOutside{[&](auto queryField, const std::string& name) {
        constexpr double ox{3.0};
        constexpr double oy{3.0};
        constexpr double oz{3.0};
        const auto actual{queryField(ox, oy, oz)};
        Point3D expected{0, 0, 0};
        throwIfNotClose3D(actual, expected,
                          fmt::format("{} outside grid at ({},{},{})", name, ox, oy, oz));
    }};

    // Test 3D field symmetry by cross-verification.
    // querySym(x,y,z) -> sym field map result
    // queryNonSym(x,y,z) -> non-sym field map result
    // (x,y,z) is the negative test point, flipMask: bit 0=X, 1=Y, 2=Z
    const auto test3DSymmetry{[&](auto querySym, auto queryNonSym,
                                  double x, double y, double z, int flipMask,
                                  const std::string& name) {
        // Project coordinates to positive half-space
        double px{x};
        double py{y};
        double pz{z};
        if (flipMask & 1) {
            px = std::abs(px);
        }
        if (flipMask & 2) {
            py = std::abs(py);
        }
        if (flipMask & 4) {
            pz = std::abs(pz);
        }

        const auto symResult{querySym(x, y, z)};
        auto nonSymResult{queryNonSym(px, py, pz)};

        // Apply field symmetry flips to the non-symmetric result
        if (flipMask & 1) {
            nonSymResult[0] = -nonSymResult[0];
        }
        if (flipMask & 2) {
            nonSymResult[1] = -nonSymResult[1];
        }
        if (flipMask & 4) {
            nonSymResult[2] = -nonSymResult[2];
        }

        throwIfNotClose3D(symResult, nonSymResult,
                          fmt::format("{} at ({},{},{})", name, x, y, z));
    }};

    // =========================================================================
    // Create temporary ROOT files
    // =========================================================================

    const auto pathA{Mustard::IO::CreateTemporaryFile("mustard-test-field-map-setA", ".root")};
    const auto pathB{Mustard::IO::CreateTemporaryFile("mustard-test-field-map-setB", ".root")};

    writeSetA(pathA);
    writeSetB(pathB);

    // =========================================================================
    // Construct non-symmetric field maps
    // =========================================================================

    MagneticFieldMap<> bField{"field", pathA.string()};
    ElectricFieldMap<> eField{"field", pathA.string()};
    ElectromagneticFieldMap<> emField{"field", pathB.string()};

    // =========================================================================
    // Non-symmetric tests: grid points
    // =========================================================================

    // MagneticFieldMap: test B() at all grid points
    test3DGridPoints(
        [&](double x, double y, double z) { return bField.B({x, y, z}); },
        formula3D, tesla, "MagneticFieldMap B");

    // MagneticFieldMap: test E() always returns zero (from MagneticFieldBase)
    test3DGridPoints(
        [&](double x, double y, double z) { return bField.E({x, y, z}); },
        [](double, double, double) -> Point3D { return {0, 0, 0}; },
        1.0, "MagneticFieldMap E (should be zero)");

    // MagneticFieldMap: test BE() at select grid points
    {
        const auto raw{formula3D(-2, -2, -2)};
        const auto be{bField.BE({-2.0, -2.0, -2.0})};
        throwIfNotCloseBE(be,
                          Point3D{raw[0] * tesla, raw[1] * tesla, raw[2] * tesla},
                          Point3D{0, 0, 0},
                          "MagneticFieldMap BE at (-2,-2,-2)");
    }

    // ElectricFieldMap: test E() at all grid points
    test3DGridPoints(
        [&](double x, double y, double z) { return eField.E({x, y, z}); },
        formula3D, voltPerM, "ElectricFieldMap E");

    // ElectricFieldMap: test B() always returns zero (from ElectricFieldBase)
    test3DGridPoints(
        [&](double x, double y, double z) { return eField.B({x, y, z}); },
        [](double, double, double) -> Point3D { return {0, 0, 0}; },
        1.0, "ElectricFieldMap B (should be zero)");

    // ElectricFieldMap: test BE() at select grid points
    {
        const auto raw{formula3D(2, 2, 2)};
        const auto be{eField.BE({2.0, 2.0, 2.0})};
        throwIfNotCloseBE(be,
                          Point3D{0, 0, 0},
                          Point3D{raw[0] * voltPerM, raw[1] * voltPerM, raw[2] * voltPerM},
                          "ElectricFieldMap BE at (2,2,2)");
    }

    // ElectromagneticFieldMap: test B() at all grid points
    test3DGridPoints(
        [&](double x, double y, double z) { return emField.B({x, y, z}); },
        formula3D, tesla, "ElectromagneticFieldMap B");

    // ElectromagneticFieldMap: test E() at all grid points
    test3DGridPoints(
        [&](double x, double y, double z) { return emField.E({x, y, z}); },
        formulaE, voltPerM, "ElectromagneticFieldMap E");

    // ElectromagneticFieldMap: test BE() at all grid points
    for (auto x : gridValues) {
        for (auto y : gridValues) {
            for (auto z : gridValues) {
                const auto rawB{formula3D(x, y, z)};
                const auto rawE{formulaE(x, y, z)};
                const auto be{emField.BE({x, y, z})};
                throwIfNotCloseBE(be,
                                  Point3D{rawB[0] * tesla, rawB[1] * tesla, rawB[2] * tesla},
                                  Point3D{rawE[0] * voltPerM, rawE[1] * voltPerM, rawE[2] * voltPerM},
                                  fmt::format("ElectromagneticFieldMap BE at ({},{},{})", x, y, z));
            }
        }
    }

    // =========================================================================
    // Non-symmetric tests: midpoint (trilinear interpolation)
    // =========================================================================

    test3DMidpoint(
        [&](double x, double y, double z) { return bField.B({x, y, z}); },
        formula3D, tesla, "MagneticFieldMap B");

    test3DMidpoint(
        [&](double x, double y, double z) { return eField.E({x, y, z}); },
        formula3D, voltPerM, "ElectricFieldMap E");

    test3DMidpoint(
        [&](double x, double y, double z) { return emField.B({x, y, z}); },
        formula3D, tesla, "ElectromagneticFieldMap B");

    test3DMidpoint(
        [&](double x, double y, double z) { return emField.E({x, y, z}); },
        formulaE, voltPerM, "ElectromagneticFieldMap E");

    // EM field BE at midpoint
    {
        constexpr double mx{-1.5};
        constexpr double my{-1.5};
        constexpr double mz{-1.5};
        const auto rawB{formula3D(mx, my, mz)};
        const auto rawE{formulaE(mx, my, mz)};
        const auto be{emField.BE({mx, my, mz})};
        throwIfNotCloseBE(be,
                          Point3D{rawB[0] * tesla, rawB[1] * tesla, rawB[2] * tesla},
                          Point3D{rawE[0] * voltPerM, rawE[1] * voltPerM, rawE[2] * voltPerM},
                          fmt::format("ElectromagneticFieldMap BE at midpoint ({},{},{})", mx, my, mz));
    }

    // =========================================================================
    // Non-symmetric tests: outside grid (should return zero)
    // =========================================================================

    test3DOutside(
        [&](double x, double y, double z) { return bField.B({x, y, z}); },
        "MagneticFieldMap B");
    test3DOutside(
        [&](double x, double y, double z) { return eField.E({x, y, z}); },
        "ElectricFieldMap E");
    test3DOutside(
        [&](double x, double y, double z) { return emField.B({x, y, z}); },
        "ElectromagneticFieldMap B");
    test3DOutside(
        [&](double x, double y, double z) { return emField.E({x, y, z}); },
        "ElectromagneticFieldMap E");

    // EM field BE outside grid
    {
        constexpr double ox{3.0};
        constexpr double oy{3.0};
        constexpr double oz{3.0};
        const auto be{emField.BE({ox, oy, oz})};
        throwIfNotCloseBE(be,
                          Point3D{0, 0, 0},
                          Point3D{0, 0, 0},
                          fmt::format("ElectromagneticFieldMap BE outside grid at ({},{},{})", ox, oy, oz));
    }

    // =========================================================================
    // Symmetry tests
    // =========================================================================

    // Since each symmetric variant is a different type, we must instantiate
    // them separately. Template helpers parameterized on the axis string
    // eliminate the repeated boilerplate.

    const auto testMagneticSymmetry{[&]<muc::ceta_string AAxis>() {
        double x, y, z;
        int flipMask;
        if constexpr (AAxis == "X") {
            x = -1.0;
            y = 0.0;
            z = 0.0;
            flipMask = 1;
        } else if constexpr (AAxis == "Y") {
            x = 0.0;
            y = -1.0;
            z = 0.0;
            flipMask = 2;
        } else if constexpr (AAxis == "Z") {
            x = 0.0;
            y = 0.0;
            z = -1.0;
            flipMask = 4;
        } else if constexpr (AAxis == "XY") {
            x = -1.0;
            y = -1.0;
            z = 0.0;
            flipMask = 3;
        } else if constexpr (AAxis == "XZ") {
            x = -1.0;
            y = 0.0;
            z = -1.0;
            flipMask = 5;
        } else if constexpr (AAxis == "YZ") {
            x = 0.0;
            y = -1.0;
            z = -1.0;
            flipMask = 6;
        } else {
            x = -1.0;
            y = -1.0;
            z = -1.0;
            flipMask = 7;
        }
        MagneticFieldMapSymmetry<AAxis> sym{"field", pathA.string()};
        test3DSymmetry(
            [&](double x, double y, double z) { return sym.B({x, y, z}); },
            [&](double x, double y, double z) { return bField.B({x, y, z}); },
            x, y, z, flipMask, fmt::format("MagneticFieldMapSymmetry<\"{}\"> B", AAxis.sv()));
    }};

    const auto testElectricSymmetry{[&]<muc::ceta_string AAxis>() {
        double x, y, z;
        int flipMask;
        if constexpr (AAxis == "X") {
            x = -1.0;
            y = 0.0;
            z = 0.0;
            flipMask = 1;
        } else if constexpr (AAxis == "Y") {
            x = 0.0;
            y = -1.0;
            z = 0.0;
            flipMask = 2;
        } else if constexpr (AAxis == "Z") {
            x = 0.0;
            y = 0.0;
            z = -1.0;
            flipMask = 4;
        } else if constexpr (AAxis == "XY") {
            x = -1.0;
            y = -1.0;
            z = 0.0;
            flipMask = 3;
        } else if constexpr (AAxis == "XZ") {
            x = -1.0;
            y = 0.0;
            z = -1.0;
            flipMask = 5;
        } else if constexpr (AAxis == "YZ") {
            x = 0.0;
            y = -1.0;
            z = -1.0;
            flipMask = 6;
        } else {
            x = -1.0;
            y = -1.0;
            z = -1.0;
            flipMask = 7;
        }
        ElectricFieldMapSymmetry<AAxis> sym{"field", pathA.string()};
        test3DSymmetry(
            [&](double x, double y, double z) { return sym.E({x, y, z}); },
            [&](double x, double y, double z) { return eField.E({x, y, z}); },
            x, y, z, flipMask, fmt::format("ElectricFieldMapSymmetry<\"{}\"> E", AAxis.sv()));
    }};

    const auto testElectromagneticSymmetry{[&]<muc::ceta_string AAxis>() {
        double x, y, z;
        int flipMask;
        if constexpr (AAxis == "X") {
            x = -1.0;
            y = 0.0;
            z = 0.0;
            flipMask = 1;
        } else if constexpr (AAxis == "Y") {
            x = 0.0;
            y = -1.0;
            z = 0.0;
            flipMask = 2;
        } else if constexpr (AAxis == "Z") {
            x = 0.0;
            y = 0.0;
            z = -1.0;
            flipMask = 4;
        } else if constexpr (AAxis == "XY") {
            x = -1.0;
            y = -1.0;
            z = 0.0;
            flipMask = 3;
        } else if constexpr (AAxis == "XZ") {
            x = -1.0;
            y = 0.0;
            z = -1.0;
            flipMask = 5;
        } else if constexpr (AAxis == "YZ") {
            x = 0.0;
            y = -1.0;
            z = -1.0;
            flipMask = 6;
        } else {
            x = -1.0;
            y = -1.0;
            z = -1.0;
            flipMask = 7;
        }
        ElectromagneticFieldMapSymmetry<AAxis> sym{"field", pathB.string()};
        test3DSymmetry(
            [&](double x, double y, double z) { return sym.B({x, y, z}); },
            [&](double x, double y, double z) { return emField.B({x, y, z}); },
            x, y, z, flipMask, fmt::format("ElectromagneticFieldMapSymmetry<\"{}\"> B", AAxis.sv()));
        test3DSymmetry(
            [&](double x, double y, double z) { return sym.E({x, y, z}); },
            [&](double x, double y, double z) { return emField.E({x, y, z}); },
            x, y, z, flipMask, fmt::format("ElectromagneticFieldMapSymmetry<\"{}\"> E", AAxis.sv()));
        // Also verify BE() at the grid boundary point
        const double sx = (flipMask & 1) ? -2.0 : 0.0;
        const double sy = (flipMask & 2) ? -2.0 : 0.0;
        const double sz = (flipMask & 4) ? -2.0 : 0.0;
        const auto be{sym.BE({sx, sy, sz})};
        const auto nonSymBE{emField.BE({std::abs(sx), std::abs(sy), std::abs(sz)})};
        Point3D expectedB{nonSymBE.B[0], nonSymBE.B[1], nonSymBE.B[2]};
        Point3D expectedE{nonSymBE.E[0], nonSymBE.E[1], nonSymBE.E[2]};
        if (flipMask & 1) {
            expectedB[0] = -expectedB[0];
            expectedE[0] = -expectedE[0];
        }
        if (flipMask & 2) {
            expectedB[1] = -expectedB[1];
            expectedE[1] = -expectedE[1];
        }
        if (flipMask & 4) {
            expectedB[2] = -expectedB[2];
            expectedE[2] = -expectedE[2];
        }
        throwIfNotCloseBE(be, expectedB, expectedE,
                          fmt::format("ElectromagneticFieldMapSymmetry<\"{}\"> BE at ({},{},{})", AAxis.sv(), sx, sy, sz));
    }};

    // --- MagneticFieldMap symmetry ---
    testMagneticSymmetry.template operator()<"X">();
    testMagneticSymmetry.template operator()<"Y">();
    testMagneticSymmetry.template operator()<"Z">();
    testMagneticSymmetry.template operator()<"XY">();
    testMagneticSymmetry.template operator()<"XZ">();
    testMagneticSymmetry.template operator()<"YZ">();
    testMagneticSymmetry.template operator()<"XYZ">();

    // --- ElectricFieldMap symmetry ---
    testElectricSymmetry.template operator()<"X">();
    testElectricSymmetry.template operator()<"Y">();
    testElectricSymmetry.template operator()<"Z">();
    testElectricSymmetry.template operator()<"XY">();
    testElectricSymmetry.template operator()<"XZ">();
    testElectricSymmetry.template operator()<"YZ">();
    testElectricSymmetry.template operator()<"XYZ">();

    // --- ElectromagneticFieldMap symmetry ---
    testElectromagneticSymmetry.template operator()<"X">();
    testElectromagneticSymmetry.template operator()<"Y">();
    testElectromagneticSymmetry.template operator()<"Z">();
    testElectromagneticSymmetry.template operator()<"XY">();
    testElectromagneticSymmetry.template operator()<"XZ">();
    testElectromagneticSymmetry.template operator()<"YZ">();
    testElectromagneticSymmetry.template operator()<"XYZ">();

    // =========================================================================
    // Timing
    // =========================================================================

    constexpr auto testCount{1'000'000};

    // --- MagneticFieldMap B() ---
    {
        PrintLn("  Timing: MagneticFieldMap B() ({} loops):", testCount);
        Point3D pos{0.5, 0.5, 0.5};
        volatile double dummy[3]{};
        const auto compute{[&]() {
            auto r{bField.B(pos)};
            dummy[0] += r[0];
            dummy[1] += r[1];
            dummy[2] += r[2];
            pos[0] += 1e-9;
        }};
        for (int i{}; i < testCount / 100; ++i) { compute(); }
        muc::chrono::stopwatch stopwatch;
        for (int i{}; i < testCount; ++i) { compute(); }
        muc::chrono::milliseconds<double> time{stopwatch.read()};
        PrintLn("    {} calls: {} (avg: {}/call)", testCount, time,
                muc::chrono::nanoseconds<double>{time / testCount});
    }

    // --- ElectricFieldMap E() ---
    {
        PrintLn("  Timing: ElectricFieldMap E() ({} loops):", testCount);
        Point3D pos{0.5, 0.5, 0.5};
        volatile double dummy[3]{};
        const auto compute{[&]() {
            auto r{eField.E(pos)};
            dummy[0] += r[0];
            dummy[1] += r[1];
            dummy[2] += r[2];
            pos[0] += 1e-9;
        }};
        for (int i{}; i < testCount / 100; ++i) { compute(); }
        muc::chrono::stopwatch stopwatch;
        for (int i{}; i < testCount; ++i) { compute(); }
        muc::chrono::milliseconds<double> time{stopwatch.read()};
        PrintLn("    {} calls: {} (avg: {}/call)", testCount, time,
                muc::chrono::nanoseconds<double>{time / testCount});
    }

    // --- ElectromagneticFieldMap B() ---
    {
        PrintLn("  Timing: ElectromagneticFieldMap B() ({} loops):", testCount);
        Point3D pos{0.5, 0.5, 0.5};
        volatile double dummy[3]{};
        const auto compute{[&]() {
            auto r{emField.B(pos)};
            dummy[0] += r[0];
            dummy[1] += r[1];
            dummy[2] += r[2];
            pos[0] += 1e-9;
        }};
        for (int i{}; i < testCount / 100; ++i) { compute(); }
        muc::chrono::stopwatch stopwatch;
        for (int i{}; i < testCount; ++i) { compute(); }
        muc::chrono::milliseconds<double> time{stopwatch.read()};
        PrintLn("    {} calls: {} (avg: {}/call)", testCount, time,
                muc::chrono::nanoseconds<double>{time / testCount});
    }

    // --- ElectromagneticFieldMap E() ---
    {
        PrintLn("  Timing: ElectromagneticFieldMap E() ({} loops):", testCount);
        Point3D pos{0.5, 0.5, 0.5};
        volatile double dummy[3]{};
        const auto compute{[&]() {
            auto r{emField.E(pos)};
            dummy[0] += r[0];
            dummy[1] += r[1];
            dummy[2] += r[2];
            pos[0] += 1e-9;
        }};
        for (int i{}; i < testCount / 100; ++i) { compute(); }
        muc::chrono::stopwatch stopwatch;
        for (int i{}; i < testCount; ++i) { compute(); }
        muc::chrono::milliseconds<double> time{stopwatch.read()};
        PrintLn("    {} calls: {} (avg: {}/call)", testCount, time,
                muc::chrono::nanoseconds<double>{time / testCount});
    }

    // --- ElectromagneticFieldMap BE() ---
    {
        PrintLn("  Timing: ElectromagneticFieldMap BE() ({} loops):", testCount);
        Point3D pos{0.5, 0.5, 0.5};
        volatile double dummyB[3]{};
        volatile double dummyE[3]{};
        const auto compute{[&]() {
            auto r{emField.BE(pos)};
            dummyB[0] += r.B[0];
            dummyB[1] += r.B[1];
            dummyB[2] += r.B[2];
            dummyE[0] += r.E[0];
            dummyE[1] += r.E[1];
            dummyE[2] += r.E[2];
            pos[0] += 1e-9;
        }};
        for (int i{}; i < testCount / 100; ++i) { compute(); }
        muc::chrono::stopwatch stopwatch;
        for (int i{}; i < testCount; ++i) { compute(); }
        muc::chrono::milliseconds<double> time{stopwatch.read()};
        PrintLn("    {} calls: {} (avg: {}/call)", testCount, time,
                muc::chrono::nanoseconds<double>{time / testCount});
    }

    // =========================================================================
    // All tests passed
    // =========================================================================

    PrintLn("All TestFieldMap tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
