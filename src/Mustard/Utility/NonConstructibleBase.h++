#pragma once

namespace Mustard::inline Utility {

class NonConstructibleBase {
private:
    constexpr NonConstructibleBase() = delete;
};

} // namespace Mustard::inline Utility
