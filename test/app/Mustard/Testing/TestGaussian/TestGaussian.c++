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

#include "Mustard/CLI/MonteCarloCLI.h++"
#include "Mustard/Data/Model.h++"
#include "Mustard/Data/Object/Field.h++"
#include "Mustard/Data/Object/Tuple.h++"
#include "Mustard/Data/Processing/Writer.h++"
#include "Mustard/Env/MPIMonteCarloEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Random/Distribution/Gaussian.h++"
#include "Mustard/Math/Random/Distribution/Gaussian2D.h++"
#include "Mustard/Math/Random/Distribution/Gaussian3D.h++"
#include "Mustard/Math/Random/Distribution/Gaussian4D.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Testing/TestGaussian/TestGaussian.h++"

#include "TFile.h"

#include "muc/array"
#include "muc/chrono"

#include <array>
#include <cstdlib>

namespace Mustard::Testing {

TestGaussian::TestGaussian() :
    Subprogram{"TestGaussian", "Test Mustard::Random::Gaussian and its variants."} {}

auto TestGaussian::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::MonteCarloCLI<> cli;
    cli->add_argument("n").help("Number of entries.").default_value(1'000'000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("--mu1").help("Mean of 1D normal distribution.").default_value(0.).nargs(1).scan<'g', double>();
    cli->add_argument("--sigma1").help("Sigma of 1D normal distribution.").default_value(1.).nargs(1).scan<'g', double>();
    cli->add_argument("--mu2").help("Mean 2 of 2D normal distribution.").default_value(1.).nargs(1).scan<'g', double>();
    cli->add_argument("--sigma2").help("Sigma 2 of 2D normal distribution.").default_value(2.).nargs(1).scan<'g', double>();
    cli->add_argument("--mu3").help("Mean 3 of 3D normal distribution.").default_value(2.).nargs(1).scan<'g', double>();
    cli->add_argument("--sigma3").help("Sigma 3 of 3D normal distribution.").default_value(3.).nargs(1).scan<'g', double>();
    cli->add_argument("--mu4").help("Mean 4 of 4D normal distribution.").default_value(3.).nargs(1).scan<'g', double>();
    cli->add_argument("--sigma4").help("Sigma 4 of 4D normal distribution.").default_value(4.).nargs(1).scan<'g', double>();
    Mustard::Env::MPIMonteCarloEnv<256> env{argc, argv, cli};

    const auto n{cli->get<unsigned long long>("n")};
    const auto mu1{cli->get<double>("--mu1")};
    const auto sigma1{cli->get<double>("--sigma1")};
    const auto mu2{cli->get<double>("--mu2")};
    const auto sigma2{cli->get<double>("--sigma2")};
    const auto mu3{cli->get<double>("--mu3")};
    const auto sigma3{cli->get<double>("--sigma3")};
    const auto mu4{cli->get<double>("--mu4")};
    const auto sigma4{cli->get<double>("--sigma4")};

    Random::Gaussian normal1D{mu1, sigma1};
    Random::GaussianS normal1DS{mu1, sigma1};
    Random::Gaussian2D normal2D({mu1, sigma1}, {mu2, sigma2});
    Random::Gaussian2DS normal2DS({mu1, sigma1}, {mu2, sigma2});
    Random::Gaussian3D normal3D({mu1, sigma1}, {mu2, sigma2}, {mu3, sigma3});
    Random::Gaussian3DS normal3DS({mu1, sigma1}, {mu2, sigma2}, {mu3, sigma3});
    Random::Gaussian4D normal4D({mu1, sigma1}, {mu2, sigma2}, {mu3, sigma3}, {mu4, sigma4});
    Random::Gaussian4DS normal4DS({mu1, sigma1}, {mu2, sigma2}, {mu3, sigma3}, {mu4, sigma4});

    struct Result
        : Data::Model<
              Data::Field<double, "n1", float>,
              Data::Field<muc::array2d, "n2", muc::array2f>,
              Data::Field<muc::array3d, "n3", muc::array3f>,
              Data::Field<muc::array4d, "n4", muc::array4f>> {};

    muc::chrono::stopwatch sw;
    std::array<Statistic1D, 8> timeStat;

    ProcessSpecificFile<TFile> file{"test_normal_distribution.root", "RECREATE"};

    Data::Writer<Result> writer{"Gaussian"};
    Mustard::Executor<unsigned long long>{}.Run(n, [&](unsigned long long) {
        Data::Tuple<Result> entry;
        auto ts{timeStat.begin()};

        sw.reset();
        F<"n1">(entry) = normal1D(env.RandomEngine());
        ts++->Fill(sw.read().count());

        sw.reset();
        F<"n2">(entry) = normal2D(env.RandomEngine());
        ts++->Fill(sw.read().count());

        sw.reset();
        F<"n3">(entry) = normal3D(env.RandomEngine());
        ts++->Fill(sw.read().count());

        sw.reset();
        F<"n4">(entry) = normal4D(env.RandomEngine());
        ts++->Fill(sw.read().count());

        writer.Fill(entry);
    });

    Data::Writer<Result> writerS{"GaussianS"};
    Mustard::Executor<unsigned long long>{}.Run(n, [&](unsigned long long) {
        Data::Tuple<Result> entry;
        auto ts{timeStat.begin() + 4};

        sw.reset();
        F<"n1">(entry) = normal1DS(env.RandomEngine());
        ts++->Fill(sw.read().count());

        sw.reset();
        F<"n2">(entry) = normal2DS(env.RandomEngine());
        ts++->Fill(sw.read().count());

        sw.reset();
        F<"n3">(entry) = normal3DS(env.RandomEngine());
        ts++->Fill(sw.read().count());

        sw.reset();
        F<"n4">(entry) = normal4DS(env.RandomEngine());
        ts++->Fill(sw.read().count());

        writerS.Fill(entry);
    });

    PrintLn("CPU time per sample:");
    PrintLn("  Gaussian:     {:.2f} +/- {:.2f} ns", timeStat[0].Mean(), timeStat[0].StdDevOfMean());
    PrintLn("  Gaussian2D:   {:.2f} +/- {:.2f} ns", timeStat[1].Mean(), timeStat[1].StdDevOfMean());
    PrintLn("  Gaussian3D:   {:.2f} +/- {:.2f} ns", timeStat[2].Mean(), timeStat[2].StdDevOfMean());
    PrintLn("  Gaussian4D:   {:.2f} +/- {:.2f} ns", timeStat[3].Mean(), timeStat[3].StdDevOfMean());
    PrintLn("  GaussianS:    {:.2f} +/- {:.2f} ns", timeStat[4].Mean(), timeStat[4].StdDevOfMean());
    PrintLn("  Gaussian2DS:  {:.2f} +/- {:.2f} ns", timeStat[5].Mean(), timeStat[5].StdDevOfMean());
    PrintLn("  Gaussian3DS:  {:.2f} +/- {:.2f} ns", timeStat[6].Mean(), timeStat[6].StdDevOfMean());
    PrintLn("  Gaussian4DS:  {:.2f} +/- {:.2f} ns", timeStat[7].Mean(), timeStat[7].StdDevOfMean());

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
