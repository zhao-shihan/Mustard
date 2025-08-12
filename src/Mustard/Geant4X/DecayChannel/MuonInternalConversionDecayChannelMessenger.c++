// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#include "Mustard/Geant4X/DecayChannel/MuonInternalConversionDecayChannel.h++"
#include "Mustard/Geant4X/DecayChannel/MuonInternalConversionDecayChannelMessenger.h++"
#include "Mustard/IO/Print.h++"
#include "Mustard/Parallel/ReseedRandomEngine.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

#include "mplr/mplr.hpp"

#include "fmt/core.h"

#include <sstream>
#include <string>

namespace Mustard::Geant4X::inline DecayChannel {

MuonInternalConversionDecayChannelMessenger::MuonInternalConversionDecayChannelMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fMetropolisDelta{},
    fMetropolisDiscard{},
    fInitialize{},
    fEstimateWeightNormalizationFactor{} {

    fDirectory = std::make_unique<G4UIdirectory>("/Mustard/Physics/MuonDecay/ICDecay/");
    fDirectory->SetGuidance("Muon(ium) internal pair production decay channel (mu->eeevv / M->eeevve).");

    fMetropolisDelta = std::make_unique<G4UIcmdWithADouble>("/Mustard/Physics/MuonDecay/ICDecay/MetropolisDelta", this);
    fMetropolisDelta->SetGuidance("Set the 1D-displacement (20 dimensions in total) of the random walk in the Metropolis algorithm. "
                                  "Smaller values enhance autocorrelation, while larger values decrease performance and lead to biased results. "
                                  "The typical value is below 0.05.");
    fMetropolisDelta->SetParameterName("delta", false);
    fMetropolisDelta->SetRange("0 < delta && delta < 0.5");
    fMetropolisDelta->AvailableForStates(G4State_Idle);

    fMetropolisDiscard = std::make_unique<G4UIcmdWithAnInteger>("/Mustard/Physics/MuonDecay/ICDecay/MetropolisDiscard", this);
    fMetropolisDiscard->SetGuidance("Set how many samples are discarded between two outputs in the Metropolis algorithm. "
                                    "The more samples are discarded, the less significant the autocorrelation is, but it will reduce the performance of sampling. "
                                    "When the total number of samples (number of IPP decay events) is small, a larger number of discards should be set. "
                                    "When the number of samples is large, the number of discards can be appropriately reduced. "
                                    "This value is also related to the delta of the random walk, and the smaller the delta, the more samples that should be discarded.");
    fMetropolisDiscard->SetParameterName("n", false);
    fMetropolisDiscard->SetRange("n >= 0");
    fMetropolisDiscard->AvailableForStates(G4State_Idle);

    fInitialize = std::make_unique<G4UIcmdWithoutParameter>("/Mustard/Physics/MuonDecay/Initialize", this);
    fInitialize->SetGuidance("Manaually (re)initialize random state.");
    fInitialize->AvailableForStates(G4State_Idle);

    fEstimateWeightNormalizationFactor = std::make_unique<G4UIcommand>("/Mustard/Physics/MuonDecay/ICDecay/EstimateWeightNormalizationFactor", this);
    fEstimateWeightNormalizationFactor->SetGuidance("Estimate the weight normalization factor with error of the user-defined bias by 1000*n samples.");
    fEstimateWeightNormalizationFactor->SetParameter(new G4UIparameter{"kinematics_name", 's', false});
    fEstimateWeightNormalizationFactor->SetParameter(new G4UIparameter{"parent_name", 's', false});
    fEstimateWeightNormalizationFactor->SetParameter(new G4UIparameter{"n_kilo_sample", 'l', false});
    fEstimateWeightNormalizationFactor->SetRange("n_kilo_sample >= 0");
    fEstimateWeightNormalizationFactor->AvailableForStates(G4State_Idle);
}

MuonInternalConversionDecayChannelMessenger::~MuonInternalConversionDecayChannelMessenger() = default;

auto MuonInternalConversionDecayChannelMessenger::SetNewValue(G4UIcommand* command, G4String value) -> void {
    if (command == fMetropolisDelta.get()) {
        Deliver<MuonInternalConversionDecayChannel>([&](auto&& r) {
            r.MetropolisDelta(fMetropolisDelta->GetNewDoubleValue(value));
        });
    } else if (command == fMetropolisDiscard.get()) {
        Deliver<MuonInternalConversionDecayChannel>([&](auto&& r) {
            r.MetropolisDiscard(fMetropolisDiscard->GetNewIntValue(value));
        });
    } else if (command == fInitialize.get()) {
        Deliver<MuonInternalConversionDecayChannel>([&](auto&& r) {
            r.Initialize();
        });
    } else if (command == fEstimateWeightNormalizationFactor.get()) {
        std::string kinematicsName;
        std::string parentName;
        unsigned long long nKiloSample;
        std::istringstream is{value};
        is >> kinematicsName >> parentName >> nKiloSample;
        Deliver<MuonInternalConversionDecayChannel>([&](auto&& r) {
            if (r.GetKinematicsName() == kinematicsName and r.GetParentName() == parentName) {
                Parallel::ReseedRandomEngine();
                const auto nSample{1000 * nKiloSample};
                MasterPrintLn("Estimating mu->eeevv weight normalization factor with {} samples...", nSample);
                const auto [result, error, nEff]{r.EstimateWeightNormalizationFactor(nSample)};
                MasterPrintLn("Weight normalization factor of user-defined bias on mu->eeevv ({} decay according to {}):\n"
                              "    {} +/- {}\n"
                              "      rel. err. = {:.2}% ,  N_eff = {:.2f}\n"
                              "(Multiply event weights with this factor to normalize weights to the number of generated events)",
                              parentName, kinematicsName, result, error, error / result * 100., nEff);
            }
        });
    }
}

} // namespace Mustard::Geant4X::inline DecayChannel
