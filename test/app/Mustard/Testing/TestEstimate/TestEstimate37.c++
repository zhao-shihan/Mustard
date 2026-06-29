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
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Testing/TestEstimate/TestEstimate.h++"
#include "Mustard/Testing/TestEstimate/TestEstimateCommon.h++"

#include <cstdlib>
#include <stdexcept>

namespace Mustard::Testing {

TestEstimate37::TestEstimate37() :
    Subprogram{"TestEstimate37", "Test Mustard::Math::Estimate (Section 37: Concat, Head, Tail, Segment)."} {}

namespace TestEstimateSection {

using namespace Mustard::Math;
using namespace Mustard::Testing::TestEstimate;

// =========================================================================
// Helper: build full cov matrix from Estimate accessors
// =========================================================================

constexpr auto toFullCov{[]<int K, CovarianceOption C>(const Estimate<K, C>& est, int dim) {
    Eigen::MatrixXd m(dim, dim);
    if constexpr (K == 1) {
        m(0, 0) = est.Variance();
    } else {
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                m(i, j) = est.Covariance(i, j);
            }
        }
    }
    return m;
}};

// =========================================================================
// Section 37 Smoke: all call combinations compile
// =========================================================================

constexpr auto sec37Smoke{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // Concat same-type
        {
            Est e2{MakeEstimate<K, C>(v, cov)};
            [[maybe_unused]] auto c1{e.Concat(e2)};
            [[maybe_unused]] auto c2{e.Concat(Est{v, cov})};
            [[maybe_unused]] auto c3{Est{v, cov}.Concat(e2)};
        }

        // Concat cross-type
        if constexpr (C == CovarianceOption::Full) {
            using EstD = Estimate<K, CovarianceOption::Diagonal>;
            auto covD{MakeTestCov<K, CovarianceOption::Diagonal>(dim)};
            EstD eCross{v, covD};
            [[maybe_unused]] auto c1{e.Concat(eCross)};
            [[maybe_unused]] auto c2{eCross.Concat(e)};
        } else {
            using EstF = Estimate<K, CovarianceOption::Full>;
            auto covF{MakeTestCov<K, CovarianceOption::Full>(dim)};
            EstF eCross{v, covF};
            [[maybe_unused]] auto c1{e.Concat(eCross)};
            [[maybe_unused]] auto c2{eCross.Concat(e)};
        }

        // Head
        [[maybe_unused]] auto h1{e.Head(1)};
        [[maybe_unused]] auto h2{e.template Head<1>()};

        // Tail
        [[maybe_unused]] auto t1{e.Tail(1)};
        [[maybe_unused]] auto t2{e.template Tail<1>()};

        // Segment
        [[maybe_unused]] auto s1{e.Segment(0, 1)};
        [[maybe_unused]] auto s2{e.template Segment<1>(0)};
    }
}};

// =========================================================================
// Section 37a: Concat value correctness
// =========================================================================

constexpr auto sec37aConcatValue{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim + i + 1;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};

        Est e1{MakeEstimate<K, C>(v, cov)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};
        const auto result{e1.Concat(e2)};

        CheckEq(result.Dimension(), 2 * dim, "37a: Concat dimension");
        for (auto i{0}; i < dim; ++i) {
            CheckClose(result.Value(i), v(i), fmt::format("37a: Concat first half value({})", i));
            CheckClose(result.Value(dim + i), v2(i), fmt::format("37a: Concat second half value({})", i));
        }
    }
}};

// =========================================================================
// Section 37b: Concat covariance (block-diagonal)
// =========================================================================

constexpr auto sec37bConcatCov{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e1{MakeEstimate<K, C>(v, cov)};

        auto v2{MakeTestValue<K, C>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v2(i)) = dim + i + 1;
        }
        const auto cov2{MakeTestCov<K, C>(dim)};
        Est e2{MakeEstimate<K, C>(v2, cov2)};
        const auto result{e1.Concat(e2)};

        const auto fullCov1{toFullCov(e1, dim)};
        const auto fullCov2{toFullCov(e2, dim)};

        // Top-left block
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(i, j), fullCov1(i, j),
                           fmt::format("37b: top-left cov({},{})", i, j));
            }
        }

        // Bottom-right block
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(dim + i, dim + j), fullCov2(i, j),
                           fmt::format("37b: bottom-right cov({},{})", i, j));
            }
        }

        // Off-diagonal blocks should be zero
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(i, dim + j), 0.,
                           fmt::format("37b: off-diag cov({},{})", i, dim + j));
                CheckClose(result.Covariance(dim + i, j), 0.,
                           fmt::format("37b: off-diag cov({},{})", dim + i, j));
            }
        }
    }
}};

