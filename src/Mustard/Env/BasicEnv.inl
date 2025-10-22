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

namespace Mustard::Env {

template<char L>
    requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
MUSTARD_ALWAYS_INLINE auto BasicEnv::VerboseLevelReach() const -> bool {
    // if constexpr (L == 'Q') { return fVerboseLevel >= VerboseLevel::Quiet; }
    if constexpr (L == 'E') {
        return fVerboseLevel >= VerboseLevel::Error;
    }
    if constexpr (L == 'W') {
        return fVerboseLevel >= VerboseLevel::Warning;
    }
    if constexpr (L == 'I') {
        return fVerboseLevel >= VerboseLevel::Informative;
    }
    if constexpr (L == 'V') {
        return fVerboseLevel >= VerboseLevel::Verbose;
    }
}

template<char L>
    requires(L == 'E' or L == 'W' or L == 'I' or L == 'V')
MUSTARD_ALWAYS_INLINE auto VerboseLevelReach() -> bool {
    if (not BasicEnv::Available()) [[unlikely]] {
        return true;
    }
    return BasicEnv::Instance().VerboseLevelReach<L>();
}

} // namespace Mustard::Env
