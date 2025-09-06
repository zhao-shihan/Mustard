# Copyright (C) 2020-2025  The Mustard development team
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

message(STATUS "Looking for Geant4")

set(MUSTARD_G4_MINIMUM_REQUIRED 11.0.0)

find_package(Geant4 ${MUSTARD_G4_MINIMUM_REQUIRED} REQUIRED) # to load Geant4Config.cmake

set(MUSTARD_G4_REQUIRED_COMPONENTS ui_all gdml)

if(MUSTARD_USE_G4VIS)
    list(APPEND MUSTARD_G4_REQUIRED_COMPONENTS vis_all)
endif()

if(MUSTARD_USE_STATIC_G4)
    if(Geant4_static_FOUND)
        list(APPEND MUSTARD_G4_REQUIRED_COMPONENTS static)
        message(STATUS "Geant4 static libraries will be linked")
    else()
        message(NOTICE "***Notice: MUSTARD_USE_STATIC_G4 is ON but Geant4-static not found. Dynamic libraries will be linked")
    endif()
endif()

find_package(Geant4 ${MUSTARD_G4_MINIMUM_REQUIRED} REQUIRED ${MUSTARD_G4_REQUIRED_COMPONENTS})

message(STATUS "Looking for Geant4 - found (version: ${Geant4_VERSION})")
