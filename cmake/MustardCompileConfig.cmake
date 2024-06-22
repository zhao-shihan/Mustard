# This file is included after "find_package"s and "LookFor"s.

# =============================================================================
# Mustard at C++20
# =============================================================================

set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard.")
if(CMAKE_CXX_STANDARD LESS 20)
    message(FATAL_ERROR "Mustard should be built, at least, with C++20")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "Mustard will be compiled with C++${CMAKE_CXX_STANDARD}")

# =============================================================================
# By default, no C++ extensions available for Mustard
# =============================================================================

set(CMAKE_CXX_EXTENSIONS OFF CACHE INTERNAL "Boolean specifying whether compiler specific extensions are requested.")

# =============================================================================
# Unity build for Mustard
# =============================================================================

if(MUSTARD_ENABLE_UNITY_BUILD)
    set(CMAKE_UNITY_BUILD ON)
    if(MUSTARD_FULL_UNITY_BUILD)
        set(CMAKE_UNITY_BUILD_BATCH_SIZE 0)
    else()
        set(CMAKE_UNITY_BUILD_BATCH_SIZE 8)
    endif()
    if(CMAKE_UNITY_BUILD_BATCH_SIZE GREATER 0)
        message(STATUS "Unity build enabled for Mustard (batch size: ${CMAKE_UNITY_BUILD_BATCH_SIZE})")
    else()
        message(STATUS "Unity build enabled for Mustard (batch size: unlimited)")
    endif()
endif()

# =============================================================================
# LTO/IPO for Mustard
# =============================================================================

if(MUSTARD_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT MUSTARD_ENABLE_IPO_SUPPORTED
                        OUTPUT MUSTARD_IPO_SUPPORTED_ERROR)
    if(MUSTARD_ENABLE_IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
        message(STATUS "LTO/IPO enabled for Mustard")
    else()
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE OFF)
        message(NOTICE "***Notice: LTO/IPO not supported. Turning off CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE")
    endif()
endif()

# =============================================================================
# Compile options and definitions for Mustard
# =============================================================================

set(MUSTARD_PRIVATE_COMPILE_OPTIONS "")
set(MUSTARD_PUBLIC_COMPILE_DEFINITIONS "")

# =============================================================================
# Compile warnings for Mustard
# =============================================================================

# More warnings
if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS -Wall -Wextra -Wduplicated-cond -Wnon-virtual-dtor -pedantic -Wundef -Wunused-macros)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS -WCL4 -Wmove -Wnon-virtual-dtor -pedantic -Wundef -Wunused-macros -Wno-gnu-zero-variadic-macro-arguments)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /W4)
endif()

# Surpress some, if required
if(NOT MUSTARD_SHOW_MORE_COMPILER_WARNINGS)
    if(CMAKE_COMPILER_IS_GNUCXX)
        # nothing for now
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # nothing for now
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # TODO: below is ancient; need update.
        # # ROOT (conditional expression is constant)
        # list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /wd4127)
        # # Common ('argument': conversion from 'type1' to 'type2', possible loss of data)
        # list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /wd4244)
        # # Common ('var': conversion from 'size_t' to 'type', possible loss of data)
        # list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /wd4267)
        # # MSVC std::tuple ('derived class' : destructor was implicitly defined as deleted because a base class destructor is inaccessible or deleted)
        # list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /wd4624)
        # # Common (The file contains a character that cannot be represented in the current code page (number). Save the file in Unicode format to prevent data loss)
        # list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /wd4819)
        # # ROOT (using a function, class member, variable, or typedef that's marked deprecated)
        # list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /wd4996)
        # # Eigen (operator 'operator-name': deprecated between enumerations of different types)
        # list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /wd5054)
    endif()
# Even more warnings, if required
elseif(MUSTARD_SHOW_EVEN_MORE_COMPILER_WARNINGS)
    if(CMAKE_COMPILER_IS_GNUCXX)
        list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS -Weffc++)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS -Weverything)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /Wall)
    endif()
endif()

# =============================================================================
# Other CMake-options-controlled compile options for Mustard
# =============================================================================

if(MUSTARD_SIGNAL_HANDLER)
    list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS MUSTARD_SIGNAL_HANDLER=1)
else()
    list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS MUSTARD_SIGNAL_HANDLER=0)
endif()

if(MUSTARD_USE_G4VIS)
    list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS MUSTARD_USE_G4VIS=1)
else()
    list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS MUSTARD_USE_G4VIS=0)
endif()

if(MUSTARD_ENABLE_MSVC_STD_CONFORMITY AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Enable standard-conformity
    list(APPEND MUSTARD_PRIVATE_COMPILE_OPTIONS /permissive- /Zc:__cplusplus /Zc:inline)
    message(STATUS "MSVC standard-conformity enabled (/permissive- /Zc:__cplusplus /Zc:inline)")
    # Be permissive to standard cfunctions
    list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS=1)
endif()

# =============================================================================
# MPI-induced compile options for Mustard
# =============================================================================

# Inform OpenMPI not to bring mpicxx in, it's necessary for most cases.
list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS OMPI_SKIP_MPICXX=1)

# Inform MPICH and derivatives not to bring mpicxx in, seems unnecessary but more consistent.
list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS MPICH_SKIP_MPICXX=1)

# =============================================================================
# Eigen-induced compile options for Mustard
# =============================================================================

# Inform Eigen not to enable multithreading, though we are not using OpenMP. It is safer to do so.
list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS EIGEN_DONT_PARALLELIZE=1)

if(CMAKE_CXX_PLATFORM_ID STREQUAL "MinGW")
    # MinGW and GCC 12.2 have issues with explitic vectorization
    list(APPEND MUSTARD_PUBLIC_COMPILE_DEFINITIONS EIGEN_DONT_VECTORIZE=1)
    message(NOTICE "***Notice: Building on Windows with MinGW, disabling vectorization of Eigen")
endif()