// =========================================================================
// Section 37c: Concat cross-type
// =========================================================================

auto sec37cConcatCrossType() -> void {
    constexpr int dim{3};
    using EstFF = Estimate<dim, CovarianceOption::Full>;
    using EstFD = Estimate<dim, CovarianceOption::Diagonal>;
    using EstDD = Estimate<dim, CovarianceOption::Diagonal>;

    const auto v{MakeTestValue<dim, CovarianceOption::Full>(dim)};
    auto v2{MakeTestValue<dim, CovarianceOption::Full>(dim)};
    for (auto i{0}; i < dim; ++i) {
        const_cast<double&>(v2(i)) = dim + i + 1;
    }
    const auto covFull{MakeTestCov<dim, CovarianceOption::Full>(dim)};
    const auto covDiag{MakeTestCov<dim, CovarianceOption::Diagonal>(dim)};

    // Full + Diagonal
    {
        EstFF eF{v, covFull};
        EstFD eD{v2, covDiag};
        const auto result{eF.Concat(eD)};
        CheckEq(result.Dimension(), 2 * dim, "37c: Full+Diag dim");
        const auto fullCovF{toFullCov(eF, dim)};
        const auto fullCovD{toFullCov(eD, dim)};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(i, j), fullCovF(i, j),
                           fmt::format("37c: Full+Diag top-left({},{})", i, j));
                CheckClose(result.Covariance(dim + i, dim + j), fullCovD(i, j),
                           fmt::format("37c: Full+Diag bottom-right({},{})", i, j));
            }
        }
    }

    // Diagonal + Full
    {
        EstFD eD{v, covDiag};
        EstFF eF{v2, covFull};
        const auto result{eD.Concat(eF)};
        CheckEq(result.Dimension(), 2 * dim, "37c: Diag+Full dim");
        const auto fullCovD{toFullCov(eD, dim)};
        const auto fullCovF{toFullCov(eF, dim)};
        for (auto i{0}; i < dim; ++i) {
            for (auto j{0}; j < dim; ++j) {
                CheckClose(result.Covariance(i, j), fullCovD(i, j),
                           fmt::format("37c: Diag+Full top-left({},{})", i, j));
                CheckClose(result.Covariance(dim + i, dim + j), fullCovF(i, j),
                           fmt::format("37c: Diag+Full bottom-right({},{})", i, j));
            }
        }
    }

    // Diagonal + Diagonal
    {
        EstDD eD1{v, covDiag};
        auto v3{MakeTestValue<dim, CovarianceOption::Diagonal>(dim)};
        for (auto i{0}; i < dim; ++i) {
            const_cast<double&>(v3(i)) = dim + i + 1;
        }
        EstDD eD2{v3, covDiag};
        const auto result{eD1.Concat(eD2)};
        CheckEq(result.Dimension(), 2 * dim, "37c: Diag+Diag dim");
        for (auto i{0}; i < 2 * dim; ++i) {
            for (auto j{0}; j < 2 * dim; ++j) {
                if (i != j) {
                    CheckClose(result.Covariance(i, j), 0.,
                               fmt::format("37c: Diag+Diag off-diag({},{})", i, j));
                }
            }
        }
    }
};

// =========================================================================
// Section 37d: Concat with K=1
// =========================================================================

constexpr auto sec37dConcatK1{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // Scalar concat with vector (both Full/Diag combos)
        {
            Estimate<1, C> s{1.5, 0.25};
            [[maybe_unused]] auto c1{s.Concat(e)};
            [[maybe_unused]] auto c2{e.Concat(s)};
        }

        // Scalar concat with scalar
        {
            Estimate<1, C> s1{1.5, 0.25};
            Estimate<1, C> s2{2.5, 0.36};
            const auto result{s1.Concat(s2)};
            CheckEq(result.Dimension(), 2, "37d: scalar+scalar dim");
            CheckClose(result.Value(0), 1.5, "37d: scalar+scalar value(0)");
            CheckClose(result.Value(1), 2.5, "37d: scalar+scalar value(1)");

            // Scalar+scalar covariance
            const auto fullCov1{toFullCov(s1, 1)};
            const auto fullCov2{toFullCov(s2, 1)};
            CheckClose(result.Covariance(0, 0), fullCov1(0, 0), "37d: scalar+scalar cov(0,0)");
            CheckClose(result.Covariance(1, 1), fullCov2(0, 0), "37d: scalar+scalar cov(1,1)");
            CheckClose(result.Covariance(0, 1), 0., "37d: scalar+scalar cov(0,1)");
            CheckClose(result.Covariance(1, 0), 0., "37d: scalar+scalar cov(1,0)");
        }
    }
}};

