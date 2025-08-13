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
    cli->add_argument("-o", "--output").help("Output file path.").default_value("test_phase_space_result.root").required().nargs(1);
    cli->add_argument("-m", "--output-mode").help("Output file creation mode.").default_value("NEW").required().nargs(1);
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

    constexpr auto FillDalitzPlot{[](Mustard::EventGenerator<1, 3>& generator,
                                     double cmsE, TH2D& dalitzPlot, TH2D& UnweightedPlot) {
        const auto event{generator({cmsE, {}})};
        const auto& [p1, p2, p3]{event.p};
        const auto m12{(p1 + p2).m2()};
        const auto m23{(p2 + p3).m2()};
        dalitzPlot.Fill(m12, m23, event.weight);
        UnweightedPlot.Fill(m12, m23);
    }};

    constexpr auto WritePlot{[](TH2D& dalitzPlot, TH2D& UnweightedPlot) {
        dalitzPlot.Write();
        UnweightedPlot.Write();
        // TH2D one{UnweightedPlot};
        // for (auto i{1}; i <= one.GetNcells(); ++i) {
        //     one.SetBinContent(i, 1);
        //     one.SetBinError(i, 0);
        // }
        // one.Divide(&UnweightedPlot);
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

    Mustard::RAMBO<1, 3> ramboLc2PiKP({211, -321, 2212}, {mPi, mK, mP});
    TH2D ramboLc2PiKPDalitzPlot{"RAMBO_Lc2PiKP", "RAMBO_Lc2PiKP", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    TH2D ramboLc2PiKPUnweighted{"RAMBO_Lc2PiKP_Unweighted", "RAMBO_Lc2PiKP_Unweighted", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(ramboLc2PiKP, mLc, ramboLc2PiKPDalitzPlot, ramboLc2PiKPUnweighted); });
    executor.PrintExecutionSummary();
    WritePlot(ramboLc2PiKPDalitzPlot, ramboLc2PiKPUnweighted);

    Mustard::GENBOD<1, 3> genbodLc2PiKP({211, -321, 2212}, {mPi, mK, mP});
    TH2D genbodLc2PiKPDalitzPlot{"GENBOD_Lc2PiKP", "GENBOD_Lc2PiKP", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    TH2D genbodLc2PiKPUnweighted{"GENBOD_Lc2PiKP_Unweighted", "GENBOD_Lc2PiKP_Unweighted", nBin, mPiKLow, mPiKUp, nBin, mKPLow, mKPUp};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(genbodLc2PiKP, mLc, genbodLc2PiKPDalitzPlot, genbodLc2PiKPUnweighted); });
    executor.PrintExecutionSummary();
    WritePlot(genbodLc2PiKPDalitzPlot, genbodLc2PiKPUnweighted);

    using namespace Mustard::PhysicalConstant;
    Mustard::RAMBO<1, 3> ramboMu2ENN({-11, -14, 12}, {electron_mass_c2, 0, 0});
    TH2D ramboMu2ENNDalitzPlot{"RAMBO_Mu2ENN", "RAMBO_Mu2ENN", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    TH2D ramboMu2ENNUnweighted{"RAMBO_Mu2ENN_Unweighted", "RAMBO_Mu2ENN_Unweighted", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(ramboMu2ENN, muon_mass_c2, ramboMu2ENNDalitzPlot, ramboMu2ENNUnweighted); });
    executor.PrintExecutionSummary();
    WritePlot(ramboMu2ENNDalitzPlot, ramboMu2ENNUnweighted);

    Mustard::GENBOD<1, 3> genbodMu2ENN({-11, -14, 12}, {electron_mass_c2, 0, 0});
    TH2D genbodMu2ENNDalitzPlot{"GENBOD_Mu2ENN", "GENBOD_Mu2ENN", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    TH2D genbodMu2ENNUnweighted{"GENBOD_Mu2ENN_Unweighted", "GENBOD_Mu2ENN_Unweighted", nBin, 0, muc::pow<2>(muon_mass_c2), nBin, 0, muc::pow<2>(muon_mass_c2)};
    executor.Execute(nEvent, [&](auto) { FillDalitzPlot(genbodMu2ENN, muon_mass_c2, genbodMu2ENNDalitzPlot, genbodMu2ENNUnweighted); });
    executor.PrintExecutionSummary();
    WritePlot(genbodMu2ENNDalitzPlot, genbodMu2ENNUnweighted);

    return EXIT_SUCCESS;
}
