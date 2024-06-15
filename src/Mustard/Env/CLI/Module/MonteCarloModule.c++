#include "Mustard/Env/CLI/Module/MonteCarloModule.h++"

#include "CLHEP/Random/Random.h"

#include "TRandom.h"

#include <bit>
#include <random>

namespace Mustard::Env::CLI::inline Module {

MonteCarloModule::MonteCarloModule(argparse::ArgumentParser& argParser) :
    ModuleBase{argParser} {
    ArgParser()
        .add_argument("-s", "--seed")
        .help("Set random seed. 0 means using random device (non deterministic random seed). Predefined deterministic seed is used by default.")
        .scan<'i', long>();
}

auto MonteCarloModule::SeedRandomIfFlagged() const -> bool {
    auto seed{ArgParser().present<long>("-s")};
    if (not seed.has_value()) { return false; }
    const auto theSeed{*seed != 0 ? *seed :
                                    std::bit_cast<int>(std::random_device{}())};
    if (const auto clhepRandom{CLHEP::HepRandom::getTheEngine()};
        clhepRandom) {
        clhepRandom->setSeed(theSeed, 3);
    }
    if (gRandom) {
        // Try to decorrelate with CLHEP
        const auto rootSeed{std::mt19937_64{static_cast<unsigned long>(theSeed)}()};
        gRandom->SetSeed(rootSeed);
    }
    return true;
}

} // namespace Mustard::Env::CLI::inline Module
