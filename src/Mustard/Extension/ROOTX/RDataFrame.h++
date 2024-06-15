#pragma once

#include "ROOT/RDF/InterfaceUtils.hxx"

#include <concepts>

namespace Mustard::inline Extension::ROOTX {

template<typename T>
concept RDataFrame =
    requires {
        requires std::convertible_to<T, ROOT::RDF::RNode>;
        requires not std::is_const_v<std::remove_reference_t<T>>;
    };

} // namespace Mustard::inline Extension::ROOTX
