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

message(STATUS "Looking for mimalloc")

set(MUSTARD_MIMALLOC_MINIMUM_REQUIRED 3.2.8)

if(NOT MUSTARD_BUILTIN_MIMALLOC)
    find_package(mimalloc ${MUSTARD_MIMALLOC_MINIMUM_REQUIRED})
    if(NOT mimalloc_FOUND)
        set(MUSTARD_BUILTIN_MIMALLOC ON)
        message(NOTICE "***Notice: mimalloc not found (minimum required is ${MUSTARD_MIMALLOC_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_MIMALLOC")
    endif()
endif()

if(MUSTARD_BUILTIN_MIMALLOC)
    message(STATUS "Mustard will use built-in mimalloc")
    # check built-in version
    if(MUSTARD_BUILTIN_MIMALLOC_VERSION VERSION_LESS MUSTARD_MIMALLOC_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_MIMALLOC_VERSION is ${MUSTARD_BUILTIN_MIMALLOC_VERSION}, which is less than the requirement (${MUSTARD_MIMALLOC_MINIMUM_REQUIRED}). Changing to ${MUSTARD_MIMALLOC_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_MIMALLOC_VERSION ${MUSTARD_MIMALLOC_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_MIMALLOC_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/mimalloc-${MUSTARD_BUILTIN_MIMALLOC_VERSION}")
    set(MUSTARD_BUILTIN_MIMALLOC_URL "https://github.com/microsoft/mimalloc/archive/refs/tags/v${MUSTARD_BUILTIN_MIMALLOC_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_MIMALLOC_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(mimalloc SOURCE_DIR "${MUSTARD_BUILTIN_MIMALLOC_SRC_DIR}")
        message(STATUS "Reusing mimalloc source ${MUSTARD_BUILTIN_MIMALLOC_SRC_DIR}")
    else()
        FetchContent_Declare(mimalloc SOURCE_DIR "${MUSTARD_BUILTIN_MIMALLOC_SRC_DIR}"
                                      URL "${MUSTARD_BUILTIN_MIMALLOC_URL}")
        message(STATUS "mimalloc will be downloaded from ${MUSTARD_BUILTIN_MIMALLOC_URL} to ${MUSTARD_BUILTIN_MIMALLOC_SRC_DIR}")
    endif()
    # set options
    set(MI_OVERRIDE OFF CACHE INTERNAL "")
    set(MI_OSX_INTERPOSE OFF CACHE INTERNAL "")
    set(MI_OSX_ZONE OFF CACHE INTERNAL "")
    set(MI_BUILD_SHARED OFF CACHE INTERNAL "")
    set(MI_BUILD_OBJECT OFF CACHE INTERNAL "")
    set(MI_BUILD_TESTS ${BUILD_TESTING} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring mimalloc (version: ${MUSTARD_BUILTIN_MIMALLOC_VERSION})")
    FetchContent_MakeAvailable(mimalloc)
    message(STATUS "Downloading (if required) and configuring mimalloc (version: ${MUSTARD_BUILTIN_MIMALLOC_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_MIMALLOC_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mimalloc-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mimalloc-subbuild")
    #     message(FATAL_ERROR "It seems that the download of mimalloc has failed. You can try running cmake again, or manually download mimalloc from ${MUSTARD_BUILTIN_MIMALLOC_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_MIMALLOC)
    message(STATUS "Looking for mimalloc - found (version: ${mimalloc_VERSION})")
else()
    message(STATUS "Looking for mimalloc - built-in (version: ${MUSTARD_BUILTIN_MIMALLOC_VERSION})")
endif()
