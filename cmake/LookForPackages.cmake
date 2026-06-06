# Copyright (C) 2020-2026  Shihan Zhao and contributors
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

# =============================================================================
# System-only dependencies (REQUIRED, no source fallback)
# =============================================================================

# ROOT
find_package(
    ROOT 6.36.00 REQUIRED
    Core Imt MathCore Hist Tree ROOTDataFrame ROOTNTuple RIO)

# Geant4
set(MUSTARD_REQUIRED_G4_COMPONENTS ui_all gdml)
if(MUSTARD_USE_G4VIS)
    list(APPEND MUSTARD_REQUIRED_G4_COMPONENTS vis_all)
endif()
if(MUSTARD_USE_STATIC_G4)
    list(APPEND MUSTARD_REQUIRED_G4_COMPONENTS static)
endif()
find_package(
    Geant4 11.0.0 REQUIRED
    ${MUSTARD_REQUIRED_G4_COMPONENTS})

# =============================================================================
# CPM.cmake, for package management
# =============================================================================

set(CPM_SOURCE_CACHE "${PROJECT_SOURCE_DIR}/.cache/cpm" CACHE PATH "CPM source cache directory")
file(DOWNLOAD
     https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.3/CPM.cmake
     ${PROJECT_SOURCE_DIR}/cmake/CPM.cmake
     EXPECTED_HASH SHA256=a609e875fd532b067174250f6abbc3dac22fe2d64869783fb1e80bda1625c844)
include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

# =============================================================================
# Public dependencies (try find_package first, fall back via CPM)
#   (Somewhat sorted by dependency order)
# =============================================================================

# mplr (-> MPI)
CPMFindPackage(
    NAME mplr
    GITHUB_REPOSITORY zhao-shihan/mplr
    VERSION 0.26.605
    OPTIONS "MPLR_INSTALL ON")

# Eigen
set(MUSTARD_EIGEN_MINIMUM_REQUIRED 5.0.1)
CPMFindPackage(
    NAME eigen
    GITLAB_REPOSITORY libeigen/eigen
    VERSION ${MUSTARD_EIGEN_MINIMUM_REQUIRED}
    GIT_TAG ${MUSTARD_EIGEN_MINIMUM_REQUIRED} # Eigen doesn't use standard tagging...
    OPTIONS "EIGEN_BUILD_TESTING ${BUILD_TESTING}"
            "EIGEN_BUILD_CMAKE_PACKAGE ON")

# yaml-cpp
set(MUSTARD_YAML_CPP_MINIMUM_REQUIRED 0.9.0)
CPMFindPackage(
    NAME yaml-cpp
    GITHUB_REPOSITORY jbeder/yaml-cpp
    VERSION ${MUSTARD_YAML_CPP_MINIMUM_REQUIRED}
    GIT_TAG yaml-cpp-${MUSTARD_YAML_CPP_MINIMUM_REQUIRED} # yaml-cpp doesn't use standard tagging...
    OPTIONS "YAML_CPP_BUILD_TOOLS OFF"
            "YAML_CPP_INSTALL ON"
            "YAML_CPP_FORMAT_SOURCE OFF"
            "YAML_CPP_BUILD_TESTS ${BUILD_TESTING}"
            "YAML_MSVC_SHARED_RT ${MUSTARD_USE_SHARED_MSVC_RT}")

# fmt
set(MUSTARD_FMT_MINIMUM_REQUIRED 12.1.0)
CPMFindPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    VERSION ${MUSTARD_FMT_MINIMUM_REQUIRED}
    GIT_TAG ${MUSTARD_FMT_MINIMUM_REQUIRED} # fmt doesn't use standard tagging...
    OPTIONS "FMT_INSTALL ON")

# =============================================================================
# Public light-weight dependencies (try find_package first, fall back via CPM)
#   (Sorted alphabetically)
# =============================================================================

# argparse
CPMFindPackage(
    NAME argparse
    GITHUB_REPOSITORY p-ranav/argparse
    VERSION 3.2
    OPTIONS "ARGPARSE_INSTALL ON")

# envparse
CPMFindPackage(
    NAME envparse
    GITHUB_REPOSITORY zhao-shihan/envparse
    VERSION 1.0.0
    OPTIONS "ENVPARSE_INSTALL ON")

# gtl
CPMFindPackage(
    NAME gtl
    GITHUB_REPOSITORY zhao-shihan/gtl
    VERSION 1.2.1
    OPTIONS "GTL_INSTALL ON")

# Microsoft.GSL
CPMFindPackage(
    NAME Microsoft.GSL
    GITHUB_REPOSITORY microsoft/GSL
    VERSION 4.2.2
    OPTIONS "GSL_INSTALL ON")

# muc
CPMFindPackage(
    NAME muc
    GITHUB_REPOSITORY zhao-shihan/muc
    VERSION 0.26.602
    OPTIONS "MUC_INSTALL ON")

# =============================================================================
# Private dependencies (try find_package first, fall back via CPM)
#   (Somewhat sorted by dependency order)
# =============================================================================

# backward-cpp
CPMFindPackage(
    NAME Backward
    GITHUB_REPOSITORY zhao-shihan/backward-cpp
    VERSION 1.6.1
    OPTIONS "BACKWARD_SHARED OFF"
            "BACKWARD_INSTALL ON"
            "BACKWARD_TESTS ${BUILD_TESTING}")

# indicators
CPMFindPackage(
    NAME indicators
    GITHUB_REPOSITORY zhao-shihan/indicators
    VERSION 2.3.2
    OPTIONS "INDICATORS_INSTALL ON" "INDICATORS_PACKAGE OFF")

# mimalloc
if(MUSTARD_USE_MIMALLOC)
    CPMFindPackage(
        NAME mimalloc
        GITHUB_REPOSITORY microsoft/mimalloc
        VERSION 3.2.8
        OPTIONS "MI_OVERRIDE OFF"
                "MI_OSX_INTERPOSE OFF"
                "MI_OSX_ZONE OFF"
                "MI_BUILD_SHARED OFF"
                "MI_BUILD_OBJECT OFF"
                "MI_BUILD_TESTS ${BUILD_TESTING}")
endif()
