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

set(MUSTARD_VERSION_MAJOR 0)
set(MUSTARD_VERSION_MINOR 26)
set(MUSTARD_VERSION_PATCH 315)
set(MUSTARD_VERSION ${MUSTARD_VERSION_MAJOR}.${MUSTARD_VERSION_MINOR}.${MUSTARD_VERSION_PATCH})

project(Mustard VERSION ${MUSTARD_VERSION}
                LANGUAGES C CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

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

# Options (ON/OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTING "Enable tests" ${PROJECT_IS_TOP_LEVEL})
option(MUSTARD_BUILTIN_ARGPARSE "Use built-in argparse (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_BACKWARD "Use built-in backward-cpp (recommended, network or pre-downloaded source is required)." ON)
option(MUSTARD_BUILTIN_EFM "Use built-in EFM (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_ENVPARSE "Use built-in envparse (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_FMT "Use built-in fmt (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_GTL "Use built-in gtl (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_INDICATORS "Use built-in indicators (recommended, network or pre-downloaded source is required)." ON)
option(MUSTARD_BUILTIN_MIMALLOC "Use built-in mimalloc if MUSTARD_USE_MIMALLOC is enabled (recommended, network or pre-downloaded source is required)." ON)
option(MUSTARD_BUILTIN_MPLR "Use built-in MPLR (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_MSGSL "Use built-in Microsoft.GSL (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_MUC "Use built-in muc (network or pre-downloaded source is required)." OFF)
option(MUSTARD_BUILTIN_YAML_CPP "Use built-in yaml-cpp (network or pre-downloaded source is required)." OFF)
option(MUSTARD_ENABLE_IPO "Build Mustard with interprocedural optimization" ${BUILD_SHARED_LIBS})
option(MUSTARD_ENABLE_MSVC_STD_CONFORMITY "Build Mustard with standard-conformity (MSVC only: /permissive- /Zc:__cplusplus /Zc:inline)." ON)
option(MUSTARD_FULL_UNITY_BUILD "Building Mustard use unity build technique COMPLETELY." OFF)
option(MUSTARD_SHOW_EVEN_MORE_COMPILER_WARNINGS "Add compiler flag to show even more compiler warnings (GCC: -Weffc++, Clang: -Weverything, MSVC: /Wall)." OFF)
option(MUSTARD_SIGNAL_HANDLER "Build with signal handling. Signal handlers are registered at the construction of environment." ON)
option(MUSTARD_USE_G4VIS "Build Geant4 applications of Mustard with available visualization. Note that whether Geant4 supports visualization is determined by options with which Geant4 is built. Mustard does not, and cannot, affect whether G4 enables visualization." ON)
option(MUSTARD_USE_MIMALLOC "Using mimalloc as the underlying implementation of Mustard memory allocation library." ON)
option(MUSTARD_USE_SHARED_MSVC_RT "Select the MSVC runtime library for use (MSVC only). ON for shared (.dll), OFF for static (.lib). If you run into link error related to MSVC runtime, you can try flip this." ON)
option(MUSTARD_USE_STATIC_G4 "Attempt to link Geant4 static libraries if available, by finding the \"static\" component of G4. If \"static\" is not found, dynamic libraries will be linked." OFF)
cmake_dependent_option(MUSTARD_SHOW_MORE_COMPILER_WARNINGS "Show compiler warnings raise from external dependencies." OFF "NOT MUSTARD_SHOW_EVEN_MORE_COMPILER_WARNINGS" ON)
cmake_dependent_option(MUSTARD_UNITY_BUILD "Build Mustard with unity build technique." OFF "NOT MUSTARD_FULL_UNITY_BUILD" ON)
set(MUSTARD_BUILTIN_ARGPARSE_VERSION 3.2 CACHE STRING "Set built-in argparse version (if required).")
set(MUSTARD_BUILTIN_BACKWARD_VERSION 1.6.1 CACHE STRING "Set built-in backward-cpp version (if required).")
set(MUSTARD_BUILTIN_EFM_VERSION 0.1.0 CACHE STRING "Set built-in EFM version (if required).")
set(MUSTARD_BUILTIN_ENVPARSE_VERSION 1.0.0 CACHE STRING "Set built-in envparse version (if required).")
set(MUSTARD_BUILTIN_FMT_VERSION 12.1.0 CACHE STRING "Set built-in fmt version (if required).")
set(MUSTARD_BUILTIN_GTL_VERSION 1.2.0 CACHE STRING "Set built-in gtl version (if required).")
set(MUSTARD_BUILTIN_INDICATORS_VERSION 2.3.1 CACHE STRING "Set built-in indicators version (if required).")
set(MUSTARD_BUILTIN_MIMALLOC_VERSION 3.2.8 CACHE STRING "Set built-in mimalloc version (if required).")
set(MUSTARD_BUILTIN_MPLR_VERSION 0.25.1003 CACHE STRING "Set built-in MPLR version (if required).")
set(MUSTARD_BUILTIN_MSGSL_VERSION 4.2.1 CACHE STRING "Set built-in Microsoft.GSL version (if required).")
set(MUSTARD_BUILTIN_MUC_VERSION 0.26.302 CACHE STRING "Set built-in muc version (if required).")
set(MUSTARD_BUILTIN_YAML_CPP_VERSION 0.9.0 CACHE STRING "Set built-in yaml-cpp version (if required).")
set(MUSTARD_DEBUG_POSTFIX d CACHE STRING "Debug library postfix.")

# =============================================================================
# Check compiler
# =============================================================================

set(MUSTARD_MINIMUM_GCC_VERSION 13)
set(MUSTARD_MINIMUM_CLANG_VERSION 19)
set(MUSTARD_MINIMUM_INTELLLVM_VERSION 2025)
set(MUSTARD_MINIMUM_MSVC_VERSION 19.50)

set(MUSTARD_COMPILER_OKAY TRUE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MUSTARD_MINIMUM_GCC_VERSION})
        set(MUSTARD_COMPILER_OKAY FALSE)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MUSTARD_MINIMUM_CLANG_VERSION})
        set(MUSTARD_COMPILER_OKAY FALSE)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MUSTARD_MINIMUM_INTELLLVM_VERSION})
        set(MUSTARD_COMPILER_OKAY FALSE)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${MUSTARD_MINIMUM_MSVC_VERSION})
        set(MUSTARD_COMPILER_OKAY FALSE)
    endif()
