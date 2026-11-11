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

# Avoid reusing packages registered in the CMake user package registry (e.g. a
# build-tree Eigen3 left behind by a previous build directory).
set(CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY ON)

# =============================================================================
# System-only packages (REQUIRED, no source fallback)
# =============================================================================

# ROOT
set(MUSTARD_ROOT_MINIMUM_REQUIRED 6.36.00)
set(MUSTARD_ROOT_REQUIRED_COMPONENTS Core Imt MathCore Hist Tree ROOTDataFrame ROOTNTuple RIO)
find_package(
    ROOT ${MUSTARD_ROOT_MINIMUM_REQUIRED} REQUIRED
    COMPONENTS ${MUSTARD_ROOT_REQUIRED_COMPONENTS})

# Geant4
set(MUSTARD_G4_MINIMUM_REQUIRED 11.0.0)
set(MUSTARD_G4_REQUIRED_COMPONENTS ui_all gdml)
if(MUSTARD_USE_G4VIS)
    list(APPEND MUSTARD_G4_REQUIRED_COMPONENTS vis_all)
endif()
if(MUSTARD_USE_STATIC_G4)
    list(APPEND MUSTARD_G4_REQUIRED_COMPONENTS static)
endif()
find_package(
    Geant4 ${MUSTARD_G4_MINIMUM_REQUIRED} REQUIRED
    COMPONENTS ${MUSTARD_G4_REQUIRED_COMPONENTS})

# =============================================================================
# CPM.cmake, for package management
# =============================================================================

file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.3/CPM.cmake
    ${MUSTARD_CMAKE_MODULE_PATH}/CPM.cmake
    EXPECTED_HASH SHA256=a609e875fd532b067174250f6abbc3dac22fe2d64869783fb1e80bda1625c844)
set(CPM_SOURCE_CACHE "${PROJECT_SOURCE_DIR}/.cache/cpm" CACHE PATH "CPM source cache directory")
include(CPM)

# =============================================================================
# Public dependencies (somewhat sorted by dependency order)
# =============================================================================

# mplr (-> MPI)
set(MUSTARD_MPLR_MINIMUM_REQUIRED 0.26.1110)
CPMFindPackage(
    NAME mplr
    GITHUB_REPOSITORY zhao-shihan/mplr
    VERSION ${MUSTARD_MPLR_MINIMUM_REQUIRED}
    GIT_TAG v0.26.1111 # The project version inside the tag is still 0.26.1110
    OPTIONS "MPLR_INSTALL ON")

# FFTW
if(MUSTARD_USE_FFTW)
    file(DOWNLOAD
        https://github.com/egpbos/findFFTW/raw/d449ea0bcbf94a4a1c3dbb2108aa57609a4967ff/FindFFTW.cmake
        ${MUSTARD_CMAKE_MODULE_PATH}/FindFFTW.cmake
        EXPECTED_HASH SHA256=56a669b2496797b9214a60c3c2e0c36be9292d8be0cd5b5c3c56320503e2c2ff)
    set(MUSTARD_FFTW_MINIMUM_REQUIRED 3.3.11)
    CPMFindPackage(
        NAME FFTW
        VERSION ${MUSTARD_FFTW_MINIMUM_REQUIRED}
        URL "https://fftw.org/fftw-${MUSTARD_FFTW_MINIMUM_REQUIRED}.tar.gz"
        OPTIONS "CMAKE_POLICY_VERSION_MINIMUM 3.5" # Remove this option in next FFTW release!
        "BUILD_TESTS ${BUILD_TESTING}"
        "DISABLE_FORTRAN ON")
endif()

# Eigen
set(MUSTARD_EIGEN_MINIMUM_REQUIRED 5.0.0)
CPMFindPackage(
    NAME Eigen3
    COMPONENTS unsupported
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
set(MUSTARD_FMT_MINIMUM_REQUIRED 12.2.1)
CPMFindPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    VERSION ${MUSTARD_FMT_MINIMUM_REQUIRED}
    GIT_TAG ${MUSTARD_FMT_MINIMUM_REQUIRED} # fmt doesn't use standard tagging...
    OPTIONS "FMT_INSTALL ON")

# =============================================================================
# Public light-weight dependencies (sorted alphabetically)
# =============================================================================

# argparse
set(MUSTARD_ARGPARSE_MINIMUM_REQUIRED 3.2)
CPMFindPackage(
    NAME argparse
    GITHUB_REPOSITORY p-ranav/argparse
    VERSION ${MUSTARD_ARGPARSE_MINIMUM_REQUIRED}
    OPTIONS "ARGPARSE_INSTALL ON")

# envparse
set(MUSTARD_ENVPARSE_MINIMUM_REQUIRED 1.0.0)
CPMFindPackage(
    NAME envparse
    GITHUB_REPOSITORY zhao-shihan/envparse
    VERSION ${MUSTARD_ENVPARSE_MINIMUM_REQUIRED}
    OPTIONS "ENVPARSE_INSTALL ON")

# gtl
set(MUSTARD_GTL_MINIMUM_REQUIRED 1.3.1)
CPMFindPackage(
    NAME gtl
    GITHUB_REPOSITORY greg7mdp/gtl
    VERSION ${MUSTARD_GTL_MINIMUM_REQUIRED}
    OPTIONS "GTL_INSTALL ON")

# Microsoft.GSL
set(MUSTARD_MSGSL_MINIMUM_REQUIRED 5.0.1)
CPMFindPackage(
    NAME Microsoft.GSL
    GITHUB_REPOSITORY microsoft/GSL
    VERSION ${MUSTARD_MSGSL_MINIMUM_REQUIRED}
    OPTIONS "GSL_INSTALL ON")

# muc
set(MUSTARD_MUC_MINIMUM_REQUIRED 0.26.925)
CPMFindPackage(
    NAME muc
    GITHUB_REPOSITORY zhao-shihan/muc
    VERSION ${MUSTARD_MUC_MINIMUM_REQUIRED}
    OPTIONS "MUC_INSTALL ON")

# =============================================================================
# Private dependencies (somewhat sorted by dependency order)
# =============================================================================

# backward-cpp
CPMFindPackage(
    NAME Backward
    GITHUB_REPOSITORY zhao-shihan/backward-cpp
    VERSION 1.6.3
    OPTIONS "BACKWARD_SHARED OFF"
    "BACKWARD_INSTALL ON"
    "BACKWARD_TESTS ${BUILD_TESTING}")

# indicators
set(MUSTARD_INDICATORS_MINIMUM_REQUIRED 2.3.2)
CPMFindPackage(
    NAME indicators
    GITHUB_REPOSITORY zhao-shihan/indicators
    VERSION ${MUSTARD_INDICATORS_MINIMUM_REQUIRED}
    OPTIONS "INDICATORS_INSTALL ON" "INDICATORS_PACKAGE OFF")

# mimalloc
set(MUSTARD_MIMALLOC_MINIMUM_REQUIRED 3.3.2)
if(MUSTARD_USE_MIMALLOC)
    CPMFindPackage(
        NAME mimalloc
        GITHUB_REPOSITORY microsoft/mimalloc
        VERSION ${MUSTARD_MIMALLOC_MINIMUM_REQUIRED}
        OPTIONS "MI_OVERRIDE OFF"
        "MI_OSX_INTERPOSE OFF"
        "MI_OSX_ZONE OFF"
        "MI_BUILD_SHARED OFF"
        "MI_BUILD_OBJECT OFF"
        "MI_BUILD_TESTS ${BUILD_TESTING}")
endif()
