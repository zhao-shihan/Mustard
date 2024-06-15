#pragma once

#include "Mustard/Extension/Geant4X/Memory/SingletonG4Allocator.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace Mustard::inline Extension::Geant4X::inline Memory {

template<typename ADerived>
class UseG4Allocator {
protected:
    UseG4Allocator() = default;
    ~UseG4Allocator() = default;

public:
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator new(std::size_t) -> void*;
    [[nodiscard]] MUSTARD_ALWAYS_INLINE auto operator new[](std::size_t) -> void* = delete;

    MUSTARD_ALWAYS_INLINE auto operator delete(void* ptr) -> void;
    MUSTARD_ALWAYS_INLINE auto operator delete[](void*) -> void = delete;
};

} // namespace Mustard::inline Extension::Geant4X::inline Memory

#include "Mustard/Extension/Geant4X/Memory/UseG4Allocator.inl"
