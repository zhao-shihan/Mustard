#pragma once

#include "Mustard/Extension/CLHEPX/Random/Wrap.h++"
#include "Mustard/Math/Random/Generator/MT1993732.h++"
#include "Mustard/Math/Random/Generator/MT1993764.h++"

namespace Mustard::inline Extension::CLHEPX::Random {

using MT1993732 = CLHEPX::Random::Wrap<Math::Random::MT1993732>;
using MT1993764 = CLHEPX::Random::Wrap<Math::Random::MT1993764>;

} // namespace Mustard::inline Extension::CLHEPX::Random
