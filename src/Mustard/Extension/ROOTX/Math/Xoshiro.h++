#pragma once

#include "Mustard/Extension/ROOTX/Math/AsTRandom.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256PP.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro256SS.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512PP.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512Plus.h++"
#include "Mustard/Math/Random/Generator/Xoshiro512SS.h++"

namespace Mustard::inline Extension::ROOTX::Math {

using Xoshiro256SS = ROOTX::Math::AsTRandom<Mustard::Math::Random::Xoshiro256SS>;
using Xoshiro256PP = ROOTX::Math::AsTRandom<Mustard::Math::Random::Xoshiro256PP>;
using Xoshiro256Plus = ROOTX::Math::AsTRandom<Mustard::Math::Random::Xoshiro256Plus>;
using Xoshiro512SS = ROOTX::Math::AsTRandom<Mustard::Math::Random::Xoshiro512SS>;
using Xoshiro512PP = ROOTX::Math::AsTRandom<Mustard::Math::Random::Xoshiro512PP>;
using Xoshiro512Plus = ROOTX::Math::AsTRandom<Mustard::Math::Random::Xoshiro512Plus>;

} // namespace Mustard::inline Extension::ROOTX::Math
