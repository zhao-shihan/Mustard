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

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<double> delta, std::optional<unsigned> discard) :
    Base{pI, pdgID, mass},
    fIdenticalSet{},
    fMCMCDelta{fgDefaultInvalidMCMCDelta},
    fMCMCDiscard{fgDefaultInvalidMCMCDiscard},
    fBurntIn{},
    fMarkovChain{} {
    if (delta) {
        MCMCDelta(*delta);
    }
    if (discard) {
        MCMCDiscard(*discard);
    }
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, CLHEP::Hep3Vector polarization,
                                      const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<double> delta, std::optional<unsigned> discard) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> : // clang-format on
    MCMCGenerator{pI, pdgID, mass, std::move(delta), std::move(discard)} {
    this->InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
MCMCGenerator<M, N, A>::MCMCGenerator(const InitialStateMomenta& pI, const std::array<CLHEP::Hep3Vector, M>& polarization,
                                      const std::array<int, N>& pdgID, const std::array<double, N>& mass,
                                      std::optional<double> delta, std::optional<unsigned> discard) // clang-format off
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) : // clang-format on
    MCMCGenerator{pI, pdgID, mass, std::move(delta), std::move(discard)} {
    this->InitialStatePolarization(polarization);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization() const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    return Base::InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(int i) const -> CLHEP::Hep3Vector
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return Base::InitialStatePolarization(i);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization() const -> const std::array<CLHEP::Hep3Vector, M>&
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    return Base::InitialStatePolarization();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<1, N>> {
    if (not pol.isNear(InitialStatePolarization(), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    Base::InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(int i, CLHEP::Hep3Vector pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    if (not pol.isNear(InitialStatePolarization(i), muc::default_tolerance<double>)) {
        BurnInRequired();
    }
    Base::InitialStatePolarization(i, pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::InitialStatePolarization(const std::array<CLHEP::Hep3Vector, M>& pol) -> void
    requires std::derived_from<A, QFT::PolarizedMatrixElement<M, N>> and (M > 1) {
    if (not std::ranges::equal(pol, InitialStatePolarization(),
                               [](auto&& a, auto&& b) { return a.isNear(b, muc::default_tolerance<double>); })) {
        BurnInRequired();
    }
    Base::InitialStatePolarization(pol);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Acceptance(AcceptanceFunction Acceptance) -> void {
    Base::Acceptance(Acceptance);
    BurnInRequired();
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::MCMCDelta(double delta) -> void {
    if (not std::isfinite(delta)) [[unlikely]] {
        PrintError(fmt::format("Infinite MCMC delta (got {})", delta));
    }
    if (delta <= muc::default_tolerance<double> or 0.5 <= delta) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious MCMC delta (got {}, expects {} < delta < 0.5)", delta, muc::default_tolerance<double>));
    }
    // Rescale delta
    // E(distance in d-dim space) ~ sqrt(d), if delta = delta0 / sqrt(d) => E(step size) ~ delta0
    fMCMCDelta = (2.38 / std::sqrt(MarkovChain::dim)) * delta;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::MCMCDiscard(unsigned n) -> void {
    if (n == fgDefaultInvalidMCMCDiscard) [[unlikely]] {
        PrintError(fmt::format("Invalid number of discarded MCMC samples (got {})", n));
    }
    if (n >= std::numeric_limits<int>::max()) [[unlikely]] {
        PrintWarning(fmt::format("Suspicious number of discarded MCMC samples (got {})", n));
    }
    fMCMCDiscard = n;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::BurnIn(CLHEP::HepRandomEngine& rng) -> void {
    MasterPrintLn("Markov chain of {} burning in, please wait...", muc::try_demangle(typeid(*this).name()));
    muc::chrono::stopwatch stopwatch;

    // find phase space
    muc::ranges::iota(fMarkovChain.state.pID, 0);
    while (true) {
        rng.flatArray(fMarkovChain.state.u.size(), fMarkovChain.state.u.data());
        const auto event{DirectPhaseSpace(fMarkovChain.state.u)};
        if (not this->IRSafe(event.p)) {
            continue;
        }
        if (const auto acceptance{this->ValidAcceptance(event.p)};
            acceptance > std::numeric_limits<double>::epsilon()) {
            const auto& [detJ, _, pF]{event};
            fMarkovChain.mSqAcceptanceDetJ = this->ValidMSqAcceptanceDetJ(pF, acceptance, detJ);
            break;
        }
    }
    // burning in
    constexpr auto delta0{0.1};
    constexpr auto epsilon{1e-6};
    // E(distance in d-dim space) ~ sqrt(d), and E(random walk displacement) ~ sqrt(random walk distance),
    // nBurnIn should satisfy E(random walk displacement) ~ scale * E(distance in d-dim space),
    // so we solve nBurnIn from sqrt(random walk distance) = scale * sqrt(dimension) with some scale
    constexpr auto travelScale{10};
    const auto nBurnIn{std::log(epsilon / delta0) / std::log(1 - delta0 / (muc::pow(travelScale, 2) * MarkovChain::dim))};
    const auto factor{std::pow(epsilon / delta0, 1 / nBurnIn)};
    for (auto delta{delta0}; delta > epsilon; delta *= factor) {
        NextEvent(rng, delta);
    }
    fBurntIn = true;

    auto time{muc::chrono::seconds<double>{stopwatch.read()}.count()};
    if (mplr::available()) {
        mplr::comm_world().ireduce(mplr::max<double>{}, 0, time).wait(mplr::duty_ratio::preset::relaxed);
    }
    MasterPrintLn("Markov chain burnt in in {:.3f}s.", time);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::operator()(CLHEP::HepRandomEngine& rng, InitialStateMomenta) -> Event {
    if (std::isnan(fMCMCDelta)) {
        Throw<std::logic_error>("MCMC delta has not been set");
    }
    if (fMCMCDiscard == fgDefaultInvalidMCMCDiscard) {
        Throw<std::logic_error>("Number of discarded MCMC samples has not been set");
    }

    if (not fBurntIn) [[unlikely]] {
        PrintWarning("Markov chain not burnt in. Burning it in");
        BurnIn(rng);
    }
    for (unsigned i{}; i < fMCMCDiscard; ++i) {
        NextEvent(rng, fMCMCDelta);
    }
    return NextEvent(rng, fMCMCDelta);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ISMomenta(const InitialStateMomenta& pI) -> void {
    if constexpr (M == 1) {
        if (pI.isNear(Base::ISMomenta(), muc::default_tolerance<double>)) {
            BurnInRequired();
        }
    } else {
        if (not std::ranges::equal(pI, Base::ISMomenta(),
                                   [](auto p, auto q) { return p.isNear(q, muc::default_tolerance<double>); })) {
            BurnInRequired();
        }
    }
    Base::ISMomenta(pI);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::Mass(const std::array<double, N>& mass) -> void {
    if (not std::ranges::equal(mass, this->fGENBOD.Mass(),
                               [](auto a, auto b) { return muc::isclose(a, b); })) {
        BurnInRequired();
    }
    Base::Mass(mass);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::IRCut(int i, double cut) -> void {
    BurnInRequired();
    Base::IRCut(i, cut);
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::AddIdenticalSet(std::vector<int> set) -> void {
    if (set.size() < 2) [[unlikely]] {
        PrintWarning(fmt::format("Identical set should have at least 2 elements (got {}), ignoring it", set.size()));
        return;
    }
    muc::timsort(set);
    if (const auto duplicate{std::ranges::unique(set)};
        duplicate.size() != 0) [[unlikely]] {
        PrintWarning(fmt::format("There are {} duplicate index in identical set, removing them", duplicate.size()));
        set.erase(duplicate.begin(), duplicate.end());
    }
    for (auto&& addedSet : std::as_const(fIdenticalSet)) {
        const auto duplicated{std::ranges::find_first_of(addedSet, set)};
        if (duplicated != addedSet.cend()) [[unlikely]] {
            PrintError(fmt::format("Particle {} added accross different identical sets", *duplicated));
        }
    }
    fIdenticalSet.emplace_back(std::move(set));
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::PhaseSpace(const MarkovChain::State& state) -> Event {
    auto event{DirectPhaseSpace(state.u)};
    event.p = std::apply([&](auto... i) { return FinalStateMomenta{event.p[i]...}; }, state.pID);
    return event;
}

template<int M, int N, std::derived_from<QFT::MatrixElement<M, N>> A>
auto MCMCGenerator<M, N, A>::ProposePID(CLHEP::HepRandomEngine& rng, const std::array<int, N>& pID0, std::array<int, N>& pID) -> void {
    pID = pID0;
    // Walk particle mapping if there are identical particles
    if (fIdenticalSet.empty() or rng.flat() < 0.5) {
        return;
    }
    const auto RandomIndex([&rng](int n) {
        muc::assume(n >= 2);
        auto i{static_cast<int>(n * rng.flat())};
        while (i == n) [[unlikely]] {
            i = n * rng.flat();
        }
        return i;
    });
    const auto& idSet{fIdenticalSet.size() == 1 ?
                          fIdenticalSet.front() :
                          fIdenticalSet[RandomIndex(fIdenticalSet.size())]};
    if (idSet.size() == 2) {
        std::swap(pID[idSet[0]], pID[idSet[1]]);
    } else {
        const auto idA{RandomIndex(idSet.size())};
        const auto idB{(idA + 1) % idSet.size()};
        std::swap(pID[idSet[idA]], pID[idSet[idB]]);
    }
}

} // namespace Mustard::inline Physics::inline Generator
