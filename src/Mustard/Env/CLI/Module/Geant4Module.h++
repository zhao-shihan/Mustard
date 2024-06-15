#pragma once

#include "Mustard/Env/CLI/Module/ModuleBase.h++"

namespace Mustard::Env::CLI::inline Module {

class Geant4Module : public ModuleBase {
public:
    Geant4Module(argparse::ArgumentParser& argParser);

    auto Macro() const -> auto { return ArgParser().present("macro"); }
    auto IsInteractive() const -> auto { return not Macro().has_value() or ArgParser().is_used("-i"); }
};

} // namespace Mustard::Env::CLI::inline Module
