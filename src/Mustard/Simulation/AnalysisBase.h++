// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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

#pragma once

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/ObjectRegistry/PassiveSingleton.h++"
#include "Mustard/Geant4X/Utility/ConvertGeometry.h++"
#include "Mustard/IO/File.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Simulation/AnalysisBaseMessenger.h++"

#include "TMacro.h"

#include "muc/ceta_string"

#include "gsl/gsl"

#include "fmt/format.h"

#include <concepts>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <utility>

namespace Mustard::Simulation {

template<typename ADerived, muc::ceta_string AAppName>
class AnalysisBase : public Env::PassiveSingleton<ADerived> {
public:
    AnalysisBase(ADerived* self);
    virtual ~AnalysisBase() = default;

    auto FilePath(std::filesystem::path path) -> void { fFilePath = std::move(path); }
    auto FileMode(std::string mode) -> void { fFileMode = std::move(mode); }

    auto RunBeginAction(int runID) -> void;
    auto EventEndAction() -> void;
    auto RunEndAction(int runID) -> void;

private:
    virtual auto RunBeginUserAction(int runID) -> void = 0;
    virtual auto EventEndUserAction() -> void = 0;
    virtual auto RunEndUserAction(int runID) -> void = 0;

private:
    std::filesystem::path fFilePath;
    std::string fFileMode;

    std::filesystem::path fLastUsedFullFilePath;
    std::optional<ProcessSpecificFile<TFile>> fFile;

    AnalysisBaseMessenger<ADerived>::template Register<ADerived> fMessengerRegister;
};

} // namespace Mustard::Simulation

#include "Mustard/Simulation/AnalysisBase.inl"
