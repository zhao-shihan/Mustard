#pragma once

namespace Mustard::Concept {

template<typename T>
concept Unordered =
    requires(T& v, const T& cv) {
        requires not requires { v <=> cv; };
        requires not requires { cv <=> cv; };
        requires not requires { v < cv; };
        requires not requires { cv < cv; };
        requires not requires { v > cv; };
        requires not requires { cv > cv; };
        requires not requires { v <= cv; };
        requires not requires { cv <= cv; };
        requires not requires { v >= cv; };
        requires not requires { cv >= cv; };
    };

} // namespace Mustard::Concept
