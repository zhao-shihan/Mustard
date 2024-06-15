#pragma once

#include "Mustard/Concept/Unordered.h++"

namespace Mustard::Concept {

template<typename T>
concept NonComparable =
    requires(T& v, const T& cv) {
        requires not requires { v == cv; };
        requires not requires { cv == cv; };
        requires not requires { v != cv; };
        requires not requires { cv != cv; };
        requires Unordered<T>;
    };

} // namespace Mustard::Concept
