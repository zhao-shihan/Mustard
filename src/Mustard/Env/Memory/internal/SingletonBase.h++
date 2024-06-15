#pragma once

#include "Mustard/Utility/NonMoveableBase.h++"

namespace Mustard::Env::Memory::internal {

/// @brief Implementation detail of Mustard::Env::Memory::Singleton.
/// Not API.
/// @details The direct base of Singleton<T>. This allow us to manage instances
/// using polymorphism mechanism.
class SingletonBase : public NonMoveableBase {
    friend class SingletonDeleter;

protected:
    SingletonBase() = default;
    virtual ~SingletonBase() = default;
};

} // namespace Mustard::Env::Memory::internal
