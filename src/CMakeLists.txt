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

file(GLOB_RECURSE MUSTARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/*.c++)
add_library(Mustard ${MUSTARD_SRC})
add_library(Mustard::Mustard ALIAS Mustard)

# Mustard
target_include_directories(Mustard PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_OPTIONS}
                               PRIVATE ${MUSTARD_PRIVATE_COMPILE_OPTIONS})
target_compile_definitions(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_DEFINITIONS})

# Generate version header
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Mustard/Version.inl.in
                              ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/Version.inl
                              INSTALL_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Mustard)

# envparse
target_link_libraries(Mustard PUBLIC envparse::envparse)

# muc
target_link_libraries(Mustard PUBLIC muc::muc)

# EFM
target_link_libraries(Mustard PUBLIC EFM::EFM)

# backward-cpp
target_link_libraries(Mustard PUBLIC Backward::Interface)

# argparse
target_link_libraries(Mustard PUBLIC argparse::argparse)

# indicators
target_link_libraries(Mustard PUBLIC indicators::indicators)

# MPLR
target_link_libraries(Mustard PUBLIC mplr::mplr)

# Microsoft.GSL
target_link_libraries(Mustard PUBLIC Microsoft.GSL::GSL)

# fmt
target_link_libraries(Mustard PUBLIC fmt::fmt)

# Eigen
target_link_libraries(Mustard PUBLIC Eigen3::Eigen)

# yaml-cpp
target_link_libraries(Mustard PUBLIC yaml-cpp::yaml-cpp)

# Geant4
target_link_libraries(Mustard PUBLIC ${Geant4_LIBRARIES})

# ROOT
target_link_libraries(Mustard PUBLIC ROOT::Core ROOT::Hist ROOT::Thread ROOT::Tree ROOT::MathCore ROOT::ROOTDataFrame ROOT::Geom)
