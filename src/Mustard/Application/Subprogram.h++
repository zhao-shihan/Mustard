// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

#include <string>

namespace Mustard::Application {

class Subprogram {
public:
    Subprogram(std::string name, std::string description);
    virtual ~Subprogram() = default;

    auto Name() const -> const auto& { return fName; }
    auto Description() const -> const auto& { return fDescription; }

    virtual auto Main(int argc, char* argv[]) const -> int = 0;

private:
    std::string fName;
    std::string fDescription;
};

} // namespace Mustard::Application
