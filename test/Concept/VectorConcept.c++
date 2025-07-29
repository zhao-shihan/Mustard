#include "Mustard/Concept/MathVector.h++"
#include "Mustard/Concept/NumericVector.h++"

#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/TwoVector.h"

#if __has_include("TEveVector.h")
#    include "TEveVector.h"
#endif

#include "Eigen/Core"

#include "muc/array"

#include <array>
#include <iostream>
#include <vector>

using namespace Mustard::Concept;

static_assert(InputVector<float*, float, 3>);
static_assert(InputVector<double*, double, 3>);
static_assert(InputVector<long double*, long double, 3>);
static_assert(not NumericVector<float*, float, 3>);
static_assert(not NumericVector<double*, double, 3>);
static_assert(not NumericVector<long double*, long double, 3>);

static_assert(InputVector<const float*, float, 3>);
static_assert(InputVector<const double*, double, 3>);
static_assert(InputVector<const long double*, long double, 3>);
static_assert(not NumericVector<const float*, float, 3>);
static_assert(not NumericVector<const double*, double, 3>);
static_assert(not NumericVector<const long double*, long double, 3>);

static_assert(InputVector<float[3], float, 3>);
static_assert(InputVector<double[3], double, 3>);
static_assert(InputVector<long double[3], long double, 3>);
static_assert(not NumericVector<float[3], float, 3>);
static_assert(not NumericVector<double[3], double, 3>);
static_assert(not NumericVector<long double[3], long double, 3>);

static_assert(InputVector<const float[3], float, 3>);
static_assert(InputVector<const double[3], double, 3>);
static_assert(InputVector<const long double[3], long double, 3>);
static_assert(not NumericVector<const float[3], float, 3>);
static_assert(not NumericVector<const double[3], double, 3>);
static_assert(not NumericVector<const long double[3], long double, 3>);

static_assert(NumericVector<muc::array2f, float, 2>);
static_assert(NumericVector<muc::array3f, float, 3>);
static_assert(NumericVector<muc::array4f, float, 4>);
static_assert(NumericVector<muc::array2d, double, 2>);
static_assert(NumericVector<muc::array3d, double, 3>);
static_assert(NumericVector<muc::array4d, double, 4>);

// static_assert(not NumericVector<std::vector<float>, float, 2>);
// static_assert(not NumericVector<std::vector<float>, float, 3>);
// static_assert(not NumericVector<std::vector<float>, float, 4>);
// static_assert(not NumericVector<std::vector<double>, double, 2>);
// static_assert(not NumericVector<std::vector<double>, double, 3>);
// static_assert(not NumericVector<std::vector<double>, double, 4>);

static_assert(MathVector<Eigen::Vector2f, float, 2>);
static_assert(MathVector<Eigen::Vector3f, float, 3>);
static_assert(MathVector<Eigen::Vector4f, float, 4>);
static_assert(MathVector<Eigen::Vector2d, double, 2>);
static_assert(MathVector<Eigen::Vector3d, double, 3>);
static_assert(MathVector<Eigen::Vector4d, double, 4>);

static_assert(MathVector<CLHEP::Hep2Vector, double, 2>);
static_assert(MathVector<CLHEP::Hep3Vector, double, 3>);
static_assert(MathVector<CLHEP::HepLorentzVector, double, 4>);

#if __has_include("TEveVector.h")
static_assert(not MathVector<TEveVector2D, double, 2>);
static_assert(not MathVector<TEveVectorD, double, 3>);
static_assert(not MathVector<TEveVector4D, double, 4>);
static_assert(not MathVector<TEveVector2F, float, 2>);
static_assert(not MathVector<TEveVectorF, float, 3>);
static_assert(not MathVector<TEveVector4F, float, 4>);
#endif

int main() {
    std::cout << "I'm compiled, I'm passed." << std::endl;
    return EXIT_SUCCESS;
}
