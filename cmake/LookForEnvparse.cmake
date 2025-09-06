# Copyright (C) 2020-2025  The Mustard development team
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

message(STATUS "Looking for envparse")

set(MUSTARD_ENVPARSE_MINIMUM_REQUIRED 1.0.0)

if(NOT MUSTARD_BUILTIN_ENVPARSE)
    find_package(envparse ${MUSTARD_ENVPARSE_MINIMUM_REQUIRED})
    if(NOT envparse_FOUND)
        set(MUSTARD_BUILTIN_ENVPARSE ON)
        message(NOTICE "***Notice: envparse not found (minimum required is ${MUSTARD_ENVPARSE_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_ENVPARSE")
    endif()
endif()

if(MUSTARD_BUILTIN_ENVPARSE)
    message(STATUS "Mustard will use built-in envparse")
    # check built-in version
    if(MUSTARD_BUILTIN_ENVPARSE_VERSION VERSION_LESS MUSTARD_ENVPARSE_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_ENVPARSE_VERSION is ${MUSTARD_BUILTIN_ENVPARSE_VERSION}, which is less than the requirement (${MUSTARD_ENVPARSE_MINIMUM_REQUIRED}). Changing to ${MUSTARD_ENVPARSE_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_ENVPARSE_VERSION ${MUSTARD_ENVPARSE_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_ENVPARSE_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/envparse-v${MUSTARD_BUILTIN_ENVPARSE_VERSION}")
    set(MUSTARD_BUILTIN_ENVPARSE_URL "https://github.com/zhao-shihan/envparse/archive/refs/tags/v${MUSTARD_BUILTIN_ENVPARSE_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_ENVPARSE_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(envparse SOURCE_DIR "${MUSTARD_BUILTIN_ENVPARSE_SRC_DIR}")
        message(STATUS "Reusing envparse source ${MUSTARD_BUILTIN_ENVPARSE_SRC_DIR}")
    else()
        FetchContent_Declare(envparse SOURCE_DIR "${MUSTARD_BUILTIN_ENVPARSE_SRC_DIR}"
                                      URL "${MUSTARD_BUILTIN_ENVPARSE_URL}")
        message(STATUS "envparse will be downloaded from ${MUSTARD_BUILTIN_ENVPARSE_URL} to ${MUSTARD_BUILTIN_ENVPARSE_SRC_DIR}")
    endif()
    # set options
    set(ENVPARSE_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring envparse (version: ${MUSTARD_BUILTIN_ENVPARSE_VERSION})")
    FetchContent_MakeAvailable(envparse)
    message(STATUS "Downloading (if required) and configuring envparse (version: ${MUSTARD_BUILTIN_ENVPARSE_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_ENVPARSE_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/envparse-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/envparse-subbuild")
    #     message(FATAL_ERROR "It seems that the download of envparse has failed. You can try running cmake again, or manually download envparse from ${MUSTARD_BUILTIN_ENVPARSE_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_ENVPARSE)
    message(STATUS "Looking for envparse - found (version: ${envparse_VERSION})")
else()
    message(STATUS "Looking for envparse - built-in (version: ${MUSTARD_BUILTIN_ENVPARSE_VERSION})")
endif()
