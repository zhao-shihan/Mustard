#include "Mustard/Env/CLI/Module/BasicModule.h++"
#include "Mustard/Version.h++"

#include "muc/utility"

#include "fmt/core.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace Mustard::Env::CLI::inline Module {

BasicModule::BasicModule(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser},
    fVerboseLevelValue{muc::to_underlying(VerboseLevel::Warning)} {
    ArgParser()
        .add_argument("-h", "--help")
        .help("Show help message and exit.")
        .nargs(0)
        .action([this](auto&&) {
            fmt::println("{}", ArgParser().help().str());
            std::exit(EXIT_SUCCESS);
        });
    ArgParser()
        .add_argument("-v", "--version")
        .help("Print version and exit.")
        .nargs(0)
        .action([](auto&&) {
            fmt::println("{}", MUSTARD_VERSION_STRING);
            std::exit(EXIT_SUCCESS);
        });
    ArgParser()
        .add_argument("-V", "--verbose")
        .help("Increase verbose level (-2: quiet, -1: error, 0: warning (default), 1: informative, 2: verbose). "
              "This is repeatable (e.g. -V -V or -VV) and can be combined with -Q or --quiet (e.g. -VVQ (=1) -QV (=0) -QQVQV (=-1)).")
        .flag()
        .append()
        .nargs(0)
        .action([this](auto&&) { ++fVerboseLevelValue; });
    ArgParser()
        .add_argument("-Q", "--quiet")
        .help("Decrease verbose level (see previous).")
        .flag()
        .append()
        .nargs(0)
        .action([this](auto&&) { --fVerboseLevelValue; });
}

auto BasicModule::VerboseLevel() const -> std::optional<enum VerboseLevel> {
    if (ArgParser().is_used("-V") or ArgParser().is_used("-Q")) {
        return static_cast<enum VerboseLevel>(std::clamp(fVerboseLevelValue,
                                                         muc::to_underlying(VerboseLevel::Quiet),
                                                         muc::to_underlying(VerboseLevel::Verbose)));
    } else {
        return std::nullopt;
    }
}

} // namespace Mustard::Env::CLI::inline Module
