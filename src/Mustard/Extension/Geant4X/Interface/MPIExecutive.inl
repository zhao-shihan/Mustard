namespace Mustard::inline Extension::Geant4X::inline Interface {

auto MPIExecutive::StartSession(const Geant4CLI& cli, auto&& macFileOrCmdList) const -> void {
    StartSessionImpl(cli, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<typename T>
auto MPIExecutive::StartSession(const Geant4CLI& cli, std::initializer_list<T> cmdList) const -> void {
    StartSessionImpl(cli, cmdList);
}

auto MPIExecutive::StartSession(int argc, char* argv[], auto&& macFileOrCmdList) const -> void {
    StartSessionImpl(argc, argv, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<typename T>
auto MPIExecutive::StartSession(int argc, char* argv[], std::initializer_list<T> cmdList) const -> void {
    StartSessionImpl(argc, argv, cmdList);
}

auto MPIExecutive::StartInteractiveSession(int argc, char* argv[], auto&& macFileOrCmdList) const -> void {
    StartInteractiveSessionImpl(argc, argv, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<typename T>
auto MPIExecutive::StartInteractiveSession(int argc, char* argv[], std::initializer_list<T> cmdList) const -> void {
    StartInteractiveSessionImpl(argc, argv, cmdList);
}

auto MPIExecutive::StartBatchSession(auto&& macFileOrCmdList) const -> void {
    StartBatchSessionImpl(std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

template<typename T>
auto MPIExecutive::StartBatchSession(std::initializer_list<T> cmdList) const -> void {
    StartBatchSessionImpl(cmdList);
}

auto MPIExecutive::StartSessionImpl(const Geant4CLI& cli, auto&& macFileOrCmdList) const -> void {
    if (cli.IsInteractive()) {
        const auto [argc, argv]{cli.ArgcArgv()};
        auto macro{cli.Macro()};
        if (macro) {
            StartInteractiveSessionImpl(argc, argv, std::move(*macro));
        } else {
            StartInteractiveSessionImpl(argc, argv, std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
        }
    } else {
        StartBatchSessionImpl(*cli.Macro());
    }
}

auto MPIExecutive::StartInteractiveSessionImpl(int argc, char* argv[], auto&& macFileOrCmdList) const -> void {
    CheckSequential();
#if MUSTARD_USE_G4VIS
    G4UIExecutive uiExecutive{argc, argv};
    G4VisExecutive visExecutive;
    visExecutive.Initialize();
#else
    G4UIExecutive uiExecutive{argc, argv, "tcsh"};
#endif
    Execute(std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
    uiExecutive.SessionStart();
}

auto MPIExecutive::StartBatchSessionImpl(auto&& macFileOrCmdList) const -> void {
    Execute(std::forward<decltype(macFileOrCmdList)>(macFileOrCmdList));
}

auto MPIExecutive::Execute(const std::ranges::input_range auto& cmdList) -> void
    requires std::convertible_to<typename std::decay_t<decltype(cmdList)>::value_type, std::string>
{
    for (auto&& command : cmdList) {
        if (const auto success = ExecuteCommand(std::forward<decltype(command)>(command));
            not success) { break; }
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline Interface
