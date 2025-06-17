message(STATUS "Looking for mpl")

set(MUSTARD_MPL_MINIMUM_REQUIRED 0.6.0)

if(NOT MUSTARD_BUILTIN_MPL)
    find_package(mpl ${MUSTARD_MPL_MINIMUM_REQUIRED})
    if(NOT mpl_FOUND)
        set(MUSTARD_BUILTIN_MPL ON)
        message(NOTICE "***Notice: mpl not found (minimum required is ${MUSTARD_MPL_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_MPL")
    endif()
endif()

if(MUSTARD_BUILTIN_MPL)
    message(STATUS "Mustard will use built-in mpl")
    # check built-in version
    if(MUSTARD_BUILTIN_MPL_VERSION VERSION_LESS MUSTARD_MPL_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_MPL_VERSION is ${MUSTARD_BUILTIN_MPL_VERSION}, which is less than the requirement (${MUSTARD_MPL_MINIMUM_REQUIRED}). Changing to ${MUSTARD_MPL_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_MPL_VERSION ${MUSTARD_MPL_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_MPL_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/mpl-${MUSTARD_BUILTIN_MPL_VERSION}")
    set(MUSTARD_BUILTIN_MPL_URL "https://github.com/zhao-shihan/mpl/archive/refs/tags/v${MUSTARD_BUILTIN_MPL_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_MPL_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(mpl SOURCE_DIR "${MUSTARD_BUILTIN_MPL_SRC_DIR}")
        message(STATUS "Reusing mpl source ${MUSTARD_BUILTIN_MPL_SRC_DIR}")
    else()
        FetchContent_Declare(mpl SOURCE_DIR "${MUSTARD_BUILTIN_MPL_SRC_DIR}"
                                    URL "${MUSTARD_BUILTIN_MPL_URL}")
        message(STATUS "mpl will be downloaded from ${MUSTARD_BUILTIN_MPL_URL} to ${MUSTARD_BUILTIN_MPL_SRC_DIR}")
    endif()
    # set options
    set(BUILD_TESTING OFF CACHE INTERNAL "")
    set(MPL_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring mpl (version: ${MUSTARD_BUILTIN_MPL_VERSION})")
    FetchContent_MakeAvailable(mpl)
    message(STATUS "Downloading (if required) and configuring mpl (version: ${MUSTARD_BUILTIN_MPL_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_MPL_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mpl-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/mpl-subbuild")
    #     message(FATAL_ERROR "It seems that the download of mpl has failed. You can try running cmake again, or manually download mpl from ${MUSTARD_BUILTIN_MPL_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_MPL)
    message(STATUS "Looking for mpl - found (version: ${mpl_VERSION})")
else()
    message(STATUS "Looking for mpl - built-in (version: ${MUSTARD_BUILTIN_MPL_VERSION})")
endif()
