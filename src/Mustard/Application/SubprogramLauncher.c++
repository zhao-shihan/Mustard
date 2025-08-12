#include "Mustard/Application/SubprogramLauncher.h++"
#include "Mustard/IO/PrettyLog.h++"

#include "argparse/argparse.hpp"

#include "muc/hash_map"
#include "muc/utility"

#include "fmt/core.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <typeindex>
#include <utility>
#include <vector>

namespace Mustard::Application {

struct SubprogramLauncher::State {
    std::unique_ptr<argparse::ArgumentParser> command;
    muc::flat_hash_map<std::type_index, std::pair<std::unique_ptr<Subprogram>, std::unique_ptr<argparse::ArgumentParser>>> subprogram;
};

SubprogramLauncher::SubprogramLauncher() :
    fState(new State{std::make_unique<argparse::ArgumentParser>("", "", argparse::default_arguments::help), {}}) {
}

SubprogramLauncher::~SubprogramLauncher() = default;

auto SubprogramLauncher::AddSubprogram(std::unique_ptr<Subprogram> subprogram) -> void {
    if (fState == nullptr) {
        Throw<std::logic_error>("Subprogram has been launched");
    }
    const auto& subprogramObject{*subprogram}; // mute a warning
    const auto& subprogramType{typeid(subprogramObject)};
    if (fState->subprogram.contains(subprogramType)) {
        Throw<std::invalid_argument>(fmt::format("Subprogram '{}' added twice", muc::try_demangle(subprogramType.name())));
    }

    auto subcommand{std::make_unique<argparse::ArgumentParser>(subprogram->Name(), "", argparse::default_arguments::none)};
    subcommand->add_description(subprogram->Description());
    fState->command->add_subparser(*subcommand);

    fState->subprogram.try_emplace(subprogramType, std::move(subprogram), std::move(subcommand));
}

auto SubprogramLauncher::LaunchMain(int argc, char* argv[]) -> int {
    if (fState == nullptr) {
        Throw<std::logic_error>("Subprogram has been launched");
    }

    try {
        fState->command->parse_args(std::min(argc, 2), argv);
    } catch (const std::runtime_error& exception) {
        fmt::println(stderr, "{}", exception.what());
        fmt::println(stderr, "Try {} --help", argv[0]);
        std::exit(EXIT_FAILURE);
    }
    const auto iSubprogram{
        std::ranges::find_if(fState->subprogram,
                             [this](auto&& p) {
                                 const auto& [_, subcommand]{p.second};
                                 return fState->command->is_subcommand_used(*subcommand);
                             })};
    if (iSubprogram == fState->subprogram.cend()) {
        fmt::print("{}", fState->command->help().view());
        std::exit(EXIT_FAILURE);
    }
    auto& [subprogram, _]{iSubprogram->second};
    const auto launchingSubprogram{std::move(subprogram)};
    fState.reset();
    return launchingSubprogram->Main(argc - 1, argv + 1);
}

} // namespace Mustard::Application
