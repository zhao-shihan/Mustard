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

#pragma once

#include "Mustard/Math/Estimate.h++"
#include "Mustard/Utility/Base64.h++"

#include "Eigen/Core"

#include "muc/numeric"

#include "fmt/format.h"
#include "fmt/ostream.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace Mustard::Testing::TestEstimate {

using namespace Mustard::Math;

// =========================================================================
// Helper: make an Eigen vector from an initializer list
// =========================================================================
template<int K>
auto MakeVector(std::initializer_list<double> init) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v(init.size());
        auto i{0};
        for (auto val : init) {
            v[i++] = val;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        v.setZero();
        auto i{0};
        for (auto val : init) {
            v[i++] = val;
        }
        return v;
    }
}

template<int K>
auto MakeMatrix(std::initializer_list<double> init, int rows, int cols) -> auto {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::MatrixXd m{rows, cols};
        auto i{0};
        for (auto r{0}; r < rows; ++r) {
            for (auto c{0}; c < cols; ++c) {
                m(r, c) = std::data(init)[i++];
            }
        }
        return m;
    } else {
        Eigen::Matrix<double, K, K> m;
        auto i{0};
        for (auto r{0}; r < rows; ++r) {
            for (auto c{0}; c < cols; ++c) {
                m(r, c) = std::data(init)[i++];
            }
        }
        return m;
    }
}

// =========================================================================
// Unified factory functions for test data
// =========================================================================

// MakeTestValue<K, C>(dim) → always [1.0, 2.0, 3.0, ..., dim]
template<int K, CovarianceOption C>
auto MakeTestValue(int dim) -> Eigen::Vector<double, K> {
    if constexpr (K == Eigen::Dynamic) {
        Eigen::VectorXd v(dim);
        for (auto i{0}; i < dim; ++i) {
            v[i] = i + 1;
        }
        return v;
    } else {
        Eigen::Vector<double, K> v;
        for (auto i{0}; i < K; ++i) {
            v[i] = i + 1;
        }
        return v;
    }
}

// MakeTestCov<K, C>(dim) → tri-diagonal covariance, correct type per C
// Full:    primes [4,3,5,7,11,...] on diagonal, 1.0 on adjacent off-diagonals
// Diagonal: same primes, stored as DiagonalMatrix
template<int K, CovarianceOption C>
auto MakeTestCov(int dim) -> auto {
    const double diagVals[]{0.4, 0.3, 0.5, 0.7, 1.1, 1.3, 1.7, 1.9, 2.3, 2.9};
    if constexpr (K == Eigen::Dynamic) {
        Eigen::MatrixXd m(dim, dim);
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                if (i == j) {
                    m(i, j) = diagVals[i];
                } else if (i == j - 1 or j == i - 1) {
                    m(i, j) = 0.1;
                } else {
                    m(i, j) = 0.0;
                }
            }
        }
        if constexpr (C == CovarianceOption::Full) {
            return m;
        } else {
            return Eigen::DiagonalMatrix<double, Eigen::Dynamic>{m.diagonal()};
        }
    } else {
        Eigen::Matrix<double, K, K> m;
        for (auto i{0}; i < K; ++i) {
            for (auto j{0}; j < K; ++j) {
                if (i == j and i < dim) {
                    m(i, j) = diagVals[i];
                } else if ((i == j - 1 or j == i - 1) and i < dim and j < dim) {
                    m(i, j) = 0.1;
                } else {
                    m(i, j) = 0.0;
                }
            }
        }
        if constexpr (C == CovarianceOption::Full) {
            return m;
        } else {
            return Eigen::DiagonalMatrix<double, K>{m.diagonal()};
        }
    }
}

// MakeEstimate<K, C>(x, cov) → construct Estimate<K, C>
// For K=1, uses the scalar constructor Estimate{value, variance}.
template<int K, CovarianceOption C>
auto MakeEstimate(const Eigen::Vector<double, K>& x, const auto& cov) -> Estimate<K, C> {
    if constexpr (K == 1) {
        return Estimate<K, C>{x(0), cov.diagonal()(0)};
    } else {
        return Estimate<K, C>{x, cov};
    }
}

// Evaluate an Eigen expression template into a concrete type.
// Only used when the expression captures temporaries (e.g. auto x = mat * scalar).
template<int K, CovarianceOption C, typename Expr>
auto EvalCov(const Expr& expr) -> auto {
    if constexpr (C == CovarianceOption::Full) {
        return Eigen::Matrix<double, K, K>{expr};
    } else {
        return Eigen::DiagonalMatrix<double, K>{expr};
    }
}

// =========================================================================
// Iteration helpers
// =========================================================================

// All static K dimensions to test
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

// Helper: check element-wise Jacobian-based covariance for a math function
template<int K, CovarianceOption C, typename Func>
auto CheckMathFunction(
    const Estimate<K, C>& e,
    const auto& cov,
    const std::string& name,
    Func jacobian) -> void {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};
    for (auto i{0}; i < dim; ++i) {
        for (auto j{0}; j < dim; ++j) {
            double cov_ij;
            if constexpr (isFull) {
                cov_ij = cov(i, j);
            } else {
                cov_ij = (i == j) ? cov.diagonal()(i) : 0.0;
            }
            auto expectedCov{jacobian(i) * cov_ij * jacobian(j)};
            CheckClose(e.Covariance(i, j), expectedCov,
                       fmt::format("{}: Cov({},{})", name, i, j));
        }
    }
}

// Helper: check element-wise binary Jacobian-based covariance
// Cov_result(i,j) = jacX(i)*covX(i,j)*jacX(j) + jacY(i)*covY(i,j)*jacY(j)
template<int K, CovarianceOption C, typename JacXFunc, typename JacYFunc>
auto CheckBinaryCov(
    const Estimate<K, C>& e,
    const auto& covX,
    const auto& covY,
    const std::string& name,
    JacXFunc jacX,
    JacYFunc jacY) -> void {
    constexpr int dim{(K == Eigen::Dynamic) ? 2 : K};
    constexpr bool isFull{C == CovarianceOption::Full};
    for (auto i{0}; i < dim; ++i) {
        for (auto j{0}; j < dim; ++j) {
            double covX_ij;
            double covY_ij;
            if constexpr (isFull) {
                covX_ij = covX(i, j);
                covY_ij = covY(i, j);
            } else {
                covX_ij = (i == j) ? covX.diagonal()(i) : 0.0;
                covY_ij = (i == j) ? covY.diagonal()(i) : 0.0;
            }
            auto expectedCov{jacX(i) * covX_ij * jacX(j) + jacY(i) * covY_ij * jacY(j)};
            CheckClose(e.Covariance(i, j), expectedCov,
                       fmt::format("{}: Cov({},{})", name, i, j));
        }
    }
}

} // namespace Mustard::Testing::TestEstimate
