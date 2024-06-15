#pragma once

#include "Mustard/Env/CLI/Module/ModuleBase.h++"

namespace Mustard::Env::CLI::inline Module {

class MonteCarloModule : public ModuleBase {
public:
    MonteCarloModule(argparse::ArgumentParser& argParser);

    auto SeedRandomIfFlagged() const -> bool;
};

} // namespace Mustard::Env::CLI::inline Module
