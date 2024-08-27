cmake_minimum_required(VERSION 3.21)

# Enable MSVC_RUNTIME_LIBRARY target property
# See https://cmake.org/cmake/help/latest/policy/CMP0091.html
cmake_policy(SET CMP0091 NEW)
# Set CMP0077 to new in order to configure options for build-in libraries
# See https://discourse.cmake.org/t/what-is-the-correct-way-to-set-options-of-a-project-before-fetch-content/268/6
# See also https://cmake.org/cmake/help/latest/policy/CMP0077.html
cmake_policy(SET CMP0077 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    # ExternalProject_Add sets the timestamps of extracted contents to the time of extraction
    # See https://cmake.org/cmake/help/latest/policy/CMP0135.html
    cmake_policy(SET CMP0135 NEW)
endif()

set(MUSTARD_VERSION 0.5.0)

project(Mustard VERSION ${MUSTARD_VERSION}
                LANGUAGES C CXX)

# =============================================================================
# Variables
# =============================================================================

# Dirs
set(MUSTARD_PROJECT_CMAKE_DIR ${PROJECT_SOURCE_DIR}/cmake)
set(MUSTARD_PROJECT_3RDPARTY_DIR ${PROJECT_SOURCE_DIR}/thirdparty)

# =============================================================================
# Options
# =============================================================================

include(CMakeDependentOption)

# Basic
# Options (ON/OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(MUSTARD_BUILD_TEST "Build Mustard with tests." ${PROJECT_IS_TOP_LEVEL})
option(MUSTARD_BUILTIN_ARGPARSE "Use built-in argparse (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_BACKWARD "Use built-in backward-cpp (network or pre-downloaded source is required)." ON)
option(MUSTARD_BUILTIN_EFM "Use built-in EFM (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_ENVPARSE "Use built-in envparse (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_FMT "Use built-in fmt (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_MSGSL "Use built-in Microsoft.GSL (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_MUC "Use built-in muc (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_TIMSORT "Use built-in TimSort (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_YAML_CPP "Use built-in yaml-cpp (network or pre-downloaded source is required)." OFF)
option(MUSTARD_SIGNAL_HANDLER "Build with signal handling. Signal handlers are registered at the construction of environment." ON)
option(MUSTARD_USE_G4VIS "Build Geant4 applications of Mustard with available visualization. Note that whether Geant4 supports visualization is determined by options with which Geant4 is built. Mustard does not, and cannot, affect whether G4 enables visualization." ON)
option(MUSTARD_USE_STATIC_G4 "Attempt to link Geant4 static libraries if available, by finding the \"static\" component of G4. If \"static\" is not found, dynamic libraries will be linked." OFF)
cmake_dependent_option(MUSTARD_INSTALL "Install Mustard." ${PROJECT_IS_TOP_LEVEL} "NOT BUILD_SHARED_LIBS" ON)

# Advance
# Options (ON/OFF)
option(MUSTARD_ENABLE_IPO "Bulid Mustard with interprocedural optimization" ${BUILD_SHARED_LIBS})
option(MUSTARD_ENABLE_MSVC_STD_CONFORMITY "Build Mustard with standard-conformity (MSVC only: /permissive- /Zc:__cplusplus /Zc:inline)." ON)
option(MUSTARD_FULL_UNITY_BUILD "Building Mustard use unity build technique COMPLETELY." ON)
option(MUSTARD_SHOW_EVEN_MORE_COMPILER_WARNINGS "Add compiler flag to show even more compiler warnings (GCC: -Weffc++, Clang: -Weverything, MSVC: /Wall)." OFF)
option(MUSTARD_USE_SHARED_MSVC_RT "Select the MSVC runtime library for use (MSVC only). ON for shared (.dll), OFF for static (.lib). If you run into link error related to MSVC runtime, you can try flip this." ON)
cmake_dependent_option(MUSTARD_ENABLE_UNITY_BUILD "Build Mustard with unity build technique." OFF "NOT MUSTARD_FULL_UNITY_BUILD" ON)
cmake_dependent_option(MUSTARD_SHOW_MORE_COMPILER_WARNINGS "Show compiler warnings raise from external dependencies." OFF "NOT MUSTARD_SHOW_EVEN_MORE_COMPILER_WARNINGS" ON)
set(MUSTARD_BUILTIN_ARGPARSE_VERSION 3.0 CACHE STRING "Set built-in argparse version (if required).")
set(MUSTARD_BUILTIN_BACKWARD_VERSION 1.6 CACHE STRING "Set built-in backward-cpp version (if required).")
set(MUSTARD_BUILTIN_EFM_VERSION 0.0.1 CACHE STRING "Set built-in EFM version (if required).")
set(MUSTARD_BUILTIN_ENVPARSE_VERSION 1.0.0 CACHE STRING "Set built-in envparse version (if required).")
set(MUSTARD_BUILTIN_FMT_VERSION 10.2.1 CACHE STRING "Set built-in fmt version (if required).")
set(MUSTARD_BUILTIN_MSGSL_VERSION 4.0.0 CACHE STRING "Set built-in Microsoft.GSL version (if required).")
set(MUSTARD_BUILTIN_MUC_VERSION 0.0.1 CACHE STRING "Set built-in muc version (if required).")
set(MUSTARD_BUILTIN_TIMSORT_VERSION 3.0.0 CACHE STRING "Set built-in TimSort version (if required).")
set(MUSTARD_BUILTIN_YAML_CPP_VERSION 0.8.0 CACHE STRING "Set built-in yaml-cpp version (if required).")
set(MUSTARD_DEBUG_POSTFIX d CACHE STRING "Debug library postfix.")

# =============================================================================
# Installation path
# =============================================================================

if(MUSTARD_INSTALL)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${PROJECT_SOURCE_DIR}/install)
    endif()
    message(STATUS "Mustard install path: ${CMAKE_INSTALL_PREFIX} (add -DCMAKE_INSTALL_PREFIX=<install_path> into CMake command if you wish to change it)")
else()
    message(STATUS "Mustard will not be installed.")
endif()

# =============================================================================
# Global compile options
# =============================================================================

include(${MUSTARD_PROJECT_CMAKE_DIR}/GlobalCompileConfig.cmake)

# =============================================================================
# Dependencies
# =============================================================================

# Required:

# MPI
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForMPI.cmake)
# Eigen
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForEigen.cmake)
# Geant4
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForGeant4.cmake)
# ROOT
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForROOT.cmake)

# Required, built-in if not found:

# argparse
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForArgparse.cmake)
# backward-cpp
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForBackward.cmake)
# EFM
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForEFM.cmake)
# envparse
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForEnvparse.cmake)
# fmt
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForFmt.cmake)
# Microsoft.GSL
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForMicrosoftGSL.cmake)
# muc
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForMUC.cmake)
# yaml-cpp
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForYamlCpp.cmake)

# =============================================================================
# Let's go!
# =============================================================================

# Config for Mustard
include(${MUSTARD_PROJECT_CMAKE_DIR}/MustardCompileConfig.cmake)

add_subdirectory(src)
if(MUSTARD_BUILD_TEST)
    add_subdirectory(test)
endif()
