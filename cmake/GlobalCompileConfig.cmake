# This file is included before "find_package"s and "LookFor"s.

# =============================================================================
# Mustard build type
# =============================================================================

if(DEFINED CMAKE_CONFIGURATION_TYPES)
    # multi-config generator
    message(STATUS "Using multi-config generator \"${CMAKE_GENERATOR}\"")
    message(STATUS "It provides the following build types: ${CMAKE_CONFIGURATION_TYPES}")
    if(DEFINED CMAKE_BUILD_TYPE)
        message(NOTICE "***Notice: CMAKE_BUILD_TYPE is defined while using a multi-config generator \"${CMAKE_GENERATOR}\".")
        message(NOTICE "           CMAKE_BUILD_TYPE is ignored by the multi-config generator and the build type should be specified at build time")
    endif()
else()
    # single-config generator
    message(STATUS "Using single-config generator \"${CMAKE_GENERATOR}\"")
    if(CMAKE_BUILD_TYPE STREQUAL "")
        # Default to "Release"
        set(CMAKE_BUILD_TYPE "Release")
    endif()
    string(TOLOWER ${CMAKE_BUILD_TYPE} LOWERCASED_CMAKE_BUILD_TYPE)
    message(STATUS "Mustard build type: ${CMAKE_BUILD_TYPE}")
endif()

# =============================================================================
# Select MSVC runtime library
# =============================================================================

# Respect to MUSTARD_USE_SHARED_MSVC_RT
if(MUSTARD_USE_SHARED_MSVC_RT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL)
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)
endif()
