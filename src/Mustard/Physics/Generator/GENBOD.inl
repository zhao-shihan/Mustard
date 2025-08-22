// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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
auto GENBOD<M, N>::operator()(const RandomState& u, InitialStateMomenta pI) -> Event {
    const auto cmsE{this->CalculateCMSEnergy(pI)};
    this->CheckCMSEnergy(cmsE);
    const auto beta{this->BoostToCMS(pI)};

    auto random{u.cbegin()};
    std::array<double, N> u0;
    u0.front() = 0;
    random = std::ranges::copy_n(random, N - 2, u0.begin() + 1).in;
    u0.back() = 1;
    [](double arr[]) {
        for (auto i{1}; i < N - 2; ++i) {
            const auto key{arr[i]};
            auto j{i - 1};
            for (; j >= 0 and arr[j] > key; --j) {
                arr[j + 1] = arr[j];
            }
            arr[j + 1] = key;
        }
    }(u0.data() + 1);

    std::array<double, N> invMass;
    double sumMass{};
    const auto cmsEk{cmsE - this->fSumMass};
    for (int i{}; i < N; ++i) {
        sumMass += this->fMass[i];
        invMass[i] = u0[i] * cmsEk + sumMass;
    }

    Event event{.weight = 1, .pdgID = this->fPDGID, .p = {}};
    std::array<double, N> pRel;
    for (int i{}; i < N - 1; ++i) {
        constexpr auto RelativeMomentum{[](double m12, double m1, double m2) {
            return std::sqrt((m12 - m1 - m2) * (m12 + m1 + m2) * (m12 - m1 + m2) * (m12 + m1 - m2)) / (2 * m12);
        }};
        pRel[i] = RelativeMomentum(invMass[i + 1], invMass[i], this->fMass[i + 1]);
        event.weight *= pRel[i];
    }

    // clang-format off
    event.p[0] = {{0, pRel[0], 0}, muc::hypot(pRel[0], this->fMass[0])};          // clang-format on
    for (int i{1};; ++i) { // clang-format off
        event.p[i] = {{0, -pRel[i - 1], 0}, muc::hypot(pRel[i - 1], this->fMass[i])}; // clang-format on

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

    this->BoostToOriginalFrame(beta, event.p);
    return event;
}

} // namespace Mustard::inline Physics::inline Generator
