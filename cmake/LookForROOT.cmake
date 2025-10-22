# Copyright (C) 2020-2025  Mustard developers
#
# This file is part of Mustard, an offline software framework for HEP experiments.
#
# Mustard is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# Mustard. If not, see <https://www.gnu.org/licenses/>.

message(STATUS "Looking for ROOT")

set(MUSTARD_ROOT_MINIMUM_REQUIRED 6.30.00)
set(MUSTARD_ROOT_REQUIRED_COMPONENTS Core Hist Thread Tree MathCore ROOTDataFrame Geom)
find_package(ROOT 6.30.00 REQUIRED ${MUSTARD_ROOT_REQUIRED_COMPONENTS})

message(STATUS "Looking for ROOT - found (version: ${ROOT_VERSION})")
