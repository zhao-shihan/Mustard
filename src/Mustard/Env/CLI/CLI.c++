#include "Mustard/Env/CLI/CLI.h++"
#include "Mustard/Version.h++"

#include "fmt/core.h"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace Mustard::Env::CLI {

CLI<>::CLI() :
    NonMoveableBase{},
    fArgcArgv{},
    fArgParser{std::make_unique<argparse::ArgumentParser>("", MUSTARD_VERSION_STRING, argparse::default_arguments::none)} {
    if (static bool gInstantiated{}; gInstantiated) {
        throw std::logic_error{"Mustard::Env::CLI::CLI<>: Trying to construct CLI twice"};
    } else {
        gInstantiated = true;
    }
}

CLI<>::~CLI() = default;

auto CLI<>::ParseArgs(int argc, char* argv[]) -> void {
    if (not Parsed()) {
        try {
            ArgParser().parse_args(argc, argv);
        } catch (const std::runtime_error& exception) {
            fmt::println(stderr, "{}", exception.what());
            fmt::println(stderr, "Try {} --help", argv[0]);
            std::exit(EXIT_FAILURE);
        }
        fArgcArgv = {argc, argv};
    } else {
        ThrowParsed();
    }
}

auto CLI<>::ArgcArgv() const -> ArgcArgvType {
    if (Parsed()) {
        return fArgcArgv.value();
    } else {
        ThrowNotParsed();
    }
}

[[noreturn]] auto CLI<>::ThrowParsed() -> void {
    throw std::logic_error{"Mustard::Env::CLI: Command line arguments has been parsed"};
}

[[noreturn]] auto CLI<>::ThrowNotParsed() -> void {
    throw std::logic_error{"Mustard::Env::CLI: Command line arguments has not been parsed"};
}

} // namespace Mustard::Env::CLI
