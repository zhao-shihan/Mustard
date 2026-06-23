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

#pragma once

#include "Mustard/Math/Statistic.h++"
#include "Mustard/Utility/Base64.h++"

#include "Eigen/Core"

#include "muc/numeric"

#include "fmt/format.h"
#include "fmt/ostream.h"

#include <cmath>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace Mustard::Testing::TestStatistic {

using namespace Mustard::Math;

// =========================================================================
// Helpers: generate test data for arbitrary dimension
// =========================================================================

// Generate a vector [start, start+1, ..., start+dim-1]
template<int K>
auto MakeSeqVector(int dim, double start = 1.0) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v{dim};
        for (auto i{0}; i < dim; ++i) {
            v[i] = start + i;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        for (auto i{0}; i < dim; ++i) {
            v[i] = start + i;
        }
        return v;
    }
}

// Generate a constant vector [val, val, ..., val]
template<int K>
auto MakeConstVector(int dim, double val) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v{dim};
        for (auto i{0}; i < dim; ++i) {
            v[i] = val;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        for (auto i{0}; i < dim; ++i) {
            v[i] = val;
        }
        return v;
    }
}

// =========================================================================
// Helper: construct Statistic with correct dynamic/static behavior
// =========================================================================
template<int K, CovarianceOption C = CovarianceOption::Full>
auto MakeStatistic(int dim = 0) -> Statistic<K, C> {
    if constexpr (K == Eigen::Dynamic) {
        return Statistic<K, C>(dim);
    } else {
        return Statistic<K, C>{};
    }
}

// =========================================================================
// Iteration helpers
// =========================================================================

// All static K dimensions to test (K>=1)
using AllStaticDims = std::integer_sequence<int, 1, 2, 3, 5, 10>;

// Run a test function over all static K>=2 with both Full and Diagonal
template<int... Ks, typename Func, typename... Args>
auto RunOverStaticDims(Func&& func, Args&&... args) -> void {
    (std::forward<Func>(func).template operator()<Ks, CovarianceOption::Full>(std::forward<Args>(args)...), ...);
    (std::forward<Func>(func).template operator()<Ks, CovarianceOption::Diagonal>(std::forward<Args>(args)...), ...);
}

// Run a test function over dynamic dimension with both Full and Diagonal
template<typename Func, typename... Args>
auto RunOverDynamic(Func&& func, Args&&... args) -> void {
    std::forward<Func>(func).template operator()<Eigen::Dynamic, CovarianceOption::Full>(std::forward<Args>(args)...);
    std::forward<Func>(func).template operator()<Eigen::Dynamic, CovarianceOption::Diagonal>(std::forward<Args>(args)...);
}

// Run a test function over all K>=2 (static + dynamic) with both Full and Diagonal
template<int... Ks, typename Func, typename... Args>
auto RunOverAllDims(Func&& func, Args&&... args) -> void {
    RunOverStaticDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    RunOverDynamic(std::forward<Func>(func), std::forward<Args>(args)...);
}

// Wrappers that accept std::integer_sequence (AllStaticDims) and deduce the int pack
template<typename Seq, typename Func, typename... Args>
auto RunOverStaticDims(Func&& func, Args&&... args) -> void {
    [&]<int... Ks>(std::integer_sequence<int, Ks...>) {
        RunOverStaticDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    }(Seq{});
}

template<typename Seq, typename Func, typename... Args>
auto RunOverAllDims(Func&& func, Args&&... args) -> void {
    [&]<int... Ks>(std::integer_sequence<int, Ks...>) {
        RunOverAllDims<Ks...>(std::forward<Func>(func), std::forward<Args>(args)...);
    }(Seq{});
}

// =========================================================================
// Checker functions
// =========================================================================

template<typename T, typename U>
auto CheckClose(const T& actual, const U& expected, const std::string& ctx) -> void {
    if constexpr (std::is_arithmetic_v<T>) {
        if (not muc::isclose(actual, expected)) {
            Throw<std::runtime_error>(fmt::format("\n"
                                                  "{}: expected {}, got {}",
                                                  ctx, expected, actual));
        }
    } else {
        auto isCloseMat{[](const auto& a, const auto& b) {
            for (auto i{0}; i < a.rows(); ++i) {
                for (auto j{0}; j < a.cols(); ++j) {
                    if (not muc::isclose(a.coeff(i, j), b.coeff(i, j))) {
                        return false;
                    }
                }
            }
            return true;
        }};
        if (not isCloseMat(actual, expected)) {
            Throw<std::runtime_error>(fmt::format("\n"
                                                  "{}: expected approx\n"
                                                  "{},\n"
                                                  "got\n"
                                                  "{}",
                                                  ctx, fmt::streamed(expected), fmt::streamed(actual)));
        }
    }
}

auto CheckEq(auto actual, auto expected, const std::string& ctx) -> void {
    if (actual != expected) {
        Throw<std::runtime_error>(fmt::format("\n"
                                              "{}: expected {}, got {}",
                                              ctx, expected, actual));
    }
}

} // namespace Mustard::Testing::TestStatistic
