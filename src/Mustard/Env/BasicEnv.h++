#pragma once

#include "Mustard/Env/CLI/CLI.h++"
#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Env/VerboseLevel.h++"
#include "Mustard/Env/internal/EnvBase.h++"
#include "Mustard/Utility/InlineMacro.h++"

#include "fmt/format.h"

#include <functional>
#include <optional>

namespace Mustard::Env {

class BasicEnv : public internal::EnvBase,
                 public Memory::PassiveSingleton<BasicEnv> {
public:
    BasicEnv(int argc, char* argv[],
             std::optional<std::reference_wrapper<CLI::CLI<>>> cli = {},
             enum VerboseLevel verboseLevel = {},
             bool printWelcomeMessage = true);
    virtual ~BasicEnv();

    auto Argc() const -> auto { return fArgc; }
    auto Argv() const -> auto { return fArgv; }
    auto VerboseLevel() const -> auto { return fVerboseLevel; }

protected:
    auto PrintWelcomeMessageSplitLine() const -> void;
    auto PrintWelcomeMessageBody(int argc, char* argv[]) const -> void;

private:
    int fArgc;
    char** fArgv;
    enum VerboseLevel fVerboseLevel;
};

template<char L>
    requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
MUSTARD_ALWAYS_INLINE auto VerboseLevelReach() -> bool {
    if (not BasicEnv::Available()) [[unlikely]] { return true; }
    const auto vl{BasicEnv::Instance().VerboseLevel()};
    // if constexpr (L == 'Q') { return vl >= VerboseLevel::Quiet; }
    if constexpr (L == 'E') { return vl >= VerboseLevel::Error; }
    if constexpr (L == 'W') { return vl >= VerboseLevel::Warning; }
    if constexpr (L == 'I') { return vl >= VerboseLevel::Informative; }
    if constexpr (L == 'V') { return vl >= VerboseLevel::Verbose; }
}

} // namespace Mustard::Env
