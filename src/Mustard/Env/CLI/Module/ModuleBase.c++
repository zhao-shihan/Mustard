#include "Mustard/Env/CLI/Module/ModuleBase.h++"

namespace Mustard::Env::CLI::inline Module {

ModuleBase::ModuleBase(argparse::ArgumentParser& argParser) :
    NonMoveableBase{},
    fArgParser{&argParser} {}

} // namespace Mustard::Env::CLI::inline Module
