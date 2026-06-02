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

namespace Mustard::Detector::Field {

template<muc::general_arithmetic T,
         std::regular_invocable<Point3D> AProjection,
         std::regular_invocable<Point3D, T> ATransformation>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
                 std::convertible_to<std::invoke_result_t<ATransformation, Point3D, T>, T>
FieldMap3D<T, AProjection, ATransformation>::FieldMap3D(std::string_view dataName, std::string_view fileName,
                                                        AProjection projection, ATransformation transformation,
                                                        muc::tolerance<float> tol, bool enableIMT) :
    fProjection{std::move(projection)},
    fTransformation{std::move(transformation)},
    fGridInfo{},
    fCachedProjPos{std::numeric_limits<double>::quiet_NaN()},
    fCachedInterpolatedValue{} {
    gtl::vector<GridValueType> fieldGrid;
    if (not mplr::available() or mplr::comm_world().rank() == 0) {
        // Take grid and field columns from RDataFrame.
        auto gridData{[&] {
            const auto readData{[&] {
                ROOT::RDataFrame rdf{dataName, fileName};
                return *rdf.Define("grid_field",
                                   [](const muc::array3f& grid, const GridValueType& field) {
                                       return std::pair{grid, field};
                                   },
                                   {"grid", "field"})
                            .template Take<std::pair<muc::array3f, GridValueType>>("grid_field");
            }};
            if (enableIMT) {
                ROOTX::ScopedIMT imt;
                return readData();
            } else {
                return readData();
            }
        }()};
        if (gridData.empty()) {
            Throw<std::runtime_error>("RDataFrame contains no grid points");
        }

        // Sort by grid coordinate (lexicographic: x, then y, then z) and check duplicate points.
        muc::timsort(gridData);
        const auto dup{std::ranges::adjacent_find(gridData, [](const auto& a, const auto& b) {
            return a.first == b.first;
        })};
        if (dup != gridData.cend()) {
            Throw<std::runtime_error>(fmt::format("Point [{}, {}, {}] appeared twice",
                                                  dup->first[0], dup->first[1], dup->first[2]));
        }

        // Validate grid regularity and extract grid data
        constexpr auto lowestFloat{std::numeric_limits<float>::lowest()};
        muc::array3f x0{lowestFloat, lowestFloat, lowestFloat};
        muc::array3f lastDelta{};
        muc::array3i counter{};
        muc::array3i countChecker{};
        fieldGrid.reserve(gridData.size());
        for (auto&& [x, field] : std::as_const(gridData)) {
            for (int i{}; i < 3; ++counter[i++]) {
                if (x[i] <= x0[i]) {
                    continue; // not a new point along this axis, skip
                }
                // check normal delta
                if (fGridInfo[i].n >= 2) {
                    if (not muc::isclose(x[i] - x0[i], lastDelta[i], tol)) {
                        Throw<std::runtime_error>("Irregular grid (inconsistent delta)");
                    }
                }
                lastDelta[i] = x[i] - x0[i];
                // check normal switch count
                if (fGridInfo[i].n <= 1) {
                    countChecker[i] = counter[i];
                } else if (counter[i] != countChecker[i]) {
                    Throw<std::runtime_error>(fmt::format("Irregular grid (inconsistent n points per row along axis {}: expected {}, got {})",
                                                          i, countChecker[i], counter[i]));
                }
                counter[i] = 0;
                // update
                ++fGridInfo[i].n;
                x0[i] = x[i];
            }
            fieldGrid.emplace_back(field);
        }
        if (fieldGrid.size() != static_cast<std::size_t>(fGridInfo[0].n) *
                                    static_cast<std::size_t>(fGridInfo[1].n) *
                                    static_cast<std::size_t>(fGridInfo[2].n)) {
            Throw<std::runtime_error>("Irregular grid (N != Nx * Ny * Nz)");
        }
        for (int i{}; i < 3; ++i) {
            fGridInfo[i].min = gridData.front().first[i];
            fGridInfo[i].max = gridData.back().first[i];
        }
        for (auto&& grid : fGridInfo) {
            if (grid.n < 2) {
                Throw<std::runtime_error>("Too few grid points (should >= 2 in each direction)");
            }
            if (grid.n >= 1 / tol.rel) {
                Throw<std::runtime_error>(fmt::format("Too much grid points (in each direction should < 1 / tol.rel, tol.rel == {})", tol.rel));
            }
            if (grid.n >= std::numeric_limits<int>::max()) {
                Throw<std::runtime_error>("Too much grid points (in each direction should < INT_MAX)");
            }
            grid.delta = (grid.max - grid.min) / (grid.n - 1);
        }
    }

    // Broadcast fGridInfo: split into int[3] and double[9] then broadcast.
    if (mplr::available() and mplr::comm_world().size() > 1) {
        std::array gridN{fGridInfo[0].n, fGridInfo[1].n, fGridInfo[2].n};
        std::array gridD{fGridInfo[0].min, fGridInfo[0].max, fGridInfo[0].delta,
                         fGridInfo[1].min, fGridInfo[1].max, fGridInfo[1].delta,
                         fGridInfo[2].min, fGridInfo[2].max, fGridInfo[2].delta};
        mplr::irequest_pool bcast;
        bcast.push(mplr::comm_world().ibcast(0, gridN.data(), mplr::vector_layout<long long>{3}));
        bcast.push(mplr::comm_world().ibcast(0, gridD.data(), mplr::vector_layout<double>{9}));
        bcast.waitall(mplr::duty_ratio::preset::relaxed); // Mostly wait for process 0 to read data from RDF
        for (int i{}; i < 3; ++i) {
            fGridInfo[i] = {gridN[i], gridD[3 * i], gridD[3 * i + 1], gridD[3 * i + 2]};
        }
    }

    // Construct shared-memory array. BcastTag handles single-node, multi-node,
    // and non-MPI cases internally.
    fFieldGrid = Parallel::ShmArray<GridValueType>{Parallel::BcastTag{}, std::move(fieldGrid)};
}

template<muc::general_arithmetic T,
         std::regular_invocable<Point3D> AProjection,
         std::regular_invocable<Point3D, T> ATransformation>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
             std::convertible_to<std::invoke_result_t<ATransformation, Point3D, T>, T>
auto FieldMap3D<T, AProjection, ATransformation>::At(Point3D position) const -> T {
    const auto projPos{std::invoke(fProjection, position)};
    if (projPos.x() < fGridInfo[0].min or fGridInfo[0].max < projPos.x() or
        projPos.y() < fGridInfo[1].min or fGridInfo[1].max < projPos.y() or
        projPos.z() < fGridInfo[2].min or fGridInfo[2].max < projPos.z()) {
        return ZeroVector<T>();
    }
    constexpr auto floatIsClose{[](float a, float b) { return muc::isclose(a, b); }};
    if (not floatIsClose(projPos.x(), fCachedProjPos.x()) or
        not floatIsClose(projPos.y(), fCachedProjPos.y()) or
        not floatIsClose(projPos.z(), fCachedProjPos.z())) {
        constexpr auto Decompose{
            [](double coord, const GridInfo& grid) -> std::pair<int, double> {
                const auto u{(coord - grid.min) / grid.delta};
                auto i{static_cast<int>(u)};
                assert(0 <= i), assert(i <= grid.n - 1);
                if (i == grid.n - 1) {
                    --i;
                }
                return {i, u - i};
            }};
        const auto [i, u]{Decompose(projPos.x(), fGridInfo[0])};
        const auto [j, v]{Decompose(projPos.y(), fGridInfo[1])};
        const auto [k, w]{Decompose(projPos.z(), fGridInfo[2])};
        fCachedProjPos = projPos;
        fCachedInterpolatedValue = muc::trilerp( // clang-format off
            FieldGrid(i,     j,     k    ),
            FieldGrid(i,     j,     k + 1),
            FieldGrid(i,     j + 1, k    ),
            FieldGrid(i,     j + 1, k + 1),
            FieldGrid(i + 1, j,     k    ),
            FieldGrid(i + 1, j,     k + 1),
            FieldGrid(i + 1, j + 1, k    ),
            FieldGrid(i + 1, j + 1, k + 1),
            u, v, w); // clang-format on
    }
    return std::invoke(fTransformation, position, fCachedInterpolatedValue);
}

template<muc::general_arithmetic T,
         std::regular_invocable<Point3D> AProjection,
         std::regular_invocable<Point3D, T> ATransformation>
    requires std::convertible_to<std::invoke_result_t<AProjection, Point3D>, Point3D> and
             std::convertible_to<std::invoke_result_t<ATransformation, Point3D, T>, T>
auto FieldMap3D<T, AProjection, ATransformation>::FieldGrid(int i, int j, int k) const -> T {
    assert(i < fGridInfo[0].n);
    assert(j < fGridInfo[1].n);
    assert(k < fGridInfo[2].n);
    const auto idx{(i * fGridInfo[1].n + j) * fGridInfo[2].n + k};
    if constexpr (std::is_floating_point_v<T>) {
        return static_cast<T>(fFieldGrid[idx]);
    } else {
        return VectorCast<T>(fFieldGrid[idx]);
    }
}

} // namespace Mustard::Detector::Field
