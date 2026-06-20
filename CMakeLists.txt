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

cmake_minimum_required(VERSION 3.24)

# Enable MSVC_RUNTIME_LIBRARY target property
# See https://cmake.org/cmake/help/latest/policy/CMP0091.html
cmake_policy(SET CMP0091 NEW)

set(MUSTARD_VERSION_MAJOR 0)
set(MUSTARD_VERSION_MINOR 26)
set(MUSTARD_VERSION_PATCH 315)
set(MUSTARD_VERSION ${MUSTARD_VERSION_MAJOR}.${MUSTARD_VERSION_MINOR}.${MUSTARD_VERSION_PATCH})

project(Mustard
    VERSION ${MUSTARD_VERSION}
    LANGUAGES CXX)

set(MUSTARD_CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${MUSTARD_CMAKE_MODULE_PATH})

# =============================================================================
# Options
# =============================================================================

include(CMakeDependentOption)

# Options (ON/OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTING "Enable tests" OFF)
option(MUSTARD_BUILD_TESTING "Enable tests for Mustard (even if BUILD_TESTING is OFF)" ${PROJECT_IS_TOP_LEVEL})
option(MUSTARD_ENABLE_IPO "Build Mustard with interprocedural optimization" ${BUILD_SHARED_LIBS})
option(MUSTARD_ENABLE_MSVC_STD_CONFORMITY "Build Mustard with standard-conformity (MSVC only: /permissive- /Zc:__cplusplus /Zc:inline)." ON)
option(MUSTARD_FULL_UNITY_BUILD "Building Mustard use unity build technique COMPLETELY." OFF)
option(MUSTARD_SHOW_EVEN_MORE_COMPILER_WARNINGS "Add compiler flag to show even more compiler warnings (GCC: -Weffc++, Clang: -Weverything, MSVC: /Wall)." OFF)
option(MUSTARD_SIGNAL_HANDLER "Build with signal handling. Signal handlers are registered at the construction of environment." ON)
option(MUSTARD_USE_FFTW "Use FFTW for Fourier transform." ON)
option(MUSTARD_USE_G4VIS "Build Geant4 applications of Mustard with available visualization. Note that whether Geant4 supports visualization is determined by options with which Geant4 is built. Mustard does not, and cannot, affect whether G4 enables visualization." ON)
option(MUSTARD_USE_MIMALLOC "Using mimalloc as the underlying implementation of Mustard memory allocation library." ON)
option(MUSTARD_USE_SHARED_MSVC_RT "Select the MSVC runtime library for use (MSVC only). ON for shared (.dll), OFF for static (.lib). If you run into link error related to MSVC runtime, you can try flip this." ON)
option(MUSTARD_USE_STATIC_G4 "Attempt to link Geant4 static libraries if available, by finding the \"static\" component of G4. If \"static\" is not found, dynamic libraries will be linked." OFF)
option(MUSTARD_WERROR "Treat Mustard compilation warnings as errors." OFF)
cmake_dependent_option(MUSTARD_SHOW_MORE_COMPILER_WARNINGS "Show compiler warnings raise from external packages." OFF "NOT MUSTARD_SHOW_EVEN_MORE_COMPILER_WARNINGS" ON)
cmake_dependent_option(MUSTARD_UNITY_BUILD "Build Mustard with unity build technique." OFF "NOT MUSTARD_FULL_UNITY_BUILD" ON)
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
    message(FATAL_ERROR
        "Mustard cannot be built with ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}.\n"
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
# Complie configs and dependencies
# =============================================================================

# Global compile configs, before adding dependencies
include(GlobalCompileConfig)
# Find and configure dependencies.
include(LookForPackages)
# Compile configs for Mustard itself, after dependencies are added.
include(MustardCompileConfig)

# =============================================================================
# Let's dive in...
# =============================================================================

add_subdirectory(src)
if(MUSTARD_BUILD_TESTING)
    add_subdirectory(test)
endif()
add_subdirectory(tooling)

# =============================================================================
# Install
# =============================================================================

include(GNUInstallDirs)
set_target_properties(Mustard PROPERTIES
    VERSION ${MUSTARD_VERSION}
    SOVERSION ${MUSTARD_VERSION}
    DEBUG_POSTFIX ${MUSTARD_DEBUG_POSTFIX})

install(DIRECTORY src/Mustard
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN *.h++ PATTERN *.inl)
install(TARGETS Mustard EXPORT MustardTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT MustardTargets
    NAMESPACE Mustard::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${MUSTARD_CMAKE_MODULE_PATH}/MustardConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MustardConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MustardConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MustardConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MustardConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
