#pragma once

#include "Mustard/Concept/StreamIOable.h++"
#include "Mustard/Math/Random/UniformRandomBitGenerator.h++"

#include <concepts>
#include <random>
#include <type_traits>

namespace Mustard::Math::Random {

template<typename ADerived,
         std::unsigned_integral AResult,
         std::unsigned_integral ASeed>
class UniformPseudoRandomBitGeneratorBase;

/// @brief Concept of uniform pseudo random bit generator.
template<typename G>
concept UniformPseudoRandomBitGenerator =
    requires(G& g) {
        // 1. A UniformPseudoRandomBitGenerator satisfies UniformRandomBitGenerator.
        requires UniformRandomBitGenerator<G>;
        // 2. Has SeedType. It is an unsigned integral type.
        typename G::SeedType;
        requires std::unsigned_integral<typename G::SeedType>;
        // 3. It is constructible from a seed, this constructor is explicit.
        requires std::constructible_from<G, const typename G::SeedType>;
        requires not std::convertible_to<typename G::SeedType&, G>;
        // 4. It is seedable.
        requires requires(const typename G::SeedType seed) {
            { g.seed(seed) } -> std::same_as<void>;
            { g.Seed(seed) } -> std::same_as<void>;
        };
        // 5. It is equality comparable. Given two UniformPseudoRandomBitGenerator
        // g1, g2, g1 == g2 is true means the following calls to g1() and g2()
        // produce exactly the same number sequence.
        requires std::equality_comparable<G>;
        // 6. It is copyable. Given a UniformPseudoRandomBitGenerator g, G(g) == g
        // always hold.
        requires std::copyable<G>;
        // 7. It is TriviallyCopyable (a C++ named requirements).
        requires std::is_trivially_copyable_v<G>;
        // 8. It is a standard-layout type.
        requires std::is_standard_layout_v<G>;
        // 9. Its state can be saved and restored.
        requires Concept::StreamIOable<G>;
        // 10. Extra requirements.
        requires std::derived_from<G, UniformPseudoRandomBitGeneratorBase<G, typename G::ResultType, typename G::SeedType>>;
        requires std::is_final_v<G>;
    };

} // namespace Mustard::Math::Random
