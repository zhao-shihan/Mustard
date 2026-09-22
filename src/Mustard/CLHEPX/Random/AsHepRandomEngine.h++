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

#include "Mustard/IO/File.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Math/Random/Distribution/Uniform.h++"
#include "Mustard/Math/Random/RandomNumberEngineHolder.h++"
#include "Mustard/Utility/ClassSemanticHelper.h++"

#include "CLHEP/Random/RandomEngine.h"

#include "muc/math"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/ostream.h"
#include "fmt/std.h"

#include <algorithm>
#include <concepts>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>

namespace Mustard::CLHEPX::Random {

/// @brief Adapter for Mustard RNGs to CLHEP random engine interface
///
/// Provides full compatibility with CLHEP's HepRandomEngine API for any
/// standard-compliant random number engine, which is either owned by this
/// adapter or referred to by it.
///
/// @tparam R A random number engine type (this adapter owns an engine of that
/// type) or an lvalue reference to one (this adapter refers to an engine
/// belonging to someone else).
///
/// @note Implements all CLHEP engine virtual methods except getState().
/// @note Not copyable nor movable: an adapter is bound to the engine it owns or
/// refers to, and CLHEP's seed back-pointer cannot be transferred with it.
/// @note The CLHEP seed record, i.e. getSeed() and getSeeds(), reports the seed
/// this adapter was constructed or setSeed()/setSeeds()-ed with, and 0 as long
/// as it has not been told any seed. A wrapper referring to an external engine
/// cannot know how that engine was seeded, and the engine may be reseeded
/// behind its back at any time.
/// @note The seed record is not updated when the engine state is restored by
/// get()/restoreStatus(), nor when the engine is reseeded through another
/// adapter referring to it. getSeeds() points to this adapter's own
/// zero-terminated, one-element seed array, since only the first seed is ever
/// used.
/// @warning getState() is intentionally non-functional.
template<Math::Random::RandomNumberEngineOrReference R>
class AsHepRandomEngine : public CLHEP::HepRandomEngine,
                          public Math::Random::RandomNumberEngineHolder<R>,
                          public Utility::NoncopyableBase {
private:
    using Base = Math::Random::RandomNumberEngineHolder<R>;

public:
    using Base::Engine;
    using typename Base::EngineType;
    using typename Base::SeedType;

    /// @brief Construct an adapter owning a default-state engine.
    /// @note Only available when @c R is an engine type.
    AsHepRandomEngine()
        requires(not std::is_reference_v<R>);
    /// @brief Construct an adapter owning an engine seeded with @p seed.
    /// @param seed Initialization seed of the engine.
    /// @note Only available when @c R is an engine type.
    explicit AsHepRandomEngine(SeedType seed)
        requires(not std::is_reference_v<R>);
    /// @brief Construct an adapter referring to @p engine.
    /// @param engine Engine to refer to.
    /// @pre @p engine outlives this adapter.
    /// @note Only available when @c R is an engine reference type.
    explicit AsHepRandomEngine(EngineType& engine)
        requires(std::is_reference_v<R>);

    /// @brief Generate uniform double in (0, 1).
    virtual auto flat() -> double override { return DrawOpen01<double>(); }
    /// @brief Fill array with uniform doubles.
    /// @param size Number of values to generate
    /// @param vect Pre-allocated output buffer
    virtual auto flatArray(const int size, double* vect) -> void override;

    /// @brief Seed the engine (single seed).
    /// @param seed New seed value
    /// @param (ignored) Stream index parameter (CLHEP compatibility)
    virtual auto setSeed(long seed, int = 0) -> void override;
    /// @brief Seed the engine (seed array).
    /// @param seeds Pointer to seed array (uses first element)
    /// @param (ignored) Stream index parameter
    /// @note The array is read but not retained, since this adapter is seeded
    /// from a single value only.
    virtual auto setSeeds(const long* seeds, int = 0) -> void override;

    /// @brief Save engine state to file.
    /// @param filename Output file path
    virtual auto saveStatus(gsl::czstring filename) const -> void override;
    /// @brief Restore engine state from file.
    /// @param filename Input file path
    virtual auto restoreStatus(gsl::czstring filename) -> void override;
    /// @brief Print engine status to stdout.
    virtual auto showStatus() const -> void override;

    /// @brief Get engine type name.
    /// @return Demangled type name of the wrapped engine
    virtual auto name() const -> std::string override { return muc::try_demangle(typeid(EngineType).name()); }

    /// @brief Serialize engine state to output stream.
    /// @note The state is tagged with the engine type name rather than with this
    /// adapter type, so any adapter over the same engine type can read it, and
    /// state written by an older version with a different tag is rejected.
    virtual auto put(std::ostream& os) const -> decltype(os) override;
    /// @brief Deserialize engine state from input stream.
    /// @note See put() for the state tag.
    virtual auto get(std::istream& is) -> decltype(is) override;

    /// @name Random-number-generating conversion operators
    /// @{
    virtual operator double() override { return DrawOpen01<double>(); }
    virtual operator float() override { return DrawOpen01<float>(); }
    virtual operator unsigned int() override { return Engine()(); }
    /// @}

private:
    /// @brief Draw a uniform value on the open interval (0, 1).
    /// @tparam T Floating-point type of the value.
    /// @return A value strictly between 0 and 1, as CLHEP requires: an exact 0,
    /// which the wrapped engines can produce, is drawn again.
    template<std::floating_point T>
    auto DrawOpen01() -> T;

private:
    /// @brief [Disabled] CLHEP state restoration method
    /// @return Input stream unchanged
    /// @warning DO NOT USE.
    virtual auto getState(std::istream& is) -> decltype(is) override;

private:
    /// @brief Record @p seed as the seed reported by getSeed() and getSeeds().
    /// @param seed Seed to record; 0 means unknown.
    auto RecordSeed(long seed) -> void;

private:
    long fSeedArray[2]; ///< One-element seed array reported by getSeeds().
};

} // namespace Mustard::CLHEPX::Random

#include "Mustard/CLHEPX/Random/AsHepRandomEngine.inl"
