#pragma once

#include "Mustard/Env/CLI/CLI.h++"
#include "Mustard/Env/CLI/Module/BasicModule.h++"
#include "Mustard/Env/CLI/Module/MonteCarloModule.h++"

namespace Mustard::Env::CLI {

using Geant4CLI = CLI<BasicModule,
                      MonteCarloModule>;

} // namespace Mustard::Env::CLI
