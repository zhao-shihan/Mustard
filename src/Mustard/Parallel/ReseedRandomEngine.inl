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

namespace Mustard::Parallel {

template<muc::random_number_engine E>
auto ReseedRandomEngine(E& engine) -> void {
    if (not impl::ReseedNeeded()) {
        return;
    }
    using SeedType = typename E::result_type;
    const auto material{impl::DrawSeedMaterial<SeedType>([&engine] { return engine(); })};
    engine.seed(static_cast<SeedType>(impl::RankSeed(static_cast<std::uint64_t>(material),
                                                     std::numeric_limits<SeedType>::max() - 1)));
}

namespace impl {

template<std::integral T, typename ADraw>
    requires std::unsigned_integral<std::invoke_result_t<ADraw&>>
auto DrawSeedMaterial(ADraw draw) -> T {
    using DrawResult = std::invoke_result_t<ADraw&>;
    if constexpr (sizeof(T) <= sizeof(DrawResult)) {
        return static_cast<T>(draw());
    } else {
        static_assert(sizeof(T) % sizeof(DrawResult) == 0,
                      "Seed material must be an integral number of drawn words wide.");
        std::array<DrawResult, sizeof(T) / sizeof(DrawResult)> word;
        std::ranges::generate(word, draw);
        return std::bit_cast<T>(word);
    }
}

} // namespace impl

} // namespace Mustard::Parallel
