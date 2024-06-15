#pragma once

#include "Mustard/Concept/NonMoveable.h++"

#include <concepts>
#include <type_traits>

namespace Mustard::Env::Memory {

namespace internal {

class SingletonBase;
class WeakSingletonBase;

} // namespace internal

template<typename ADerived>
class WeakSingleton;

template<typename T>
concept IndirectlyWeakSingletonified =
    requires {
        requires std::is_base_of_v<internal::WeakSingletonBase, T>;
        requires not std::is_base_of_v<internal::SingletonBase, T>;
        requires Concept::NonMoveable<T>;
    };

template<typename T>
concept WeakSingletonified =
    requires {
        requires std::derived_from<T, WeakSingleton<T>>;
        requires IndirectlyWeakSingletonified<T>;
    };

} // namespace Mustard::Env::Memory
