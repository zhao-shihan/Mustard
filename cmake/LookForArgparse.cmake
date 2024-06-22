message(STATUS "Looking for argparse")

set(MUSTARD_ARGPARSE_MINIMUM_REQUIRED 3.0)

if(NOT MUSTARD_BUILTIN_ARGPARSE)
    find_package(argparse ${MUSTARD_ARGPARSE_MINIMUM_REQUIRED}.0)
    if(NOT argparse_FOUND)
        set(MUSTARD_BUILTIN_ARGPARSE ON)
        message(NOTICE "***Notice: argparse not found (minimum required is ${MUSTARD_ARGPARSE_MINIMUM_REQUIRED}). Turning on MUSTARD_BUILTIN_ARGPARSE")
    endif()
endif()

if(MUSTARD_BUILTIN_ARGPARSE)
    message(STATUS "Mustard will use built-in argparse")
    # check built-in version
    if(MUSTARD_BUILTIN_ARGPARSE_VERSION VERSION_LESS MUSTARD_ARGPARSE_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_ARGPARSE_VERSION is ${MUSTARD_BUILTIN_ARGPARSE_VERSION}, which is less than the requirement (${MUSTARD_ARGPARSE_MINIMUM_REQUIRED}). Changing to ${MUSTARD_ARGPARSE_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_ARGPARSE_VERSION ${MUSTARD_ARGPARSE_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_ARGPARSE_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/argparse-master")
    set(MUSTARD_BUILTIN_ARGPARSE_URL "https://github.com/p-ranav/argparse/archive/refs/heads/master.zip")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_ARGPARSE_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(argparse SOURCE_DIR "${MUSTARD_BUILTIN_ARGPARSE_SRC_DIR}")
        message(STATUS "Reusing argparse source ${MUSTARD_BUILTIN_ARGPARSE_SRC_DIR}")
    else()
        FetchContent_Declare(argparse SOURCE_DIR "${MUSTARD_BUILTIN_ARGPARSE_SRC_DIR}"
                                      URL "${MUSTARD_BUILTIN_ARGPARSE_URL}")
        message(STATUS "argparse will be downloaded from ${MUSTARD_BUILTIN_ARGPARSE_URL} to ${MUSTARD_BUILTIN_ARGPARSE_SRC_DIR}")
    endif()
    # set options
    set(ARGPARSE_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring argparse (version: ${MUSTARD_BUILTIN_ARGPARSE_VERSION})")
    FetchContent_MakeAvailable(argparse)
    message(STATUS "Downloading (if required) and configuring argparse (version: ${MUSTARD_BUILTIN_ARGPARSE_VERSION}) - done")
    # check download
    if(NOT EXISTS "${MUSTARD_BUILTIN_ARGPARSE_SRC_DIR}/CMakeLists.txt")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/argparse-build")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/argparse-subbuild")
        message(FATAL_ERROR "It seems that the download of argparse is not successful. You can try to run cmake again, or manually download argparse from ${MUSTARD_BUILTIN_ARGPARSE_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try to clean the build tree and restart the build.")
    endif()
endif()

if(NOT MUSTARD_BUILTIN_ARGPARSE)
    message(STATUS "Looking for argparse - found (version: ${argparse_VERSION})")
else()
    message(STATUS "Looking for argparse - built-in (version: ${MUSTARD_BUILTIN_ARGPARSE_VERSION})")
endif()
