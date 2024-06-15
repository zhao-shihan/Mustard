#pragma once

#include "Mustard/Utility/NonMoveableBase.h++"

namespace Mustard::Env::Memory::internal {

/// @brief Implementation detail of Mustard::Env::Memory::PassiveSingleton.
/// Not API. Just a signature of passive singleton.
class PassiveSingletonBase : public NonMoveableBase {
protected:
    PassiveSingletonBase() = default;
    ~PassiveSingletonBase() = default;
};

} // namespace Mustard::Env::Memory::internal
