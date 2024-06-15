namespace Mustard::Env::CLI {

template<std::derived_from<ModuleBase>... AModules>
CLI<AModules...>::CLI() :
    CLI<>{},
    AModules{ArgParser()}... {}

} // namespace Mustard::Env::CLI
