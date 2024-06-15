#include "Mustard/Utility/CreateTemporaryFile.h++"
#include "Mustard/Utility/MakeTextTMacro.h++"

#include "TMacro.h"

#include "fmt/format.h"

#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <system_error>

namespace Mustard::inline Utility {

auto MakeTextTMacro(std::string_view text, const std::string& name, const std::string& title) -> std::unique_ptr<TMacro> {
    const auto tempMacroPath{CreateTemporaryFile(name, ".C")};
    {
        const auto tempMacroFile{std::fopen(tempMacroPath.generic_string().c_str(), "w")};
        if (tempMacroFile == nullptr) { throw std::runtime_error{"error opening temp macro file"}; }
        fmt::println(tempMacroFile, R"macro(
#include <iostream>

auto {0}() -> void {{
    std::cout << "\n" R"{0}({1}){0}" << std::endl;
}}
)macro",
                     name, text);
        std::fclose(tempMacroFile);
    }
    auto macro{std::make_unique<TMacro>(name.c_str(), title.c_str())};
    {
        const auto lines{macro->ReadFile(tempMacroPath.generic_string().c_str())};
        if (lines == 0) { throw std::runtime_error{"error opening temp macro file"}; }
    }
    std::error_code muteRemoveError;
    std::filesystem::remove(tempMacroPath, muteRemoveError);
    return macro;
}

} // namespace Mustard::inline Utility
