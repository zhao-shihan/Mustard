#include "AirShower/Action/ActionInitialization.h++"
#include "AirShower/Action/DetectorConstruction.h++"
#include "AirShower/Analysis.h++"
#include "AirShower/DefaultMacro.h++"

#include "Mustard/Env/CLI/Geant4CLI.h++"
#include "Mustard/Env/CLI/Module/Geant4ReferencePhysicsListModule.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/Geant4X/Interface/MPIExecutive.h++"
#include "Mustard/Extension/Geant4X/Run/MPIRunManager.h++"
#include "Mustard/Utility/UseXoshiro.h++"

auto main(int argc, char* argv[]) -> int {
    // Use default Geant4 CLI and extends it with reference physics list interface
    Mustard::Env::CLI::Geant4CLI<Mustard::Env::CLI::Geant4ReferencePhysicsListModule<>> cli;
    Mustard::Env::MPIEnv env{argc, argv, cli};

    Mustard::UseXoshiro<512> random;
    // First set random seed here
    cli.SeedRandomIfFlagged();

    // Mutually exclusive random seeds are distributed to all processes upon each BeamOn.
    Mustard::Geant4X::MPIRunManager runManager;
    // Physics lists
    runManager.SetUserInitialization(cli.PhysicsList());
    // Register detector construction
    runManager.SetUserInitialization(new AirShower::DetectorConstruction{env.VerboseLevelReach<'I'>()});
    // Register action initialization, including run action, event action, etc.
    runManager.SetUserInitialization(new AirShower::ActionInitialization);
    // Instanitiate analysis
    AirShower::Analysis anaylsis;

    // Start UI session
    Mustard::Geant4X::MPIExecutive{}.StartSession(cli, AirShower::defaultMacro);

    return EXIT_SUCCESS;
}
