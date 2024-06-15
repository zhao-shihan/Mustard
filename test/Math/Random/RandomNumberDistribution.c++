#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/Generator/MT1993732.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Random/RandomNumberDistribution.h++"
#include "Mustard/Math/Random/RandomNumberDistributionBase.h++"

#include <iostream>
#include <random>

using namespace Mustard;

int main(int, char* argv[]) {
    Math::Random::Xoshiro256Plus rng;
    // Math::Random::MT1993732 rng;

    using RNG = decltype(rng);

    auto x1 = std::stod(argv[1]);
    auto x2 = std::stod(argv[2]);
    auto a = x1;
    for (auto i = 0ULL; i < 2000000000ULL; ++i) {
        do {
            const auto u = static_cast<decltype(a)>(1 / static_cast<long double>(RNG::Max() - RNG::Min())) *
                           (rng() - RNG::Min());
            a = x1 * (1 - u) + x2 * u;
        } while (a <= x1 || a >= x2);
        if (a <= x1 || a >= x2) {
            std::cout << a << std::endl;
            throw "Failed";
        }
        a = Math::Random::Uniform<double>()(rng);
    }
    Math::Random::Uniform<double>().Reset();
    std::cout << a << std::endl;

    return 0;
}

static_assert(Math::Random::STDRandomNumberDistribution<std::uniform_int_distribution<int>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::uniform_real_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::bernoulli_distribution>);
static_assert(Math::Random::STDRandomNumberDistribution<std::binomial_distribution<int>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::negative_binomial_distribution<int>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::geometric_distribution<int>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::poisson_distribution<int>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::exponential_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::gamma_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::weibull_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::extreme_value_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::normal_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::lognormal_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::chi_squared_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::cauchy_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::fisher_f_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::student_t_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::discrete_distribution<int>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::piecewise_constant_distribution<double>>);
static_assert(Math::Random::STDRandomNumberDistribution<std::piecewise_linear_distribution<double>>);