endif()

if(NOT MUSTARD_COMPILER_OKAY)
    message(FATAL_ERROR "Mustard cannot be built with ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}.\n"
                        "  Recommended compilers:\n"
                        "    GNU Compiler (at least GCC ${MUSTARD_MINIMUM_GCC_VERSION})\n"
                        "    LLVM Clang (at least Clang ${MUSTARD_MINIMUM_CLANG_VERSION})\n"
                        "    Intel oneAPI DPC++/C++ Compiler (at least IntelLLVM ${MUSTARD_MINIMUM_INTELLLVM_VERSION})\n"
                        "    Microsoft Visual C++ (at least MSVC ${MUSTARD_MINIMUM_MSVC_VERSION})")
endif()

# =============================================================================
# Installation path
# =============================================================================

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE ${PROJECT_SOURCE_DIR}/install)
endif()
message(STATUS "Mustard install path: ${CMAKE_INSTALL_PREFIX} (add -DCMAKE_INSTALL_PREFIX=<install_path> into CMake command if you wish to change it)")

# =============================================================================
# Global compile options
# =============================================================================

include(${MUSTARD_PROJECT_CMAKE_DIR}/GlobalCompileConfig.cmake)

# =============================================================================
# Dependencies
# =============================================================================

# Required dependencies (must be found, otherwise the configuration will fail and MUSTARD_BUILTIN_<dependency> will be turned on if available)

# Public dependencies
# fmt (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForFmt.cmake)
# Geant4, depends on many external libraries
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForGeant4.cmake)
# ROOT, depends on many more external libraries
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForROOT.cmake)
# yaml-cpp (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForYamlCpp.cmake)

# Interface dependencies
# argparse (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForArgparse.cmake)
# EFM (built-in if not found), depends on ROOT
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForEFM.cmake)
# Eigen
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForEigen.cmake)
# envparse (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForEnvparse.cmake)
# gtl (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForGTL.cmake)
# Microsoft.GSL (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForMicrosoftGSL.cmake)
# MPLR (built-in if not found), depends on MPI
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForMPLR.cmake)
# muc (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForMUC.cmake)

# Private dependencies
# backward-cpp (built-in if not found), depends on some stacktrace library
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForBackwardCpp.cmake)
# indicators (built-in if not found)
include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForIndicators.cmake)

# Conditional dependencies (only required if certain options are enabled)

# Private dependencies
if(MUSTARD_USE_MIMALLOC)
    # mimalloc (built-in if not found)
    include(${MUSTARD_PROJECT_CMAKE_DIR}/LookForMimalloc.cmake)
endif()

# =============================================================================
# Let's dive in...
# =============================================================================

# Config for Mustard
include(${MUSTARD_PROJECT_CMAKE_DIR}/MustardCompileConfig.cmake)

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

# =============================================================================
# Install
# =============================================================================

set_target_properties(Mustard PROPERTIES VERSION ${MUSTARD_VERSION}
                                            SOVERSION ${MUSTARD_VERSION}
                                            DEBUG_POSTFIX ${MUSTARD_DEBUG_POSTFIX})

install(DIRECTORY src/Mustard DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN *.h++ PATTERN *.inl)
install(TARGETS Mustard EXPORT MustardTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT MustardTargets
        NAMESPACE Mustard::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)

configure_package_config_file(${MUSTARD_PROJECT_CMAKE_DIR}/MustardConfig.cmake.in
                                ${CMAKE_CURRENT_BINARY_DIR}/MustardConfig.cmake
                                INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/MustardConfigVersion.cmake
                                    COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MustardConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/MustardConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
