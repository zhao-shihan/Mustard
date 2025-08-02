#include "Mustard/Env/CLI/MonteCarloCLI.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Execution/Executor.h++"
#include "Mustard/Parallel/ProcessSpecificPath.h++"
#include "Mustard/Physics/Generator/EventGenerator.h++"
#include "Mustard/Physics/Generator/GENBOD.h++"
#include "Mustard/Physics/Generator/RAMBO.h++"
#include "Mustard/Utility/LiteralUnit.h++"
#include "Mustard/Utility/PhysicalConstant.h++"
#include "Mustard/Utility/UseXoshiro.h++"

#include "TFile.h"
#include "TH2D.h"

#include "muc/math"

#include <cstdlib>

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::CLI::MonteCarloCLI<> cli;
    cli->add_argument("n").help("Number of events to generate.").nargs(1).scan<'i', unsigned long long>();
    cli->add_argument("-o", "--output").help("Output file path.").default_value("TestPhaseSpaceGeneratorResult.root").required().nargs(1);
    cli->add_argument("--output-mode").help("Output file creation mode.").default_value("NEW").required().nargs(1);
    Mustard::Env::MPIEnv env{argc, argv, cli};

    Mustard::UseXoshiro<256> random;
    cli.SeedRandomIfFlagged();

    const auto filePath{Mustard::Parallel::ProcessSpecificPath(cli->get("-o"))};
    const TFile file{filePath.generic_string().c_str(), cli->get("--output-mode").c_str()};
    if (not file.IsOpen()) {
        return EXIT_FAILURE;
    }

    const auto nEvent{cli->get<unsigned long long>("n")};
    const auto nBin{gsl::narrow<int>(muc::llround(std::sqrt(nEvent / 100000)))};
    Mustard::Executor<unsigned long long> executor{"Generation", "Sample"};

    constexpr auto FillDalitzPlot{[](const Mustard::EventGenerator<3>& generator,
                                     TH2D& dalitzPlot, TH2D& weightPlot) {
        const auto event{generator()};
        const auto& [p1, p2, p3]{event.state};
        const auto m12{(p1 + p2).m2()};
        const auto m23{(p2 + p3).m2()};
        dalitzPlot.Fill(m12, m23, event.weight);
        weightPlot.Fill(m12, m23);
    }};

    constexpr auto WritePlot{[](TH2D& dalitzPlot, TH2D& weightPlot) {
        dalitzPlot.Write();
        weightPlot.Write();
        // TH2D one{weightPlot};
        // for (auto i{1}; i <= one.GetNcells(); ++i) {
        //     one.SetBinContent(i, 1);
        //     one.SetBinError(i, 0);
        // }
        // one.Divide(&weightPlot);
        // one.Write();
    }};

    using namespace Mustard::LiteralUnit::Energy;
    constexpr auto mLc{1869.66_MeV};
    constexpr auto mPi{139.57039_MeV};
    constexpr auto mK{497.677_MeV};
    constexpr auto mP{938.27209_MeV};
    constexpr auto mPiKLow{muc::pow<2>(mPi + mK)};
    constexpr auto mPiKUp{muc::pow<2>(mLc - mP)};
    constexpr auto mKPLow{muc::pow<2>(mK + mP)};
    constexpr auto mKPUp{muc::pow<2>(mLc - mPi)};

    Mustard::RAMBO<3> ramboLc2PiKP(mLc, {mPi, mK, mP});
    TH2D ramboLc2PiKPDalitzPlot{"RAMBO_Lc2PiKP", "RAMBO_Lc2PiKP", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    TH2D ramboLc2PiKPWeight{"RAMBO_Lc2PiKP_Weight", "RAMBO_Lc2PiKP_Weight", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(ramboLc2PiKP, ramboLc2PiKPDalitzPlot, ramboLc2PiKPWeight); });
    executor.PrintExecutionSummary();
    WritePlot(ramboLc2PiKPDalitzPlot, ramboLc2PiKPWeight);

    Mustard::GENBOD<3> genbodLc2PiKP(mLc, {mPi, mK, mP});
    TH2D genbodLc2PiKPDalitzPlot{"GENBOD_Lc2PiKP", "GENBOD_Lc2PiKP", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    TH2D genbodLc2PiKPWeight{"GENBOD_Lc2PiKP_Weight", "GENBOD_Lc2PiKP_Weight", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(genbodLc2PiKP, genbodLc2PiKPDalitzPlot, genbodLc2PiKPWeight); });
    executor.PrintExecutionSummary();
    WritePlot(genbodLc2PiKPDalitzPlot, genbodLc2PiKPWeight);

    using namespace Mustard::PhysicalConstant;
    Mustard::RAMBO<3> ramboMu2ENN(muon_mass_c2, {electron_mass_c2, 0, 0});
    TH2D ramboMu2ENNDalitzPlot{"RAMBO_Mu2ENN", "RAMBO_Mu2ENN", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    TH2D ramboMu2ENNWeight{"RAMBO_Mu2ENN_Weight", "RAMBO_Mu2ENN_Weight", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(ramboMu2ENN, ramboMu2ENNDalitzPlot, ramboMu2ENNWeight); });
    executor.PrintExecutionSummary();
    WritePlot(ramboMu2ENNDalitzPlot, ramboMu2ENNWeight);

    Mustard::GENBOD<3> genbodMu2ENN(muon_mass_c2, {electron_mass_c2, 0, 0});
    TH2D genbodMu2ENNDalitzPlot{"GENBOD_Mu2ENN", "GENBOD_Mu2ENN", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    TH2D genbodMu2ENNWeight{"GENBOD_Mu2ENN_Weight", "GENBOD_Mu2ENN_Weight", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(genbodMu2ENN, genbodMu2ENNDalitzPlot, genbodMu2ENNWeight); });
    executor.PrintExecutionSummary();
    WritePlot(genbodMu2ENNDalitzPlot, genbodMu2ENNWeight);

    return EXIT_SUCCESS;
}
