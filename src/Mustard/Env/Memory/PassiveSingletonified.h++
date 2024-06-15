#pragma once

#include "Mustard/Concept/NonMoveable.h++"

#include <concepts>
#include <type_traits>

namespace Mustard::Env::Memory {

namespace internal {

class SingletonBase;

} // namespace internal

template<typename ADerived>
class PassiveSingleton;

template<typename T>
concept PassiveSingletonified =
    requires {
        { T::Instance() } -> std::same_as<T&>;
        requires std::derived_from<T, PassiveSingleton<T>>;
        requires not std::is_base_of_v<internal::SingletonBase, T>;
        requires Concept::NonMoveable<T>;
    };

} // namespace Mustard::Env::Memory
