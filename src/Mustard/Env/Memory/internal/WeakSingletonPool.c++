#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/Memory/internal/WeakSingletonPool.h++"
#include "Mustard/Env/Print.h++"

#include <utility>

namespace Mustard::Env::Memory::internal {

WeakSingletonPool* WeakSingletonPool::fgInstance = nullptr;

WeakSingletonPool::WeakSingletonPool() :
    NonMoveableBase{},
    fInstanceMap{} {
    if (fgInstance == nullptr) {
        fgInstance = this;
    } else {
        throw std::logic_error{"Mustard::Env::Memory::internal::WeakSingletonPool::WeakSingletonPool(): "
                               "Trying to instantiate the pool twice"};
    }
}

WeakSingletonPool::~WeakSingletonPool() {
    for (auto&& [type, instance] : std::as_const(fInstanceMap)) {
        if (instance.expired()) {
            PrintLnError("Mustard::Env::Memory::internal::WeakSingletonPool::~WeakSingletonPool(): "
                         "Instance pointer of {} expired",
                         type.name());
        }
        if (*instance.lock() != nullptr) [[unlikely]] {
            PrintLnError("Mustard::Env::Memory::internal::WeakSingletonPool::~WeakSingletonPool(): "
                         "Instance of {} survives, "
                         "implies memory leak or following undefined behavior",
                         type.name());
        }
    }
    fgInstance = nullptr;
}

auto WeakSingletonPool::Instance() -> WeakSingletonPool& {
    if (fgInstance != nullptr) {
        return *fgInstance;
    } else {
        throw std::logic_error{"Mustard::Env::Memory::internal::WeakSingletonPool::Instance(): "
                               "The pool has not been instantiated or has been destructed "
                               "(maybe you forgot to instantiate an environment?)"};
    }
}

} // namespace Mustard::Env::Memory::internal
