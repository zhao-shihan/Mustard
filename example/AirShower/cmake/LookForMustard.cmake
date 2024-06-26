# This script is used to automatically find Mustard, or build-in Mustard if not found.
# Mustard is found by find_package, or is built-in by FetchContent.
# If build-in, Mustard sources are extracted under thirdparty/,
#   or you can manually extract Mustard sources to thirdparty/.
#   Sources will always be reused if found.

message(STATUS "Looking for Mustard")

find_package(Mustard)
if(NOT Mustard_FOUND)
    set(BUILTIN_MUSTARD ON)
    message(NOTICE "***Notice: Mustard not found. Trying built-in Mustard.")
endif()

if(BUILTIN_MUSTARD)
    message(STATUS "Built-in Mustard will be used")
    # ExternalProject_Add sets the timestamps of extracted contents to the time of extraction
    # See https://cmake.org/cmake/help/latest/policy/CMP0135.html
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
        cmake_policy(SET CMP0135 NEW)
    endif()
    # set download dest and URL
    set(BUILTIN_MUSTARD_SRC_DIR "${PROJECT_SOURCE_DIR}/thirdparty/Mustard-main")
    set(BUILTIN_MUSTARD_URL "https://github.com/zhao-shihan/Mustard/archive/refs/heads/main.zip")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${BUILTIN_MUSTARD_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(Mustard SOURCE_DIR "${BUILTIN_MUSTARD_SRC_DIR}")
        message(STATUS "Reusing Mustard source ${BUILTIN_MUSTARD_SRC_DIR}")
    else()
        FetchContent_Declare(Mustard SOURCE_DIR "${BUILTIN_MUSTARD_SRC_DIR}"
                                     URL "${BUILTIN_MUSTARD_URL}")
        message(STATUS "Mustard will be downloaded from ${BUILTIN_MUSTARD_URL} to ${BUILTIN_MUSTARD_SRC_DIR}")
    endif()
    # configure it
    message(STATUS "Downloading (if required) and configuring Mustard")
    FetchContent_MakeAvailable(Mustard)
    message(STATUS "Downloading (if required) and configuring Mustard - done")
    # check download
    if(NOT EXISTS "${BUILTIN_MUSTARD_SRC_DIR}/CMakeLists.txt")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mustard-build")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mustard-subbuild")
        message(FATAL_ERROR "It seems that the download of Mustard has failed. You can try running cmake again, or manually download Mustard from ${BUILTIN_MUSTARD_URL} and extract it to ${CMAKE_PROJECT_SOURCE_DIR}/thirdparty/ (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    endif()
endif()

if(NOT BUILTIN_MUSTARD)
    message(STATUS "Looking for Mustard - found (version: ${Mustard_VERSION})")
else()
    message(STATUS "Looking for Mustard - built-in")
endif()
