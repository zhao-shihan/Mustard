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

message(STATUS "Looking for indicators")

set(MUSTARD_INDICATORS_MINIMUM_REQUIRED 2.3)

if(NOT MUSTARD_BUILTIN_INDICATORS)
    find_package(indicators ${MUSTARD_INDICATORS_MINIMUM_REQUIRED})
    if(NOT indicators_FOUND)
        set(MUSTARD_BUILTIN_INDICATORS ON)
        message(NOTICE "***Notice: indicators not found (minimum required is ${MUSTARD_INDICATORS_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_INDICATORS")
    endif()
endif()

if(MUSTARD_BUILTIN_INDICATORS)
    message(STATUS "Mustard will use built-in indicators")
    # check built-in version
    if(MUSTARD_BUILTIN_INDICATORS_VERSION VERSION_LESS MUSTARD_INDICATORS_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_INDICATORS_VERSION is ${MUSTARD_BUILTIN_INDICATORS_VERSION}, which is less than the requirement (${MUSTARD_INDICATORS_MINIMUM_REQUIRED}). Changing to ${MUSTARD_INDICATORS_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_INDICATORS_VERSION ${MUSTARD_INDICATORS_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_INDICATORS_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/indicators-master")
    set(MUSTARD_BUILTIN_INDICATORS_URL "https://github.com/p-ranav/indicators/archive/refs/heads/master.zip")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_INDICATORS_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(indicators SOURCE_DIR "${MUSTARD_BUILTIN_INDICATORS_SRC_DIR}")
        message(STATUS "Reusing indicators source ${MUSTARD_BUILTIN_INDICATORS_SRC_DIR}")
    else()
        FetchContent_Declare(indicators SOURCE_DIR "${MUSTARD_BUILTIN_INDICATORS_SRC_DIR}"
                                        URL "${MUSTARD_BUILTIN_INDICATORS_URL}")
        message(STATUS "indicators will be downloaded from ${MUSTARD_BUILTIN_INDICATORS_URL} to ${MUSTARD_BUILTIN_INDICATORS_SRC_DIR}")
    endif()
    # set options
    set(INDICATORS_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    set(INDICATORS_PACKAGE ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring indicators (version: ${MUSTARD_BUILTIN_INDICATORS_VERSION})")
    FetchContent_MakeAvailable(indicators)
    message(STATUS "Downloading (if required) and configuring indicators (version: ${MUSTARD_BUILTIN_INDICATORS_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_INDICATORS_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/indicators-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/indicators-subbuild")
    #     message(FATAL_ERROR "It seems that the download of indicators has failed. You can try running cmake again, or manually download indicators from ${MUSTARD_BUILTIN_INDICATORS_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_INDICATORS)
    message(STATUS "Looking for indicators - found (version: ${indicators_VERSION})")
else()
    message(STATUS "Looking for indicators - built-in (version: ${MUSTARD_BUILTIN_INDICATORS_VERSION})")
endif()
