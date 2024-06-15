#pragma once

#include "Mustard/Env/CLI/CLI.h++"
#include "Mustard/Env/CLI/Module/BasicModule.h++"
#include "Mustard/Env/CLI/Module/Geant4Module.h++"
#include "Mustard/Env/CLI/Module/MonteCarloModule.h++"

namespace Mustard::Env::CLI {

using Geant4CLI = CLI<BasicModule,
                      MonteCarloModule,
                      Geant4Module>;

} // namespace Mustard::Env::CLI
