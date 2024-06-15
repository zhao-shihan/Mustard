#pragma once

#include <memory>
#include <string>
#include <string_view>

class TMacro;

namespace Mustard::inline Utility {

auto MakeTextTMacro(std::string_view text, const std::string& name, const std::string& title = {}) -> std::unique_ptr<TMacro>;

} // namespace Mustard::inline Utility
