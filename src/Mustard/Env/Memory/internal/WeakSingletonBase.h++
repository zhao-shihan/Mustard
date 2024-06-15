#pragma once

#include "Mustard/Utility/NonMoveableBase.h++"

namespace Mustard::Env::Memory::internal {

/// @brief Implementation detail of Mustard::Env::Memory::WeakSingleton.
/// Not API. Just a signature of mute singleton.
class WeakSingletonBase : public NonMoveableBase {
protected:
    WeakSingletonBase() = default;
    ~WeakSingletonBase() = default;
};

} // namespace Mustard::Env::Memory::internal
