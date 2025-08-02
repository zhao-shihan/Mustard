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

template<int N>
constexpr RAMBO<N>::RAMBO(double eCM, const std::array<double, N>& mass) :
    EventGenerator<N, 4 * N>{eCM, mass},
    fAllMassAreTiny{std::ranges::all_of(mass, [&](auto m) {
        return muc::pow<2>(m / eCM) < muc::default_tolerance<double>;
    })} {}

template<int N>
auto RAMBO<N>::operator()(const RandomState& u) const -> Event {
    // call the massless genPoint, initializing weight
    auto [p, weight]{[&] {
        std::array<std::array<double, 4>, N> p;
        std::array<double, 4> R{};

        for (int i = 0; i < N; i++) {
            const auto c{2 * u[4 * i] - 1};
            const auto s{std::sqrt(1 - muc::pow<2>(c))};
            const auto f{CLHEP::twopi * u[4 * i + 1]};
            const auto r12{u[4 * i + 2] * u[4 * i + 3]};
            const auto En{-std::log(std::max(std::numeric_limits<double>::min(), r12))};
            p[i][0] = En;
            p[i][1] = En * s * std::sin(f);
            p[i][2] = En * s * std::cos(f);
            p[i][3] = En * c;
            for (auto j{0}; j < 4; j++) {
                R[j] += p[i][j];
            }
        }
        const auto Rmass{std::sqrt(muc::pow<2>(R[0]) - muc::hypot_sq(R[1], R[2], R[3]))};
        for (auto j{0}; j < 4; j++) {
            R[j] /= -Rmass;
        }
        const auto a{1 / (1 - R[0])};
        const auto x{this->fECM / Rmass};
        for (int i = 0; i < N; i++) {
            double bq = R[1] * p[i][1] + R[2] * p[i][2] + R[3] * p[i][3];
            for (int j = 1; j < 4; j++) {
                p[i][j] = x * (p[i][j] + R[j] * (p[i][0] + a * bq));
            }
            p[i][0] = x * (-R[0] * p[i][0] + bq);
        }

        return std::pair{p, 1.};
    }()};

    const auto State{
        [&p = p] {
            std::array<CLHEP::HepLorentzVector, N> state;
            std::ranges::transform(p, state.begin(),
                                   [](const auto& q) -> CLHEP::HepLorentzVector {
                                       return {q[1], q[2], q[3], q[0]};
                                   });
            return state;
        }};

    // if none of the reduced masses is > tolerance, return
    if (fAllMassAreTiny) {
        return {weight, State()};
    }

    // rescale all the momenta
    const auto [xi, xiConverged]{muc::find_root::zbrent(
        [&, &p = p](double xi) {
            double sum{};
            for (int i{}; i < N; i++) {
                sum += muc::hypot(this->fMass[i], xi * p[i][0]);
            }
            return sum - this->fECM;
        },
        0., 1.)};
    if (not xiConverged) [[unlikely]] {
        PrintWarning(fmt::format("Momentum scale (xi = {}) not converged", xi));
    }
    for (auto iMom{0}; iMom < N; iMom++) {
        p[iMom][0] = muc::hypot(this->fMass[iMom], xi * p[iMom][0]);
        p[iMom][1] *= xi;
        p[iMom][2] *= xi;
        p[iMom][3] *= xi;
    }
    // calculate the quantities needed for the calculation of the weight
    double sumpnorm{};
    double prodpnormdivE{1};
    double sumpnormsquadivE{};
    for (auto iMom{0}; iMom < N; iMom++) {
        auto pnormsqua{muc::hypot_sq(p[iMom][1], p[iMom][2], p[iMom][3])};
        auto pnorm{std::sqrt(pnormsqua)};
        sumpnorm += pnorm;
        prodpnormdivE *= pnorm / p[iMom][0];
        sumpnormsquadivE += pnormsqua / p[iMom][0];
    }
    // There's a typo in eq. 4.11 of the Rambo paper by Kleiss,
    // Stirling and Ellis, the Ecm below is not present there
    weight *= muc::pow<2 * N - 3>(sumpnorm / this->fECM) * prodpnormdivE * this->fECM / sumpnormsquadivE;

    return {weight, State()};
}

} // namespace Mustard::inline Physics::inline Generator
