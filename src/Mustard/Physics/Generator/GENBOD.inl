// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

namespace Mustard::inline Physics::inline Generator {

template<int M, int N>
    requires(N >= 2)
MUSTARD_OPTIMIZE_FAST auto GENBOD<M, N>::operator()(const RandomState& u, InitialStateMomenta pI) -> Event {
    const auto cmE{this->CalculateCMEnergy(pI)};
    this->CheckCMEnergy(cmE);
    const auto beta{this->BoostToCMFrame(pI)};

    auto random{u.cbegin()};
    std::array<double, N - 2> u0;
    random = std::ranges::copy_n(random, u0.size(), u0.begin()).in;
    for (auto i{1}; i < ssize(u0); ++i) {
        const auto key{u0[i]};
        auto j{i - 1};
        for (; j >= 0 and u0[j] > key; --j) {
            u0[j + 1] = u0[j];
        }
        u0[j + 1] = key;
    }

    std::array<double, N> invMass{this->fMass.front()};
    const auto cmEk{cmE - this->fSumMass};
    auto sumMass{this->fMass.front()};
    for (auto i{1}; i < N - 1; ++i) {
        sumMass += this->fMass[i];
        invMass[i] = u0[i - 1] * cmEk + sumMass;
    }
    invMass.back() = cmE;

    using Mustard::MathConstant::pi;
    Event event{.weight = muc::pow(cmE, N - 3) / (4 * muc::pow(2 * pi, 2 * N + 2)), .pdgID = this->fPDGID, .p = {}};
    std::array<double, N> pRel;
    for (int i{}; i < N - 1; ++i) {
        constexpr auto RelativeMomentum{[](double m12, double m1, double m2) {
            return std::sqrt((m12 - m1 - m2) * (m12 + m1 + m2) * (m12 - m1 + m2) * (m12 + m1 - m2)) / (2 * m12);
        }};
        pRel[i] = RelativeMomentum(invMass[i + 1], invMass[i], this->fMass[i + 1]);
        event.weight *= pRel[i];
    }

    // clang-format off
    event.p[0] = {muc::hypot(pRel[0], this->fMass[0]), {0, pRel[0], 0}};          // clang-format on
    for (int i{1};; ++i) { // clang-format off
        event.p[i] = {muc::hypot(pRel[i - 1], this->fMass[i]), {0, -pRel[i - 1], 0}}; // clang-format on

        const auto cZ{2 * (*random++) - 1};
        const auto sZ{std::sqrt(1 - muc::pow(cZ, 2))};
        const auto phiY{CLHEP::twopi * (*random++)};
        const auto cY{std::cos(phiY)};
        const auto sY{std::sin(phiY)};
        for (int j{}; j <= i; ++j) {
            auto& p{event.p[j]};
            auto x{p.x()};
            const auto y{p.y()};
            p.setPx(cZ * x - sZ * y);
            p.setPy(sZ * x + cZ * y); // rotation around Z
            x = p.x();
            const auto z{p.z()};
            p.setPx(cY * x - sY * z);
            p.setPz(sY * x + cY * z); // rotation around Y
        }

        if (i == N - 1) {
            break;
        }

        const auto beta{pRel[i] / muc::hypot(pRel[i], invMass[i])};
        for (int j{}; j <= i; ++j) {
            event.p[j].boost(0, beta, 0);
        }
    }

    // Ensure the random state is exactly exhausted
    Ensures(random == u.cend());

    this->BoostToLabFrame(beta, event.p);
    return event;
}

} // namespace Mustard::inline Physics::inline Generator
