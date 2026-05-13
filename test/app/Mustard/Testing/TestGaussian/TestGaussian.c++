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

#include "Mustard/CLI/MonteCarloCLI.h++"
#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Field.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/Writer.h++"
#include "Mustard/Env/MPIMonteCarloEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Gaussian2DDiagnoal.h++"
#include "Mustard/Math/Random/Distribution/Gaussian3DDiagnoal.h++"
#include "Mustard/Testing/TestGaussian/TestGaussian.h++"

#include "TFile.h"

#include "muc/array"

#include <cstdlib>

namespace Mustard::Testing {

TestGaussian::TestGaussian() :
    Subprogram{"TestGaussian", "Test Mustard::Random::Gaussian and its variants."} {}

auto TestGaussian::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::MonteCarloCLI<> cli;
    cli->add_argument("n").help("Number of entries.").default_value(1'000'000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("--mu1").help("Mean of 1D Gaussian.").default_value(0.).nargs(1).scan<'g', double>();
    cli->add_argument("--sigma1").help("Sigma of 1D Gaussian.").default_value(1.).nargs(1).scan<'g', double>();
    cli->add_argument("--mu2").help("Mean 2 of 2D Gaussian.").default_value(1.).nargs(1).scan<'g', double>();
    cli->add_argument("--sigma2").help("Sigma 2 of 2D Gaussian.").default_value(2.).nargs(1).scan<'g', double>();
    cli->add_argument("--mu3").help("Mean 3 of 3D Gaussian.").default_value(2.).nargs(1).scan<'g', double>();
    cli->add_argument("--sigma3").help("Sigma 3 of 3D Gaussian.").default_value(3.).nargs(1).scan<'g', double>();
    Mustard::Env::MPIMonteCarloEnv<256> env{argc, argv, cli};

    const auto n{cli->get<unsigned long long>("n")};
    const auto mu1{cli->get<double>("--mu1")};
    const auto sigma1{cli->get<double>("--sigma1")};
    const auto mu2{cli->get<double>("--mu2")};
    const auto sigma2{cli->get<double>("--sigma2")};
    const auto mu3{cli->get<double>("--mu3")};
    const auto sigma3{cli->get<double>("--sigma3")};

    Random::Gaussian gaussian1D{mu1, sigma1};
    Random::Gaussian2DDiagnoal gaussian2D({mu1, sigma1}, {mu2, sigma2});
    Random::Gaussian3DDiagnoal gaussian3D({mu1, sigma1}, {mu2, sigma2}, {mu3, sigma3});

    struct Result
        : Data::Model<
              Data::Field<double, "g1">,
              Data::Field<muc::array2d, "g2">,
              Data::Field<muc::array3d, "g3">> {};

    ProcessSpecificFile<TFile> file{"test_gaussian.root", "RECREATE"};
    Data::Writer<Result> writer{"Gaussian"};
    Mustard::Executor<unsigned long long>{}.Run(n, [&](unsigned long long) {
        Data::Tuple<Result> entry;
        F<"g1">(entry) = gaussian1D(env.RandomEngine());
        F<"g2">(entry) = gaussian2D(env.RandomEngine());
        F<"g3">(entry) = gaussian3D(env.RandomEngine());
        writer.Fill(entry);
    });

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
