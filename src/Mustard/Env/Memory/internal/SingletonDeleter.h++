#pragma once

#include "Mustard/Env/Memory/WeakSingleton.h++"

namespace Mustard::Env::Memory::internal {

/// @brief Implementation detail of Mustard::Env::Memory::Singleton.
/// Not API.
class SingletonDeleter final : public WeakSingleton<SingletonDeleter> {
public:
    ~SingletonDeleter();
};

} // namespace Mustard::Env::Memory::internal
