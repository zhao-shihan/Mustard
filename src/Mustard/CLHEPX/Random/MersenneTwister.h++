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

#include "Mustard/CLHEPX/Random/Wrap.h++"
#include "Mustard/Math/Random/Generator/MT1993732.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"

namespace Mustard::CLHEPX::Random {

using MT1993732 = CLHEPX::Random::Wrap<Math::Random::MT1993732>;
using MT1993764 = CLHEPX::Random::Wrap<Math::Random::MT1993764>;

} // namespace Mustard::CLHEPX::Random
