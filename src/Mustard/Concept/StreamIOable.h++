#pragma once

#include <istream>
#include <ostream>

namespace Mustard::Concept {

template<typename T>
concept StreamInputable =
    requires(std::istream& is, std::wistream& wis, T& obj) {
        { is >> obj } -> std::same_as<std::istream&>;
        { wis >> obj } -> std::same_as<std::wistream&>;
    };

template<typename T>
concept StreamOutputable =
    requires(std::ostream& os, std::wostream& wos, const T obj) {
        { os << obj } -> std::same_as<std::ostream&>;
        { wos << obj } -> std::same_as<std::wostream&>;
    };

template<typename T>
concept StreamIOable = StreamInputable<T> and StreamOutputable<T>;

} // namespace Mustard::Concept
