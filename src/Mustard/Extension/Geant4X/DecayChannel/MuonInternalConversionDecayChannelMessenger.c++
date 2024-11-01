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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalConversionDecayChannel.h++"
#include "Mustard/Extension/Geant4X/DecayChannel/MuonInternalConversionDecayChannelMessenger.h++"

#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"

#include "fmt/core.h"

#include <sstream>
#include <string>

namespace Mustard::inline Extension::Geant4X::inline DecayChannel {

MuonInternalConversionDecayChannelMessenger::MuonInternalConversionDecayChannelMessenger() :
    SingletonMessenger{},
    fDirectory{},
    fMetropolisDelta{},
    fMetropolisDiscard{},
    fInitialize{},
    fEstimateBiasScale{} {

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

    fEstimateBiasScale = std::make_unique<G4UIcommand>("/Mustard/Physics/MuonDecay/ICDecay/EstimateBiasScale", this);
    fEstimateBiasScale->SetGuidance("Estimate the bias scale with error of the user-defined bias with 1000*n samples.");
    fEstimateBiasScale->SetParameter(new G4UIparameter{"kinematics_name", 's', false});
    fEstimateBiasScale->SetParameter(new G4UIparameter{"parent_name", 's', false});
    fEstimateBiasScale->SetParameter(new G4UIparameter{"n_kilo_sample", 'l', false});
    fEstimateBiasScale->SetRange("n_kilo_sample >= 0");
    fEstimateBiasScale->AvailableForStates(G4State_Idle);
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
    } else if (command == fEstimateBiasScale.get()) {
        std::string kinematicsName;
        std::string parentName;
        unsigned long long nKiloSample;
        std::istringstream is{value};
        is >> kinematicsName >> parentName >> nKiloSample;
        Deliver<MuonInternalConversionDecayChannel>([&](auto&& r) {
            if (r.GetKinematicsName() == kinematicsName and r.GetParentName() == parentName) {
                const auto [scale, error, nEff]{r.EstimateBiasScale(1000 * nKiloSample)};
                if (Env::MPIEnv::Instance().OnCommWorldMaster()) {
                    fmt::println("Bias scale of user-defined bias on mu->eeevv ({} decay according to {}):\n"
                                 "    {} +/- {}\n"
                                 "      rel. err. = {:.2}% ,  N_eff = {:.2f}\n"
                                 "(Multiply event weights with this bias scale to normalize the histogram to number of events)",
                                 parentName, kinematicsName, scale, error, error / scale * 100., nEff);
                }
            }
        });
    }
}

} // namespace Mustard::inline Extension::Geant4X::inline DecayChannel