// =========================================================================
// Section 37e: Head value+cov
// =========================================================================

constexpr auto sec37eHead{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // Head(2) if dim >= 2
        if constexpr (dim >= 2) {
            const auto result{e.Head(2)};
            CheckEq(result.Dimension(), 2, "37e: Head(2) dim");
            for (auto i{0}; i < 2; ++i) {
                CheckClose(result.Value(i), v(i), fmt::format("37e: Head(2) value({})", i));
            }
            const auto fullCov{toFullCov(e, dim)};
            for (auto i{0}; i < 2; ++i) {
                for (auto j{0}; j < 2; ++j) {
                    CheckClose(result.Covariance(i, j), fullCov(i, j),
                               fmt::format("37e: Head(2) cov({},{})", i, j));
                }
            }
        }

        // Head all (Head(dim)) equals original
        {
            const auto result{e.Head(dim)};
            const auto fullCov{toFullCov(e, dim)};
            for (auto i{0}; i < dim; ++i) {
                CheckClose(result.Value(i), v(i), fmt::format("37e: Head(dim) value({})", i));
                for (auto j{0}; j < dim; ++j) {
                    CheckClose(result.Covariance(i, j), fullCov(i, j),
                               fmt::format("37e: Head(dim) cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 37f: Tail value+cov
// =========================================================================

constexpr auto sec37fTail{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // Tail(2) if dim >= 2
        if constexpr (dim >= 2) {
            const auto result{e.Tail(2)};
            CheckEq(result.Dimension(), 2, "37f: Tail(2) dim");
            for (auto i{0}; i < 2; ++i) {
                CheckClose(result.Value(i), v(dim - 2 + i), fmt::format("37f: Tail(2) value({})", i));
            }
            const auto fullCov{toFullCov(e, dim)};
            for (auto i{0}; i < 2; ++i) {
                for (auto j{0}; j < 2; ++j) {
                    CheckClose(result.Covariance(i, j), fullCov(dim - 2 + i, dim - 2 + j),
                               fmt::format("37f: Tail(2) cov({},{})", i, j));
                }
            }
        }
    }
}};

// =========================================================================
// Section 37g: Head/Tail bounds
// =========================================================================

constexpr auto sec37gBounds{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // n > dim
        {
            auto threw{false};
            try {
                e.Head(dim + 1);
            } catch (const std::out_of_range&) {
                threw = true;
            }
            if (not threw) {
                Throw<std::runtime_error>("37g: Head(n>dim) should throw out_of_range");
            }
        }

        // n <= 0
        {
            auto threw{false};
            try {
                e.Head(0);
            } catch (const std::out_of_range&) {
                threw = true;
            }
            if (not threw) {
                Throw<std::runtime_error>("37g: Head(0) should throw out_of_range");
            }
        }

        // Tail n > dim
        {
            auto threw{false};
            try {
                e.Tail(dim + 1);
            } catch (const std::out_of_range&) {
                threw = true;
            }
            if (not threw) {
                Throw<std::runtime_error>("37g: Tail(n>dim) should throw out_of_range");
            }
        }
    }
}};

// =========================================================================
// Section 37h: Segment value+cov
// =========================================================================

constexpr auto sec37hSegment{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        if constexpr (dim >= 3) {
            // Segment(1, 2) — middle two elements
            const auto result{e.Segment(1, 2)};
            CheckEq(result.Dimension(), 2, "37h: Segment(1,2) dim");
            for (auto i{0}; i < 2; ++i) {
                CheckClose(result.Value(i), v(1 + i), fmt::format("37h: Segment(1,2) value({})", i));
            }
            const auto fullCov{toFullCov(e, dim)};
            for (auto i{0}; i < 2; ++i) {
                for (auto j{0}; j < 2; ++j) {
                    CheckClose(result.Covariance(i, j), fullCov(1 + i, 1 + j),
                               fmt::format("37h: Segment(1,2) cov({},{})", i, j));
                }
            }

            // Segment(0, dim) — full copy
            {
                const auto result2{e.Segment(0, dim)};
                for (auto i{0}; i < dim; ++i) {
                    CheckClose(result2.Value(i), v(i), fmt::format("37h: Segment(0,dim) value({})", i));
                }
                for (auto i{0}; i < dim; ++i) {
                    for (auto j{0}; j < dim; ++j) {
                        CheckClose(result2.Covariance(i, j), fullCov(i, j),
                                   fmt::format("37h: Segment(0,dim) cov({},{})", i, j));
                    }
                }
            }
        }
    }
}};

