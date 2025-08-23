// -*- C++ -*-
//
// Copyright 2020-2025  The Mustard development team
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
#include "Mustard/Math/Random/Generator/Xoshiro256PP.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256SS.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512PP.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512SS.h++"

namespace Mustard::CLHEPX::Random {

using Xoshiro256SS = CLHEPX::Random::Wrap<Math::Random::Xoshiro256SS>;
using Xoshiro256PP = CLHEPX::Random::Wrap<Math::Random::Xoshiro256PP>;
using Xoshiro256Plus = CLHEPX::Random::Wrap<Math::Random::Xoshiro256Plus>;
using Xoshiro512SS = CLHEPX::Random::Wrap<Math::Random::Xoshiro512SS>;
using Xoshiro512PP = CLHEPX::Random::Wrap<Math::Random::Xoshiro512PP>;
using Xoshiro512Plus = CLHEPX::Random::Wrap<Math::Random::Xoshiro512Plus>;

} // namespace Mustard::CLHEPX::Random
