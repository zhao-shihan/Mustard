#pragma once

#include "Mustard/Application/Subprogram.h++"

#include <concepts>
#include <memory>

namespace argparse {
class ArgumentParser;
} // namespace argparse

namespace Mustard::Application {

class SubprogramLauncher {
public:
    SubprogramLauncher();
    ~SubprogramLauncher();

    template<std::derived_from<Subprogram> ASubprogram>
    auto AddSubprogram() -> void { AddSubprogram(std::make_unique<ASubprogram>()); }
    auto AddSubprogram(std::unique_ptr<Subprogram> subprogram) -> void;
    auto LaunchMain(int argc, char* argv[]) -> int;

private:
    struct State;
    std::unique_ptr<State> fState;
};

} // namespace Mustard::Application