// =========================================================================
// Section 37i: Segment bounds
// =========================================================================

constexpr auto sec37iSegmentBounds{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // i < 0
        {
            auto threw{false};
            try {
                e.Segment(-1, 1);
            } catch (const std::out_of_range&) {
                threw = true;
            }
            if (not threw) {
                Throw<std::runtime_error>("37i: Segment(-1,1) should throw out_of_range");
            }
        }

        // n <= 0
        {
            auto threw{false};
            try {
                e.Segment(0, 0);
            } catch (const std::out_of_range&) {
                threw = true;
            }
            if (not threw) {
                Throw<std::runtime_error>("37i: Segment(0,0) should throw out_of_range");
            }
        }

        // i + n > dim
        {
            auto threw{false};
            try {
                e.Segment(dim - 1, 2);
            } catch (const std::out_of_range&) {
                threw = true;
            }
            if (not threw) {
                Throw<std::runtime_error>("37i: Segment(i+n>dim) should throw out_of_range");
            }
        }
    }
}};

// =========================================================================
// Section 37j: Template Head<N>/Tail<N>
// =========================================================================

constexpr auto sec37jHeadTailTemplate{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        // Head<1>
        {
            const auto result{e.template Head<1>()};
            CheckEq(result.Dimension(), 1, "37j: Head<1> dim");
            const auto fullCov{toFullCov(e, dim)};
            CheckClose(result.Value(), v(0), "37j: Head<1> value");
            CheckClose(result.Variance(), fullCov(0, 0), "37j: Head<1> var");
        }

        if constexpr (dim >= 2) {
            // Tail<2>
            {
                const auto result{e.template Tail<2>()};
                CheckEq(result.Dimension(), 2, "37j: Tail<2> dim");
                for (auto i{0}; i < 2; ++i) {
                    CheckClose(result.Value(i), v(dim - 2 + i), fmt::format("37j: Tail<2> value({})", i));
                }
            }

            // Head<2>
            {
                const auto result{e.template Head<2>()};
                CheckEq(result.Dimension(), 2, "37j: Head<2> dim");
            }
        }
    }
}};

// =========================================================================
// Section 37k: Template Segment<N>
// =========================================================================

constexpr auto sec37kSegmentTemplate{[]<int K, CovarianceOption C>() {
    if constexpr (K != 1) {
        constexpr int dim{(K == Eigen::Dynamic) ? 3 : K};
        using Est = Estimate<K, C>;
        const auto v{MakeTestValue<K, C>(dim)};
        const auto cov{MakeTestCov<K, C>(dim)};
        Est e{MakeEstimate<K, C>(v, cov)};

        if constexpr (dim >= 3) {
            // Segment<2>(1)
            {
                const auto result{e.template Segment<2>(1)};
                CheckEq(result.Dimension(), 2, "37k: Segment<2>(1) dim");
                for (auto i{0}; i < 2; ++i) {
                    CheckClose(result.Value(i), v(1 + i), fmt::format("37k: Segment<2>(1) value({})", i));
                }
                const auto fullCov{toFullCov(e, dim)};
                for (auto i{0}; i < 2; ++i) {
                    for (auto j{0}; j < 2; ++j) {
                        CheckClose(result.Covariance(i, j), fullCov(1 + i, 1 + j),
                                   fmt::format("37k: Segment<2>(1) cov({},{})", i, j));
                    }
                }
            }
        }

        // Segment<dim>(0)
        {
            const auto result{e.template Segment<dim>(0)};
            CheckEq(result.Dimension(), dim, "37k: Segment<dim>(0) dim");
        }
    }
}};

// =========================================================================
// Section 37l: Dynamic dimension
// =========================================================================

