#pragma once

#include "Mustard/Math/Random/UniformRandomBitGeneratorBase.h++"

#include <random>

namespace Mustard::Math::Random::inline Generator {

class RandomDevice final : public UniformRandomBitGeneratorBase<RandomDevice,
                                                                std::random_device::result_type> {
public:
    RandomDevice() = default;

    auto operator()() -> auto { return fRD(); }

    auto Entropy() const -> auto { return fRD.entropy(); }

    static constexpr auto Min() -> auto { return std::random_device::min(); }
    static constexpr auto Max() -> auto { return std::random_device::max(); }

private:
    std::random_device fRD;
};

} // namespace Mustard::Math::Random::inline Generator
