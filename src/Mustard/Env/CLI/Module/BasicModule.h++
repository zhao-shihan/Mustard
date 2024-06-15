#pragma once

#include "Mustard/Env/CLI/Module/ModuleBase.h++"
#include "Mustard/Env/VerboseLevel.h++"

#include <optional>
#include <type_traits>

namespace Mustard::Env::CLI::inline Module {

class BasicModule : public ModuleBase {
public:
    BasicModule(argparse::ArgumentParser& argParser);

    auto VerboseLevel() const -> std::optional<enum VerboseLevel>;

private:
    std::underlying_type_t<enum VerboseLevel> fVerboseLevelValue;
};

} // namespace Mustard::Env::CLI::inline Module
