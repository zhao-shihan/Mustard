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

message(STATUS "Looking for gtl")

set(MUSTARD_GTL_MINIMUM_REQUIRED 1.2.0)

if(NOT MUSTARD_BUILTIN_GTL)
    find_package(gtl ${MUSTARD_GTL_MINIMUM_REQUIRED})
    if(NOT gtl_FOUND)
        set(MUSTARD_BUILTIN_GTL ON)
        message(NOTICE "***Notice: gtl not found (minimum required is ${MUSTARD_GTL_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_GTL")
    endif()
endif()

if(MUSTARD_BUILTIN_GTL)
    message(STATUS "Mustard will use built-in gtl")
    # check built-in version
    if(MUSTARD_BUILTIN_GTL_VERSION VERSION_LESS MUSTARD_GTL_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_GTL_VERSION is ${MUSTARD_BUILTIN_GTL_VERSION}, which is less than the requirement (${MUSTARD_GTL_MINIMUM_REQUIRED}). Changing to ${MUSTARD_GTL_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_GTL_VERSION ${MUSTARD_GTL_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_GTL_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/gtl-${MUSTARD_BUILTIN_GTL_VERSION}")
    set(MUSTARD_BUILTIN_GTL_URL "https://github.com/greg7mdp/gtl/archive/refs/tags/v${MUSTARD_BUILTIN_GTL_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_GTL_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(gtl SOURCE_DIR "${MUSTARD_BUILTIN_GTL_SRC_DIR}")
        message(STATUS "Reusing gtl source ${MUSTARD_BUILTIN_GTL_SRC_DIR}")
    else()
        FetchContent_Declare(gtl SOURCE_DIR "${MUSTARD_BUILTIN_GTL_SRC_DIR}"
                                 URL "${MUSTARD_BUILTIN_GTL_URL}")
        message(STATUS "gtl will be downloaded from ${MUSTARD_BUILTIN_GTL_URL} to ${MUSTARD_BUILTIN_GTL_SRC_DIR}")
    endif()
    # set options
    set(GTL_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring gtl (version: ${MUSTARD_BUILTIN_GTL_VERSION})")
    FetchContent_MakeAvailable(gtl)
    message(STATUS "Downloading (if required) and configuring gtl (version: ${MUSTARD_BUILTIN_GTL_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_GTL_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/gtl-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/gtl-subbuild")
    #     message(FATAL_ERROR "It seems that the download of gtl has failed. You can try running cmake again, or manually download gtl from ${MUSTARD_BUILTIN_GTL_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_GTL)
    message(STATUS "Looking for gtl - found (version: ${gtl_VERSION})")
else()
    message(STATUS "Looking for gtl - built-in (version: ${MUSTARD_BUILTIN_GTL_VERSION})")
endif()
