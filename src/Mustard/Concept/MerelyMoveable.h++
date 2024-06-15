#pragma once

#include "Mustard/Concept/NonCopyable.h++"

#include <concepts>

namespace Mustard::Concept {

/// @brief Merely moveable and not copyable
template<typename T>
concept MerelyMoveable =
    requires {
        requires NonCopyable<T>;
        requires std::movable<T>;
    };

} // namespace Mustard::Concept
