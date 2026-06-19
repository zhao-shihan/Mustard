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
# System-only packages (REQUIRED, no source fallback)
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
CPMFindPackage(
    NAME mplr
    GITHUB_REPOSITORY zhao-shihan/mplr
    VERSION 0.26.605
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
set(MUSTARD_FMT_MINIMUM_REQUIRED 12.2.0)
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
    VERSION 1.2.2
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
    VERSION 0.26.619
    OPTIONS "MUC_INSTALL ON")

# =============================================================================
# Private dependencies (somewhat sorted by dependency order)
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
        VERSION 3.3.2
        OPTIONS "MI_OVERRIDE OFF"
                "MI_OSX_INTERPOSE OFF"
                "MI_OSX_ZONE OFF"
                "MI_BUILD_SHARED OFF"
                "MI_BUILD_OBJECT OFF"
                "MI_BUILD_TESTS ${BUILD_TESTING}")
endif()
