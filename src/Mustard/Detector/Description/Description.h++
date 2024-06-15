#pragma once

#include "Mustard/Env/Memory/Singletonified.h++"

#include <concepts>

namespace Mustard::Detector::Description {

template<typename... Ts>
    requires(sizeof...(Ts) <= 1)
class DescriptionBase;

template<typename T>
concept Description =
    requires {
        requires std::derived_from<T, DescriptionBase<>>;
        requires std::derived_from<T, DescriptionBase<T>>;
        requires Env::Memory::Singletonified<T>;
    };

} // namespace Mustard::Detector::Description
