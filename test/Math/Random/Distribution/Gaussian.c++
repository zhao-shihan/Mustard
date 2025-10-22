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

#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Gaussian2DDiagnoal.h++"
#include "Mustard/Math/Random/Distribution/Gaussian3DDiagnoal.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"

#include "ROOT/RDataFrame.hxx"

#include <cstdlib>

using namespace Mustard;

int main(int argc, char* argv[]) {
    Math::Random::MT1993764 mt1993764;

    const auto n = std::stod(argv[1]);
    const auto mu1 = argc > 2 ? std::stod(argv[2]) : 0;
    const auto sigma1 = argc > 3 ? std::stod(argv[3]) : 1;
    const auto mu2 = argc > 4 ? std::stod(argv[4]) : 0;
    const auto sigma2 = argc > 5 ? std::stod(argv[5]) : 1;
    const auto mu3 = argc > 6 ? std::stod(argv[6]) : 0;
    const auto sigma3 = argc > 7 ? std::stod(argv[7]) : 1;

    ROOT::RDataFrame dataFrame(n);
    dataFrame
        .Define("g1",
                [&] { return Math::Random::Gaussian(mu1, sigma1)(mt1993764); })
        .Define("g2",
                [&] { return Math::Random::Gaussian2DDiagnoal({mu1, sigma1}, {mu2, sigma2})(mt1993764); })
        .Define("g3",
                [&] { return Math::Random::Gaussian3DDiagnoal({mu1, sigma1}, {mu2, sigma2}, {mu3, sigma3})(mt1993764); })

        .Snapshot("gaussian", "gaussian.root");

    return EXIT_SUCCESS;
}
