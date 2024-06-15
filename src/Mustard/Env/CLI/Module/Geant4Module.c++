#include "Mustard/Env/CLI/Module/Geant4Module.h++"

namespace Mustard::Env::CLI::inline Module {

Geant4Module::Geant4Module(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser} {
    ArgParser()
        .add_argument("macro")
        .help("Run the program in batch session with it. If not provided, run in interactive session with default initialization.")
        .nargs(argparse::nargs_pattern::optional);
    ArgParser()
        .add_argument("-i", "--interactive")
        .help("Run in interactive session despite of a provided macro. The macro will initialize the session.")
        .nargs(0);
}

} // namespace Mustard::Env::CLI::inline Module
