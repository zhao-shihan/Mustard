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
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Distribution/UniformRectangle.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"
#include "Mustard/Testing/TestUniform/TestUniform.h++"

#include "ROOT/RDataFrame.hxx"

#include "muc/array"

namespace Mustard::Testing {

TestUniform::TestUniform() :
    Subprogram{"TestUniform", "Test Mustard::Random::Uniform and its variants."} {}

auto TestUniform::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    cli->add_argument("n").help("Number of entries.").default_value(1'000'000ull).required().nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-a1", "--a1").help("Lower bound of 1D uniform.").default_value(0.).nargs(1).scan<'g', double>();
    cli->add_argument("-b1", "--b1").help("Upper bound of 1D uniform.").default_value(1.).nargs(1).scan<'g', double>();
    cli->add_argument("-a2", "--a2").help("Lower bound 2 of 2D uniform.").default_value(0.).nargs(1).scan<'g', double>();
    cli->add_argument("-b2", "--b2").help("Upper bound 2 of 2D uniform.").default_value(1.).nargs(1).scan<'g', double>();
    Mustard::Env::BasicEnv env{argc, argv, cli};

    Random::MT1993764 mt1993764;

    const auto n = cli->get<unsigned long long>("n");
    const auto a1 = cli->get<double>("--a1");
    const auto b1 = cli->get<double>("--b1");
    const auto a2 = cli->get<double>("--a2");
    const auto b2 = cli->get<double>("--b2");

    ROOT::RDataFrame dataFrame(n);
    dataFrame
        .Define("uc",
                [&] { return Random::UniformCompact<double>(a1, b1)(mt1993764); })
        .Define("ur",
                [&] { return Random::Uniform<double>(a1, b1)(mt1993764); })
        .Define("ui",
                [&] { return Random::Uniform<int>(a1, b1)(mt1993764); })

        .Define("ucr",
                [&] { return Random::UniformCompactRectangle<muc::array2d>({a1, b1}, {a2, b2})(mt1993764); })
        .Define("urr",
                [&] { return Random::UniformRectangle<muc::array2d>({a1, b1}, {a2, b2})(mt1993764); })
        .Define("uir",
                [&] { return Random::UniformRectangle<muc::array2i>({(int)a1, (int)b1}, {(int)a2, (int)b2})(mt1993764); })

        // .Define("ucc",
        //         [&] { return Random::UniformCompactCuboid<muc::array3d>({a1, b1}, {a2, b2}, {a3, b3})(mt1993764); })
        // .Define("urc",
        //         [&] { return Random::UniformCuboid<muc::array3d>({a1, b1}, {a2, b2}, {a3, b3})(mt1993764); })
        // .Define("uic",
        //         [&] { return Random::UniformCuboid<muc::array3i>({(int)a1, (int)b1}, {(int)a2, (int)b2}, {(int)a3, (int)b3})(mt1993764); })

        .Snapshot("uniform", "test_uniform.root");

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
