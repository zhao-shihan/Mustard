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

file(GLOB_RECURSE MUSTARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/*.c++)
add_library(Mustard ${MUSTARD_SRC})
add_library(Mustard::Mustard ALIAS Mustard)

# Mustard
target_include_directories(Mustard PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_OPTIONS}
                               PRIVATE ${MUSTARD_PRIVATE_COMPILE_OPTIONS})
target_compile_definitions(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_DEFINITIONS}
                                   PRIVATE ${MUSTARD_PRIVATE_COMPILE_DEFINITIONS})

# Generate version header
include(CMakePackageConfigHelpers)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Mustard/Version.inl.in
               ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/Version.inl
               @ONLY)

# Required dependencies

# Public dependencies
# fmt
target_link_libraries(Mustard PUBLIC fmt::fmt)
# Geant4
target_link_libraries(Mustard PUBLIC ${Geant4_LIBRARIES})
# ROOT
target_link_libraries(Mustard PUBLIC ROOT::Core ROOT::Imt ROOT::MathCore ROOT::Hist ROOT::Tree ROOT::ROOTDataFrame ROOT::RIO)
# yaml-cpp
target_link_libraries(Mustard PUBLIC yaml-cpp::yaml-cpp)

# Public header-only dependencies
# argparse
target_link_libraries(Mustard PUBLIC argparse::argparse)
# EFM
target_link_libraries(Mustard PUBLIC EFM::EFM)
# Eigen
target_link_libraries(Mustard PUBLIC Eigen3::Eigen)
# envparse
target_link_libraries(Mustard PUBLIC envparse::envparse)
# gtl
target_link_libraries(Mustard PUBLIC gtl)
# Microsoft.GSL
target_link_libraries(Mustard PUBLIC Microsoft.GSL::GSL)
# MPLR
target_link_libraries(Mustard PUBLIC mplr::mplr)
# muc
target_link_libraries(Mustard PUBLIC muc::muc)

# Private dependencies
# backward-cpp
target_link_libraries(Mustard PRIVATE backward_interface)
# indicators
target_link_libraries(Mustard PRIVATE indicators::indicators)

# Conditional dependencies

if(MUSTARD_USE_MIMALLOC)
    # mimalloc
    target_link_libraries(Mustard PRIVATE mimalloc-static)
endif()
