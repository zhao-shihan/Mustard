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

//
// MUSTARD_STRONG_INLINE
//
#if defined _MSC_VER
#    define MUSTARD_STRONG_INLINE __forceinline
#else
#    define MUSTARD_STRONG_INLINE inline
#endif

//
// MUSTARD_ALWAYS_INLINE
//
// It can slow the program in some cases (especially for large functions).
// Use with profile and with care. If you're not sure what you're doing, don't use it.
//
#if defined __clang__
#    define MUSTARD_ALWAYS_INLINE [[clang::always_inline]] inline
#elif defined __GNUC__
#    define MUSTARD_ALWAYS_INLINE [[gnu::always_inline]] inline
#else
#    define MUSTARD_ALWAYS_INLINE MUSTARD_STRONG_INLINE
#endif

//
// MUSTARD_NOINLINE
//
#if defined __clang__
#    define MUSTARD_NOINLINE [[clang::noinline]]
#elif defined __GNUC__
#    define MUSTARD_NOINLINE [[gnu::noinline]]
#elif defined _MSC_VER
#    define MUSTARD_NOINLINE __declspec(noinline)
#endif

//
// MUSTARD_OPTIMIZE_FAST
//
// It can slow the program or produce inexact result in some cases.
// It can prevents inlining inside the function body, so only use on function with direct + - * / operation.
// Use with profile and with care. If you're not sure what you're doing, don't use it.
//
#ifdef NDEBUG
#    if defined __clang__
#        define MUSTARD_OPTIMIZE_FAST
#    elif defined __GNUC__
#        define MUSTARD_OPTIMIZE_FAST [[gnu::optimize("Ofast")]]
#    endif
#else
#    define MUSTARD_OPTIMIZE_FAST
#endif
