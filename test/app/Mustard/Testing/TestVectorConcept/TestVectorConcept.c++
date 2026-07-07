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
#include "Mustard/Concept/MathVector.h++"
#include "Mustard/Concept/NumericVector.h++"
#include "Mustard/EigenX/AlignedVector3.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Math/Vector.h++"
#include "Mustard/Testing/TestVectorConcept/TestVectorConcept.h++"

#if __has_include("TEveVector.h")
#    include "TEveVector.h"
#endif

#include "Eigen/Core"
#include "unsupported/Eigen/AlignedVector3"

#include "muc/array"

#include <array>
#include <vector>

namespace Mustard::Testing {

static_assert(Concept::InputVector3F<float*>);
static_assert(Concept::InputVector3D<double*>);
static_assert(Concept::InputVector3LD<long double*>);
static_assert(not Concept::NumericVectorAny<float*>);
static_assert(not Concept::NumericVectorAny<double*>);
static_assert(not Concept::NumericVectorAny<long double*>);

static_assert(Concept::InputVector3F<const float*>);
static_assert(Concept::InputVector3D<const double*>);
static_assert(Concept::InputVector3LD<const long double*>);
static_assert(not Concept::NumericVectorAny<const float*>);
static_assert(not Concept::NumericVectorAny<const double*>);
static_assert(not Concept::NumericVectorAny<const long double*>);

static_assert(Concept::InputVector3F<float[3]>);
static_assert(Concept::InputVector3D<double[3]>);
static_assert(Concept::InputVector3LD<long double[3]>);
static_assert(not Concept::NumericVectorAny<float[3]>);
static_assert(not Concept::NumericVectorAny<double[3]>);
static_assert(not Concept::NumericVectorAny<long double[3]>);

static_assert(Concept::InputVector3F<const float[3]>);
static_assert(Concept::InputVector3D<const double[3]>);
static_assert(Concept::InputVector3LD<const long double[3]>);
static_assert(not Concept::NumericVectorAny<const float[3]>);
static_assert(not Concept::NumericVectorAny<const double[3]>);
static_assert(not Concept::NumericVectorAny<const long double[3]>);

static_assert(Concept::InputVector3CF<std::complex<float>*>);
static_assert(Concept::InputVector3CD<std::complex<double>*>);
static_assert(Concept::InputVector3CLD<std::complex<long double>*>);
static_assert(not Concept::NumericVectorAny<std::complex<float>*>);
static_assert(not Concept::NumericVectorAny<std::complex<double>*>);
static_assert(not Concept::NumericVectorAny<std::complex<long double>*>);

static_assert(Concept::InputVector3CF<const std::complex<float>*>);
static_assert(Concept::InputVector3CD<const std::complex<double>*>);
static_assert(Concept::InputVector3CLD<const std::complex<long double>*>);
static_assert(not Concept::NumericVectorAny<const std::complex<float>*>);
static_assert(not Concept::NumericVectorAny<const std::complex<double>*>);
static_assert(not Concept::NumericVectorAny<const std::complex<long double>*>);

static_assert(Concept::InputVector3CF<std::complex<float>[3]>);
static_assert(Concept::InputVector3CD<std::complex<double>[3]>);
static_assert(Concept::InputVector3CLD<std::complex<long double>[3]>);
static_assert(not Concept::NumericVectorAny<std::complex<float>[3]>);
static_assert(not Concept::NumericVectorAny<std::complex<double>[3]>);
static_assert(not Concept::NumericVectorAny<std::complex<long double>[3]>);

static_assert(Concept::InputVector3CF<const std::complex<float>[3]>);
static_assert(Concept::InputVector3CD<const std::complex<double>[3]>);
static_assert(Concept::InputVector3CLD<const std::complex<long double>[3]>);
static_assert(not Concept::NumericVectorAny<const std::complex<float>[3]>);
static_assert(not Concept::NumericVectorAny<const std::complex<double>[3]>);
static_assert(not Concept::NumericVectorAny<const std::complex<long double>[3]>);

static_assert(Concept::InputVector2F<std::vector<float>>);
static_assert(Concept::InputVector3F<std::vector<float>>);
static_assert(Concept::InputVector4F<std::vector<float>>);
static_assert(Concept::InputVector2D<std::vector<double>>);
static_assert(Concept::InputVector3D<std::vector<double>>);
static_assert(Concept::InputVector4D<std::vector<double>>);
static_assert(Concept::InputVector2LD<std::vector<long double>>);
static_assert(Concept::InputVector3LD<std::vector<long double>>);
static_assert(Concept::InputVector4LD<std::vector<long double>>);
static_assert(not Concept::NumericVectorAny<std::vector<float>>);
static_assert(not Concept::NumericVectorAny<std::vector<float>>);
static_assert(not Concept::NumericVectorAny<std::vector<float>>);
static_assert(not Concept::NumericVectorAny<std::vector<double>>);
static_assert(not Concept::NumericVectorAny<std::vector<double>>);
static_assert(not Concept::NumericVectorAny<std::vector<double>>);
static_assert(not Concept::NumericVectorAny<std::vector<long double>>);
static_assert(not Concept::NumericVectorAny<std::vector<long double>>);
static_assert(not Concept::NumericVectorAny<std::vector<long double>>);

static_assert(Concept::InputVector2CF<std::vector<std::complex<float>>>);
static_assert(Concept::InputVector3CF<std::vector<std::complex<float>>>);
static_assert(Concept::InputVector4CF<std::vector<std::complex<float>>>);
static_assert(Concept::InputVector2CD<std::vector<std::complex<double>>>);
static_assert(Concept::InputVector3CD<std::vector<std::complex<double>>>);
static_assert(Concept::InputVector4CD<std::vector<std::complex<double>>>);
static_assert(Concept::InputVector2CLD<std::vector<std::complex<long double>>>);
static_assert(Concept::InputVector3CLD<std::vector<std::complex<long double>>>);
static_assert(Concept::InputVector4CLD<std::vector<std::complex<long double>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<float>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<float>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<float>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<double>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<double>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<double>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<long double>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<long double>>>);
static_assert(not Concept::NumericVectorAny<std::vector<std::complex<long double>>>);

static_assert(Concept::NumericVector2F<muc::array2f>);
static_assert(Concept::NumericVector3F<muc::array3f>);
static_assert(Concept::NumericVector4F<muc::array4f>);
static_assert(Concept::NumericVector2D<muc::array2d>);
static_assert(Concept::NumericVector3D<muc::array3d>);
static_assert(Concept::NumericVector4D<muc::array4d>);
static_assert(Concept::NumericVector2LD<muc::array2ld>);
static_assert(Concept::NumericVector3LD<muc::array3ld>);
static_assert(Concept::NumericVector4LD<muc::array4ld>);

static_assert(Concept::NumericVector2CF<muc::array2cf>);
static_assert(Concept::NumericVector3CF<muc::array3cf>);
static_assert(Concept::NumericVector4CF<muc::array4cf>);
static_assert(Concept::NumericVector2CD<muc::array2cd>);
static_assert(Concept::NumericVector3CD<muc::array3cd>);
static_assert(Concept::NumericVector4CD<muc::array4cd>);
static_assert(Concept::NumericVector2CLD<muc::array2cld>);
static_assert(Concept::NumericVector3CLD<muc::array3cld>);
static_assert(Concept::NumericVector4CLD<muc::array4cld>);

static_assert(Concept::MathVector2F<Eigen::Vector2f>);
static_assert(Concept::MathVector3F<Eigen::Vector3f>);
static_assert(Concept::MathVector4F<Eigen::Vector4f>);
static_assert(Concept::MathVector2D<Eigen::Vector2d>);
static_assert(Concept::MathVector3D<Eigen::Vector3d>);
static_assert(Concept::MathVector4D<Eigen::Vector4d>);

static_assert(Concept::MathVector2CD<Eigen::Vector2cd>);
static_assert(Concept::MathVector3CD<Eigen::Vector3cd>);
static_assert(Concept::MathVector4CD<Eigen::Vector4cd>);
static_assert(Concept::MathVector2CF<Eigen::Vector2cf>);
static_assert(Concept::MathVector3CF<Eigen::Vector3cf>);
static_assert(Concept::MathVector4CF<Eigen::Vector4cf>);

static_assert(Concept::MathVector2D<Mustard::Vector2D>);
static_assert(Concept::MathVector3D<Mustard::Vector3D>);
static_assert(Concept::MathVector4D<Mustard::VectorLor>);

static_assert(Concept::InputVector2Complex<std::vector<std::complex<double>>>);
static_assert(Concept::InputVector3Complex<std::vector<std::complex<float>>>);
static_assert(Concept::InputVector4Complex<std::vector<std::complex<double>>>);
static_assert(Concept::NumericVector2Complex<muc::array2cd>);
static_assert(Concept::NumericVector3Complex<muc::array3cf>);
static_assert(Concept::NumericVector4Complex<muc::array4cld>);
static_assert(Concept::MathVector2Complex<Eigen::Vector2cf>);
static_assert(Concept::MathVector3Complex<Eigen::Vector3cd>);
static_assert(Concept::MathVector4Complex<Eigen::Vector4cd>);

static_assert(Concept::InputVectorAny<std::vector<std::complex<float>>, 3>);
static_assert(Concept::InputVector4Any<std::vector<std::complex<double>>>);
static_assert(Concept::NumericVectorAny<muc::array2cd, 2>);
static_assert(Concept::NumericVector3Any<muc::array3cd>);
static_assert(Concept::MathVectorAny<Eigen::Vector3cf, 3>);
static_assert(Concept::MathVector4Any<Eigen::Vector4cd>);

// Eigen don't vectorize long double vector so Eigen::AlignedVector3<long double>'s align is 16;
// (and that's why you shouldn't use long double with Eigen)
// NumericVector doesn't allow a alignof(T) == alignof(F) type to be sizeof(T) / sizeof(F) > N
namespace EigenX = Mustard::EigenX;
static_assert(Concept::MathVector3F<EigenX::AlignedVector3f>);
static_assert(Concept::MathVector3D<EigenX::AlignedVector3d>);
static_assert(Concept::MathVector3CF<EigenX::AlignedVector3cf>);
static_assert(Concept::MathVector3CD<EigenX::AlignedVector3cd>);
static_assert(not Concept::NumericVectorAny<Eigen::AlignedVector3<long double>>);
static_assert(not Concept::NumericVectorAny<Eigen::AlignedVector3<std::complex<long double>>>);

#if __has_include("TEveVector.h")
static_assert(not Concept::MathVectorAny<TEveVector2D>);
static_assert(not Concept::MathVectorAny<TEveVectorD>);
static_assert(not Concept::MathVectorAny<TEveVector4D>);
static_assert(not Concept::MathVectorAny<TEveVector2F>);
static_assert(not Concept::MathVectorAny<TEveVectorF>);
static_assert(not Concept::MathVectorAny<TEveVector4F>);
#endif

TestVectorConcept::TestVectorConcept() :
    Subprogram{"TestVectorConcept", "Test Mustard::Concept vector concepts."} {}

auto TestVectorConcept::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    PrintLn("I'm compiled, I'm passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
