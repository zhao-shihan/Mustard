message(STATUS "Looking for mplr")

set(MUSTARD_MPLR_MINIMUM_REQUIRED 0.25.730)

if(NOT MUSTARD_BUILTIN_MPLR)
    find_package(mplr ${MUSTARD_MPLR_MINIMUM_REQUIRED})
    if(NOT mplr_FOUND)
        set(MUSTARD_BUILTIN_MPLR ON)
        message(NOTICE "***Notice: mplr not found (minimum required is ${MUSTARD_MPLR_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_MPLR")
    endif()
endif()

if(MUSTARD_BUILTIN_MPLR)
    message(STATUS "Mustard will use built-in mplr")
    # check built-in version
    if(MUSTARD_BUILTIN_MPLR_VERSION VERSION_LESS MUSTARD_MPLR_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_MPLR_VERSION is ${MUSTARD_BUILTIN_MPLR_VERSION}, which is less than the requirement (${MUSTARD_MPLR_MINIMUM_REQUIRED}). Changing to ${MUSTARD_MPLR_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_MPLR_VERSION ${MUSTARD_MPLR_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_MPLR_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/mplr-${MUSTARD_BUILTIN_MPLR_VERSION}")
    set(MUSTARD_BUILTIN_MPLR_URL "https://github.com/zhao-shihan/mplr/archive/refs/tags/v${MUSTARD_BUILTIN_MPLR_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_MPLR_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(mplr SOURCE_DIR "${MUSTARD_BUILTIN_MPLR_SRC_DIR}")
        message(STATUS "Reusing mplr source ${MUSTARD_BUILTIN_MPLR_SRC_DIR}")
    else()
        FetchContent_Declare(mplr SOURCE_DIR "${MUSTARD_BUILTIN_MPLR_SRC_DIR}"
                                  URL "${MUSTARD_BUILTIN_MPLR_URL}")
        message(STATUS "mplr will be downloaded from ${MUSTARD_BUILTIN_MPLR_URL} to ${MUSTARD_BUILTIN_MPLR_SRC_DIR}")
    endif()
    # set options
    set(MPLR_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring mplr (version: ${MUSTARD_BUILTIN_MPLR_VERSION})")
    FetchContent_MakeAvailable(mplr)
    message(STATUS "Downloading (if required) and configuring mplr (version: ${MUSTARD_BUILTIN_MPLR_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_MPLR_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mplr-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mplr-subbuild")
    #     message(FATAL_ERROR "It seems that the download of mplr has failed. You can try running cmake again, or manually download mplr from ${MUSTARD_BUILTIN_MPLR_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_MPLR)
    message(STATUS "Looking for mplr - found (version: ${mplr_VERSION})")
else()
    message(STATUS "Looking for mplr - built-in (version: ${MUSTARD_BUILTIN_MPLR_VERSION})")
endif()
