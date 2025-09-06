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

#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Distribution/UniformRectangle.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"

#include "ROOT/RDataFrame.hxx"

#include "muc/array"

using namespace Mustard;

int main(int argc, char* argv[]) {
    Math::Random::MT1993764 mt1993764;

    const auto n = std::stod(argv[1]);
    const auto a1 = argc > 2 ? std::stod(argv[2]) : 0;
    const auto b1 = argc > 3 ? std::stod(argv[3]) : 1;
    const auto a2 = argc > 4 ? std::stod(argv[4]) : 0;
    const auto b2 = argc > 5 ? std::stod(argv[5]) : 1;
    // const auto a3 = argc > 6 ? std::stod(argv[6]) : 0;
    // const auto b3 = argc > 7 ? std::stod(argv[7]) : 1;

    ROOT::RDataFrame dataFrame(n);
    dataFrame
        .Define("uc",
                [&] { return Math::Random::UniformCompact<double>(a1, b1)(mt1993764); })
        .Define("ur",
                [&] { return Math::Random::Uniform<double>(a1, b1)(mt1993764); })
        .Define("ui",
                [&] { return Math::Random::Uniform<int>(a1, b1)(mt1993764); })

        .Define("ucr",
                [&] { return Math::Random::UniformCompactRectangle<muc::array2d>({a1, b1}, {a2, b2})(mt1993764); })
        .Define("urr",
                [&] { return Math::Random::UniformRectangle<muc::array2d>({a1, b1}, {a2, b2})(mt1993764); })
        .Define("uir",
                [&] { return Math::Random::UniformRectangle<muc::array2i>({(int)a1, (int)b1}, {(int)a2, (int)b2})(mt1993764); })

        // .Define("ucc",
        //         [&] { return Math::Random::UniformCompactCuboid<muc::array3d>({a1, b1}, {a2, b2}, {a3, b3})(mt1993764); })
        // .Define("urc",
        //         [&] { return Math::Random::UniformCuboid<muc::array3d>({a1, b1}, {a2, b2}, {a3, b3})(mt1993764); })
        // .Define("uic",
        //         [&] { return Math::Random::UniformCuboid<muc::array3i>({(int)a1, (int)b1}, {(int)a2, (int)b2}, {(int)a3, (int)b3})(mt1993764); })

        .Snapshot("uniform", "uniform.root");

    return EXIT_SUCCESS;
}
