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
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Distribution/UniformRectangle.h++"
#include "Mustard/Testing/TestUniform/TestUniform.h++"

#include "TFile.h"

#include "muc/array"

#include <cstdlib>

namespace Mustard::Testing {

TestUniform::TestUniform() :
    Subprogram{"TestUniform", "Test Mustard::Random::Uniform and its variants."} {}

auto TestUniform::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::MonteCarloCLI<> cli;
    cli->add_argument("n").help("Number of entries.").default_value(1'000'000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("--a1").help("Lower bound of 1D uniform.").default_value(0.).nargs(1).scan<'g', double>();
    cli->add_argument("--b1").help("Upper bound of 1D uniform.").default_value(1.).nargs(1).scan<'g', double>();
    cli->add_argument("--a2").help("Lower bound 2 of 2D uniform.").default_value(0.).nargs(1).scan<'g', double>();
    cli->add_argument("--b2").help("Upper bound 2 of 2D uniform.").default_value(1.).nargs(1).scan<'g', double>();
    Mustard::Env::MPIMonteCarloEnv<256> env{argc, argv, cli};

    const auto n{cli->get<unsigned long long>("n")};
    const auto a1{cli->get<double>("--a1")};
    const auto b1{cli->get<double>("--b1")};
    const auto a2{cli->get<double>("--a2")};
    const auto b2{cli->get<double>("--b2")};

    Random::UniformCompact<double> uniformCompact1D{a1, b1};
    Random::Uniform<double> uniform1D{a1, b1};
    Random::Uniform<int> uniformInt1D{int(a1), int(b1)};

    Random::UniformCompactRectangle<muc::array2d> uniformCompactRectangle({a1, b1}, {a2, b2});
    Random::UniformRectangle<muc::array2d> uniformRectangle({a1, b1}, {a2, b2});
    Random::UniformRectangle<muc::array2i> uniformIntRectangle({int(a1), int(b1)}, {int(a2), int(b2)});

    struct Result
        : Data::Model<
              Data::Field<double, "uc", float>,
              Data::Field<double, "ur", float>,
              Data::Field<int, "ui", std::int32_t>,
              Data::Field<muc::array2d, "ucr", muc::array2f>,
              Data::Field<muc::array2d, "urr", muc::array2f>,
              Data::Field<muc::array2i, "uir", muc::array2i32>> {};

    ProcessSpecificFile<TFile> file{"test_uniform.root", "RECREATE"};
    Data::Writer<Result> writer{"Uniform"};
    Mustard::Executor<unsigned long long>{}.Run(n, [&](unsigned long long) {
        Data::Tuple<Result> entry;
        F<"uc">(entry) = uniformCompact1D(env.RandomEngine());
        F<"ur">(entry) = uniform1D(env.RandomEngine());
        F<"ui">(entry) = uniformInt1D(env.RandomEngine());
        F<"ucr">(entry) = uniformCompactRectangle(env.RandomEngine());
        F<"urr">(entry) = uniformRectangle(env.RandomEngine());
        F<"uir">(entry) = uniformIntRectangle(env.RandomEngine());
        writer.Fill(entry);
    });

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
