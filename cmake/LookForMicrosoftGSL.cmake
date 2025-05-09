message(STATUS "Looking for Microsoft.GSL")

set(MUSTARD_MSGSL_MINIMUM_REQUIRED 4.2.0)

if(NOT MUSTARD_BUILTIN_MSGSL)
    find_package(Microsoft.GSL ${MUSTARD_MSGSL_MINIMUM_REQUIRED})
    if(NOT Microsoft.GSL_FOUND)
        set(MUSTARD_BUILTIN_MSGSL ON)
        message(NOTICE "***Notice: Microsoft.GSL not found (minimum required is ${MUSTARD_MSGSL_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_MSGSL")
    endif()
endif()

if(MUSTARD_BUILTIN_MSGSL)
    message(STATUS "Mustard will use built-in Microsoft.GSL")
    # check built-in version
    if(MUSTARD_BUILTIN_MSGSL_VERSION VERSION_LESS MUSTARD_MSGSL_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_MSGSL_VERSION is ${MUSTARD_BUILTIN_MSGSL_VERSION}, which is less than the requirement (${MUSTARD_MSGSL_MINIMUM_REQUIRED}). Changing to ${MUSTARD_MSGSL_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_MSGSL_VERSION ${MUSTARD_MSGSL_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_MSGSL_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/GSL-${MUSTARD_BUILTIN_MSGSL_VERSION}")
    set(MUSTARD_BUILTIN_MSGSL_URL "https://github.com/microsoft/GSL/archive/refs/tags/v${MUSTARD_BUILTIN_MSGSL_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_MSGSL_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(MSGSL SOURCE_DIR "${MUSTARD_BUILTIN_MSGSL_SRC_DIR}")
        message(STATUS "Reusing Microsoft.GSL source ${MUSTARD_BUILTIN_MSGSL_SRC_DIR}")
    else()
        FetchContent_Declare(MSGSL SOURCE_DIR "${MUSTARD_BUILTIN_MSGSL_SRC_DIR}"
                                   URL "${MUSTARD_BUILTIN_MSGSL_URL}")
        message(STATUS "Microsoft.GSL will be downloaded from ${MUSTARD_BUILTIN_MSGSL_URL} to ${MUSTARD_BUILTIN_MSGSL_SRC_DIR}")
    endif()
    # set options
    set(GSL_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring Microsoft.GSL (version: ${MUSTARD_BUILTIN_MSGSL_VERSION})")
    FetchContent_MakeAvailable(MSGSL)
    message(STATUS "Downloading (if required) and configuring Microsoft.GSL (version: ${MUSTARD_BUILTIN_MSGSL_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_MSGSL_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/msgsl-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/msgsl-subbuild")
    #     message(FATAL_ERROR "It seems that the download of Microsoft.GSL has failed. You can try running cmake again, or manually download Microsoft.GSL from ${MUSTARD_BUILTIN_MSGSL_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_MSGSL)
    message(STATUS "Looking for Microsoft.GSL - found (version: ${Microsoft.GSL_VERSION})")
else()
    message(STATUS "Looking for Microsoft.GSL - built-in (version: ${MUSTARD_BUILTIN_MSGSL_VERSION})")
endif()
