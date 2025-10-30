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

message(STATUS "Looking for fmt")

set(MUSTARD_FMT_MINIMUM_REQUIRED 12.1.0)

if(NOT MUSTARD_BUILTIN_FMT)
    find_package(fmt ${MUSTARD_FMT_MINIMUM_REQUIRED})
    if(NOT fmt_FOUND)
        set(MUSTARD_BUILTIN_FMT ON)
        message(NOTICE "***Notice: fmt not found (minimum required is ${MUSTARD_FMT_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_FMT")
    endif()
endif()

if(MUSTARD_BUILTIN_FMT)
    message(STATUS "Mustard will use built-in fmt")
    # check built-in version
    if(MUSTARD_BUILTIN_FMT_VERSION VERSION_LESS MUSTARD_FMT_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_FMT_VERSION is ${MUSTARD_BUILTIN_FMT_VERSION}, which is less than the requirement (${MUSTARD_FMT_MINIMUM_REQUIRED}). Changing to ${MUSTARD_FMT_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_FMT_VERSION ${MUSTARD_FMT_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_FMT_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/fmt-${MUSTARD_BUILTIN_FMT_VERSION}")
    set(MUSTARD_BUILTIN_FMT_URL "https://github.com/fmtlib/fmt/archive/refs/tags/${MUSTARD_BUILTIN_FMT_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_FMT_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(fmt SOURCE_DIR "${MUSTARD_BUILTIN_FMT_SRC_DIR}")
        message(STATUS "Reusing fmt source ${MUSTARD_BUILTIN_FMT_SRC_DIR}")
    else()
        FetchContent_Declare(fmt SOURCE_DIR "${MUSTARD_BUILTIN_FMT_SRC_DIR}"
                                 URL "${MUSTARD_BUILTIN_FMT_URL}")
        message(STATUS "fmt will be downloaded from ${MUSTARD_BUILTIN_FMT_URL} to ${MUSTARD_BUILTIN_FMT_SRC_DIR}")
    endif()
    # set options
    set(FMT_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring fmt (version: ${MUSTARD_BUILTIN_FMT_VERSION})")
    FetchContent_MakeAvailable(fmt)
    message(STATUS "Downloading (if required) and configuring fmt (version: ${MUSTARD_BUILTIN_FMT_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_FMT_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/fmt-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/fmt-subbuild")
    #     message(FATAL_ERROR "It seems that the download of fmt has failed. You can try running cmake again, or manually download fmt from ${MUSTARD_BUILTIN_FMT_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_FMT)
    message(STATUS "Looking for fmt - found (version: ${fmt_VERSION})")
else()
    message(STATUS "Looking for fmt - built-in (version: ${MUSTARD_BUILTIN_FMT_VERSION})")
endif()
