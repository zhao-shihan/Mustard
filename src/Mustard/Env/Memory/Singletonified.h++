#pragma once

#include "Mustard/Concept/NonMoveable.h++"

#include <concepts>
#include <type_traits>

namespace Mustard::Env::Memory {

namespace internal {

class WeakSingletonBase;
class SingletonBase;

} // namespace internal

template<typename ADerived>
class Singleton;

template<typename T>
concept Singletonified =
    requires {
        { T::Instance() } -> std::same_as<T&>;
        requires std::derived_from<T, Singleton<T>>;
        requires std::derived_from<T, internal::SingletonBase>;
        requires not std::is_base_of_v<internal::WeakSingletonBase, T>;
        requires Concept::NonMoveable<T>;
        requires std::is_final_v<T>;
        requires not std::is_default_constructible_v<T>; // try to constrain to private or protected constructor
    };

} // namespace Mustard::Env::Memory
