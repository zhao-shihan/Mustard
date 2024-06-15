#pragma once

#include "Mustard/Extension/CLHEPX/Random/Wrap.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PP.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256SS.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512PP.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512SS.h++"

namespace Mustard::inline Extension::CLHEPX::Random {

using Xoshiro256SS = CLHEPX::Random::Wrap<Math::Random::Xoshiro256SS>;
using Xoshiro256PP = CLHEPX::Random::Wrap<Math::Random::Xoshiro256PP>;
using Xoshiro256Plus = CLHEPX::Random::Wrap<Math::Random::Xoshiro256Plus>;
using Xoshiro512SS = CLHEPX::Random::Wrap<Math::Random::Xoshiro512SS>;
using Xoshiro512PP = CLHEPX::Random::Wrap<Math::Random::Xoshiro512PP>;
using Xoshiro512Plus = CLHEPX::Random::Wrap<Math::Random::Xoshiro512Plus>;

} // namespace Mustard::inline Extension::CLHEPX::Random
