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
RAMBO<M, N>::RAMBO(const std::array<int, N>& pdgID, const std::array<double, N>& mass) :
    VersatileEventGenerator<M, N, 4 * N>{pdgID, mass},
    fWeightFactor{} {
    // initialization step: factorials for the phase space weight
    constexpr double po2log = 0.45158270528945486; // log(twopi / 4.);
    fWeightFactor[1] = po2log;
    for (int k = 2; k < N; k++) {
        fWeightFactor[k] = fWeightFactor[k - 1] + po2log - 2. * std::log(double(k - 1));
    }
    for (int k = 2; k < N; k++) {
        fWeightFactor[k] = (fWeightFactor[k] - std::log(double(k)));
    }
}

template<int M, int N>
    requires(N >= 2)
auto RAMBO<M, N>::operator()(const RandomState& u, InitialStateMomenta pI) -> Event {
    const auto cmE{this->CalculateCMEnergy(pI)};
    this->CheckCMEnergy(cmE);
    const auto beta{this->BoostToCMFrame(pI)};

    /**********************************************************************
     *                       rambo                                         *
     *    ra(ndom)  m(omenta)  b(eautifully)  o(rganized)                  *
     *                                                                     *
     *    a democratic multi-particle phase space generator                *
     *    authors:  s.d. ellis,  r. kleiss,  w.j. stirling                 *
     *    this is version 1.0 -  written by r. kleiss                      *
     *                                                                     *
     *    N    = number of particles                                       *
     *    cmE = total centre-of-mass energy                               *
     *    xm   = particle masses ( dim=nexternal-nincoming )               *
     *    p    = particle momenta ( dim=(4,nexternal-nincoming) )          *
     *    wt   = weight of the event                                       *
     ***********************************************************************/
    // Copyright (c) 2009, 2013, the MadTeam.  All rights reserved.

    const auto& xm{this->fMass};

    double wt;
    std::array<std::array<double, 4>, N> q{};
    std::array<std::array<double, 4>, N> p{};
    std::array<double, 4> r{};
    std::array<double, 3> b{};
    std::array<double, N> p2{};
    std::array<double, N> xm2{};
    std::array<double, N> e{};
    std::array<double, N> v{};
    constexpr double acc = 1e-14;
    constexpr int itmax = 6;
    constexpr double twopi = 6.2831853071795865;
    constexpr double po2log = 0.45158270528945486; // log(twopi / 4.);

    const auto Result{[&] {
        Event event{.weight = std::exp(wt), .pdgID = this->fPDGID, .p = {}};
        std::ranges::transform(p, event.p.begin(), [](auto&& p) {
            return CLHEP::HepLorentzVector{p[1], p[2], p[3], p[0]};
        });
        this->BoostToLabFrame(beta, event.p);
        return event;
    }};

    // count nonzero masses
    double xmt = 0.;
    int nm = 0;
    for (int i = 0; i < N; i++) {
        if (xm[i] != 0.) {
            nm = nm + 1;
        }
        xmt = xmt + std::abs(xm[i]);
    }

    // generate N massless momenta in infinite phase space
    for (int i = 0; i < N; i++) {
        double r1 = u[4 * i];
        double c = 2. * r1 - 1.;
        double s = std::sqrt(1. - c * c);
        double f = twopi * u[4 * i + 1];
        r1 = u[4 * i + 2];
        double r2 = u[4 * i + 3];
        q[i][0] = -std::log(r1 * r2);
        q[i][3] = q[i][0] * c;
        q[i][2] = q[i][0] * s * std::cos(f);
        q[i][1] = q[i][0] * s * std::sin(f);
    }
    // calculate the parameters of the conformal transformation
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < 4; k++) {
            r[k] = r[k] + q[i][k];
        }
    }
    double rmas = std::sqrt(muc::pow(r[0], 2) - muc::pow(r[3], 2) - muc::pow(r[2], 2) - muc::pow(r[1], 2));
    for (int k = 1; k < 4; k++) {
        b[k - 1] = -r[k] / rmas;
    }
    double g = r[0] / rmas;
    double a = 1. / (1. + g);
    double x = cmE / rmas;

    // transform the q's conformally into the p's
    for (int i = 0; i < N; i++) {
        double bq = b[0] * q[i][1] + b[1] * q[i][2] + b[2] * q[i][3];
        for (int k = 1; k < 4; k++) {
            p[i][k] = x * (q[i][k] + b[k - 1] * (q[i][0] + a * bq));
        }
        p[i][0] = x * (g * q[i][0] + bq);
    }

    // calculate weight
    wt = po2log;
    if (N != 2) {
        wt = (2. * N - 4.) * std::log(cmE) + fWeightFactor[N - 1];
    }

    // return for weighted massless momenta
    if (nm == 0) {
        return Result();
    }

    // massive particles: rescale the momenta by a factor x
    double xmax = std::sqrt(1. - muc::pow(xmt / cmE, 2));
    for (int i = 0; i < N; i++) {
        xm2[i] = muc::pow(xm[i], 2);
        p2[i] = muc::pow(p[i][0], 2);
    }
    int iter = 0;
    x = xmax;
    double accu = cmE * acc;
    while (true) {
        double f0 = -cmE;
        double g0 = 0.;
        double x2 = x * x;
        for (int i = 0; i < N; i++) {
            e[i] = std::sqrt(xm2[i] + x2 * p2[i]);
            f0 = f0 + e[i];
            g0 = g0 + p2[i] / e[i];
        }
        if (std::abs(f0) <= accu) {
            break;
        }
        iter = iter + 1;
        if (iter > itmax) [[unlikely]] {
            PrintWarning("Momentum scale not converged");
            break;
        }
        x = x - f0 / (x * g0);
    }
    for (int i = 0; i < N; i++) {
        v[i] = x * p[i][0];
        for (int k = 1; k < 4; k++) {
            p[i][k] = x * p[i][k];
        }
        p[i][0] = e[i];
    }

    // calculate the mass-effect weight factor
    double wt2 = 1.;
    double wt3 = 0.;
    for (int i = 0; i < N; i++) {
        wt2 = wt2 * v[i] / e[i];
        wt3 = wt3 + muc::pow(v[i], 2) / e[i];
    }
    double wtm = (2. * N - 3.) * std::log(x) + std::log(wt2 / wt3 * cmE);

    // return for  weighted massive momenta
    wt = wt + wtm;

    return Result();
}

} // namespace Mustard::inline Physics::inline Generator
