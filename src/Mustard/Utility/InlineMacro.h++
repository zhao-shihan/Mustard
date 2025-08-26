// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

#if defined _MSC_VER
#    define MUSTARD_STRONG_INLINE __forceinline
#else
#    define MUSTARD_STRONG_INLINE inline
#endif

#if defined __clang__
#    define MUSTARD_ALWAYS_INLINE [[clang::always_inline]] inline
#elif defined __GNUC__
#    define MUSTARD_ALWAYS_INLINE [[gnu::always_inline]] inline
#else
#    define MUSTARD_ALWAYS_INLINE MUSTARD_STRONG_INLINE
#endif

#if defined __clang__
#    define MUSTARD_NOINLINE [[clang::noinline]]
#elif defined __GNUC__
#    define MUSTARD_NOINLINE [[gnu::noinline]]
#elif defined _MSC_VER
#    define MUSTARD_NOINLINE __declspec(noinline)
#endif
