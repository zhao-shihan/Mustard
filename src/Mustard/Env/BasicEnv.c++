#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/CLI/Module/BasicModule.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Version.h++"

#include <filesystem>
#include <system_error>
#include <typeinfo>

namespace Mustard::Env {

BasicEnv::BasicEnv(int argc, char* argv[],
                   std::optional<std::reference_wrapper<CLI::CLI<>>> cli,
                   enum VerboseLevel verboseLevel,
                   bool printWelcomeMessage) :
    EnvBase{},
    PassiveSingleton{},
    fArgc{argc},
    fArgv{argv},
    fVerboseLevel{verboseLevel} {
    // CLI: do parse and get args
    if (cli) {
        const auto pCLI{&cli->get()};
        pCLI->ParseArgs(argc, argv);
        const auto basicCLI{dynamic_cast<const CLI::BasicModule*>(pCLI)};
        if (basicCLI) { fVerboseLevel = basicCLI->VerboseLevel().value_or(verboseLevel); }
    }
    // Print startup message after parse
    if (printWelcomeMessage) {
        PrintWelcomeMessageSplitLine();
        PrintWelcomeMessageBody(argc, argv);
        PrintWelcomeMessageSplitLine();
    }
}

auto BasicEnv::PrintWelcomeMessageSplitLine() const -> void {
    Print("\n===============================================================================\n");
}

auto BasicEnv::PrintWelcomeMessageBody(int argc, char* argv[]) const -> void {
    std::error_code cwdError;
    const auto exe{std::filesystem::path(argv[0]).filename().generic_string()};
    auto cwd{std::filesystem::current_path(cwdError).generic_string()};
    if (cwdError) { cwd = "<Error getting current working directory>"; }
    Print("\n"
          " Mustard: an offline software framework for HEP experiment\n"
          " Version {}"
          " Copyright 2020-2024 Mustard developers\n"
          "\n"
          " Exe: {}",
          MUSTARD_VERSION_STRING,
          exe);
    for (auto i{1}; i < argc; ++i) {
        Print(" {}", argv[i]);
    }
    Print("\n"
          " CWD: {}\n",
          cwd);
    Print<'I'>("\n"
               " List of all {} command line arguments:\n",
               argc);
    for (int i{}; i < argc; ++i) {
        PrintLn<'I'>("  argv[{}]: {}", i, argv[i]);
    }
}

} // namespace Mustard::Env
