#pragma once

#include "Mustard/Utility/NonMoveableBase.h++"

#include <memory>

namespace Mustard::Env {

namespace Memory::internal {

class WeakSingletonPool;
class SingletonDeleter;
class SingletonPool;

} // namespace Memory::internal

namespace internal {

class EnvBase : public NonMoveableBase {
protected:
    EnvBase();
    ~EnvBase();

private:
    static auto CheckFundamentalType() -> void;

private:
    std::unique_ptr<Memory::internal::WeakSingletonPool> fWeakSingletonPool;
    std::unique_ptr<Memory::internal::SingletonPool> fSingletonPool;
    std::unique_ptr<Memory::internal::SingletonDeleter> fSingletonDeleter;
};

} // namespace internal

} // namespace Mustard::Env