auto sec37lDynamic() -> void {
    constexpr int dim{3};
    using Est = Estimate<Eigen::Dynamic, CovarianceOption::Full>;

    const auto v{MakeTestValue<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    const auto cov{MakeTestCov<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    Est e{MakeEstimate<Eigen::Dynamic, CovarianceOption::Full>(v, cov)};

    auto v2{MakeTestValue<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    for (auto i{0}; i < dim; ++i) {
        const_cast<double&>(v2(i)) = dim + i + 1;
    }
    const auto cov2{MakeTestCov<Eigen::Dynamic, CovarianceOption::Full>(dim)};
    Est e2{MakeEstimate<Eigen::Dynamic, CovarianceOption::Full>(v2, cov2)};

    // Concat dynamic
    {
        const auto result{e.Concat(e2)};
        CheckEq(result.Dimension(), 2 * dim, "37l: Concat dynamic dim");
        for (auto i{0}; i < dim; ++i) {
            CheckClose(result.Value(i), v(i), fmt::format("37l: Concat value({})", i));
            CheckClose(result.Value(dim + i), v2(i), fmt::format("37l: Concat value({})", dim + i));
        }
    }

    // Head dynamic
    {
        const auto h{e.Head(2)};
        CheckEq(h.Dimension(), 2, "37l: Head dynamic dim");
        CheckClose(h.Value(0), v(0), "37l: Head dynamic value(0)");
        CheckClose(h.Value(1), v(1), "37l: Head dynamic value(1)");
    }

    // Tail dynamic
    {
        const auto t{e.Tail(2)};
        CheckEq(t.Dimension(), 2, "37l: Tail dynamic dim");
        CheckClose(t.Value(0), v(1), "37l: Tail dynamic value(0)");
        CheckClose(t.Value(1), v(2), "37l: Tail dynamic value(1)");
    }

    // Segment dynamic
    {
        const auto s{e.Segment(1, 2)};
        CheckEq(s.Dimension(), 2, "37l: Segment dynamic dim");
        CheckClose(s.Value(0), v(1), "37l: Segment dynamic value(0)");
        CheckClose(s.Value(1), v(2), "37l: Segment dynamic value(1)");
    }

    // Dynamic Head/N> — should throw if N > dim
    {
        auto threw{false};
        try {
            e.Head<5>();
        } catch (const std::out_of_range&) {
            threw = true;
        }
        if (not threw) {
            Throw<std::runtime_error>("37l: dynamic Head<5> should throw out_of_range");
        }
    }
};

} // namespace TestEstimateSection

auto TestEstimate37::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace TestEstimateSection;

    PrintLn("--- Section 37: Smoke Test ---");
    RunOverAllDims<AllStaticDims>(sec37Smoke);
    PrintLn("  37 smoke passed: all call combinations (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37a: Concat Value ---");
    RunOverAllDims<AllStaticDims>(sec37aConcatValue);
    PrintLn("  37a passed: Concat value (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37b: Concat Covariance ---");
    RunOverAllDims<AllStaticDims>(sec37bConcatCov);
    PrintLn("  37b passed: Concat block-diagonal covariance (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37c: Concat Cross-Type ---");
    sec37cConcatCrossType();
    PrintLn("  37c passed: Concat Full/Diag combinations");

    PrintLn("--- Section 37d: Concat with K=1 ---");
    RunOverAllDims<AllStaticDims>(sec37dConcatK1);
    PrintLn("  37d passed: Concat scalar + vector (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37e: Head ---");
    RunOverAllDims<AllStaticDims>(sec37eHead);
    PrintLn("  37e passed: Head value and covariance (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37f: Tail ---");
    RunOverAllDims<AllStaticDims>(sec37fTail);
    PrintLn("  37f passed: Tail value and covariance (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37g: Head/Tail Bounds ---");
    RunOverAllDims<AllStaticDims>(sec37gBounds);
    PrintLn("  37g passed: Head/Tail bounds checking (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37h: Segment ---");
    RunOverAllDims<AllStaticDims>(sec37hSegment);
    PrintLn("  37h passed: Segment value and covariance (K=3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37i: Segment Bounds ---");
    RunOverAllDims<AllStaticDims>(sec37iSegmentBounds);
    PrintLn("  37i passed: Segment bounds checking (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37j: Template Head/N>/Tail<N> ---");
    RunOverAllDims<AllStaticDims>(sec37jHeadTailTemplate);
    PrintLn("  37j passed: Static Head<T>/Tail<T> (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37k: Template Segment<N> ---");
    RunOverAllDims<AllStaticDims>(sec37kSegmentTemplate);
    PrintLn("  37k passed: Static Segment<T> (K=2,3,5,10 Full/Diag + dynamic)");

    PrintLn("--- Section 37l: Dynamic Dimension ---");
    sec37lDynamic();
    PrintLn("  37l passed: Concat/Head/Tail/Segment on Dynamic estimates");

    PrintLn("All TestEstimate37 tests passed.");
    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
