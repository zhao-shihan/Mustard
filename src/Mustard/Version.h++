// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#if __cplusplus < 202002L
#    error "Mustard need at least C++20"
#endif

#define MUSTARD_VERSION_MAJOR 0
#define MUSTARD_VERSION_MINOR 6
#define MUSTARD_VERSION_PATCH 0

#define MUSTARD_VERSION_STRING_HELPER1(major, minor, patch) major.minor.patch
#define MUSTARD_VERSION_STRING_HELPER2(version) #version
#define MUSTARD_VERSION_STRING_HELPER3(version) MUSTARD_VERSION_STRING_HELPER2(version)
#define MUSTARD_VERSION_STRING \
    MUSTARD_VERSION_STRING_HELPER3(MUSTARD_VERSION_STRING_HELPER1(MUSTARD_VERSION_MAJOR, MUSTARD_VERSION_MINOR, MUSTARD_VERSION_PATCH))
