#pragma once

#include "Mustard/Env/Memory/Singleton.h++"

#include "G4Allocator.hh"

namespace Mustard::inline Extension::Geant4X::inline Memory {

template<typename T>
class SingletonG4Allocator final : public Env::Memory::Singleton<SingletonG4Allocator<T>>,
                                   public G4Allocator<T> {
    friend Env::Memory::SingletonInstantiator;

private:
    SingletonG4Allocator() = default;
    ~SingletonG4Allocator() = default;
};

} // namespace Mustard::inline Extension::Geant4X::inline Memory
