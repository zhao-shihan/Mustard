#pragma once

#include <memory>

namespace Mustard::inline Utility {

/// @brief Use Xoshiro random engine for CLHEP and ROOT, Xoshiro** for CLHEP and Xoshiro++ for ROOT.
/// @tparam Xoshiro bit width.
/// @note RAII style class. Random engines share lifetime with this class object.
template<unsigned ABitWidth>
class UseXoshiro {
public:
    UseXoshiro();
    ~UseXoshiro();

private:
    struct Random;

private:
    std::unique_ptr<Random> fRandom;
};

} // namespace Mustard::inline Utility
