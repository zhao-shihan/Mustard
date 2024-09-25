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

namespace Mustard::inline Extension::CLHEPX {

template<int N>
    requires(N >= 2)
constexpr RAMBO<N>::RAMBO(double eCM, std::array<double, N> mass) :
    fECM{eCM},
    fMass{std::move(mass)},
    fAllMassAreTiny{std::ranges::all_of(fMass, [&](auto m) { return muc::pow<2>(m / fECM) < fgTiny; })} {
    if (eCM <= muc::ranges::reduce(fMass)) {
        throw std::domain_error{PrettyException("No enough energy")};
    }
}

template<int N>
    requires(N >= 2)
auto RAMBO<N>::operator()(const std::array<double, 4 * N>& u) const -> Event {
    // call the massless genPoint, initializing weight
    auto [p, weight]{[&] {
        std::array<std::array<double, 4>, N> p;
        std::array<double, 4> R{};

        for (int i = 0; i < N; i++) {
            const auto c{2 * u[4 * i] - 1};
            const auto s{std::sqrt(1 - muc::pow<2>(c))};
            const auto f{CLHEP::twopi * u[4 * i + 1]};
            const auto r12{u[4 * i + 2] * u[4 * i + 3]};
            const auto En{r12 > 0 ? -std::log(r12) : 747}; // -log(1e-323)
            p[i][0] = En;
            p[i][1] = En * s * std::sin(f);
            p[i][2] = En * s * std::cos(f);
            p[i][3] = En * c;
            for (auto j{0}; j < 4; j++) {
                R[j] += p[i][j];
            }
        }
        const auto Rmass{std::sqrt(muc::pow<2>(R[0]) - muc::hypot2(R[1], R[2], R[3]))};
        for (auto j{0}; j < 4; j++) {
            R[j] /= -Rmass;
        }
        const auto a{1 / (1 - R[0])};
        const auto x{fECM / Rmass};
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

    // if none of the reduced masses is > fgTiny, return
    if (fAllMassAreTiny) { return {weight, State()}; }

    // rescale all the momenta
    const auto [xi, xiConverged]{muc::find_root::zbrent(
        [&, &p = p](double xi) {
            double sum{};
            for (int i{}; i < N; i++) {
                sum += muc::hypot(fMass[i], xi * p[i][0]);
            }
            return sum - fECM;
        },
        0., 1.)};
    if (not xiConverged) [[unlikely]] {
        Env::PrintPrettyWarning(fmt::format("Momentum scale (xi = {}) not converged", xi));
    }
    for (auto iMom{0}; iMom < N; iMom++) {
        p[iMom][0] = muc::hypot(fMass[iMom], xi * p[iMom][0]);
        p[iMom][1] *= xi;
        p[iMom][2] *= xi;
        p[iMom][3] *= xi;
    }
    // calculate the quantities needed for the calculation of the weight
    double sumpnorm{};
    double prodpnormdivE{1};
    double sumpnormsquadivE{};
    for (auto iMom{0}; iMom < N; iMom++) {
        auto pnormsqua{muc::hypot2(p[iMom][1], p[iMom][2], p[iMom][3])};
        auto pnorm{std::sqrt(pnormsqua)};
        sumpnorm += pnorm;
        prodpnormdivE *= pnorm / p[iMom][0];
        sumpnormsquadivE += pnormsqua / p[iMom][0];
    }
    // There's a typo in eq. 4.11 of the Rambo paper by Kleiss,
    // Stirling and Ellis, the Ecm below is not present there
    weight *= muc::pow<2 * N - 3>(sumpnorm / fECM) * prodpnormdivE * fECM / sumpnormsquadivE;

    return {weight, State()};
}

template<int N>
    requires(N >= 2)
auto RAMBO<N>::operator()(CLHEP::HepRandomEngine& rng) const -> Event {
    std::array<double, 4 * N> u;
    rng.flatArray(4 * N, u.data());
    return (*this)(u);
}

template<int N>
    requires(N >= 2)
auto RAMBO<N>::operator()(const std::array<double, 4 * N>& u, const CLHEP::Hep3Vector& beta) const -> Event {
    auto [weight, state] = (*this)(u);
    for (auto&& p : state) {
        p.boost(beta);
    }
    return {weight, state};
}

template<int N>
    requires(N >= 2)
auto RAMBO<N>::operator()(CLHEP::HepRandomEngine& rng, const CLHEP::Hep3Vector& beta) const -> Event {
    std::array<double, 4 * N> u;
    rng.flatArray(4 * N, u.data());
    return (*this)(u, beta);
}

} // namespace Mustard::inline Extension::CLHEPX
