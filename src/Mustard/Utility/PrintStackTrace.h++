#pragma once

#include "Mustard/Utility/InlineMacro.h++"

#include <iostream>

namespace Mustard::inline Utility {

MUSTARD_NOINLINE auto PrintStackTrace(int depth = 64, int skip = 0, std::ostream& os = std::clog) -> void;

} // namespace Mustard::inline Utility
