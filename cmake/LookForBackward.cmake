message(STATUS "Looking for backward-cpp")

set(MUSTARD_BACKWARD_MINIMUM_REQUIRED 1.6)

if(NOT MUSTARD_BUILTIN_BACKWARD)
    find_package(Backward QUIET)
    if(NOT Backward_FOUND)
        set(MUSTARD_BUILTIN_BACKWARD ON)
        message(NOTICE "***Notice: backward-cpp not found. Turning on MUSTARD_BUILTIN_BACKWARD")
    endif()
endif()

if(MUSTARD_BUILTIN_BACKWARD)
    message(STATUS "Mustard will use built-in backward-cpp")
    # check built-in version
    if(MUSTARD_BUILTIN_BACKWARD_VERSION VERSION_LESS MUSTARD_BACKWARD_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_BACKWARD_VERSION is ${MUSTARD_BUILTIN_BACKWARD_VERSION}, which is less than the requirement (${MUSTARD_BACKWARD_MINIMUM_REQUIRED}). Changing to ${MUSTARD_BACKWARD_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_BACKWARD_VERSION ${MUSTARD_BACKWARD_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_BACKWARD_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/backward-cpp-${MUSTARD_BUILTIN_BACKWARD_VERSION}")
    set(MUSTARD_BUILTIN_BACKWARD_URL "https://github.com/bombela/backward-cpp/archive/refs/tags/v${MUSTARD_BUILTIN_BACKWARD_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_BACKWARD_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(Backward SOURCE_DIR "${MUSTARD_BUILTIN_BACKWARD_SRC_DIR}")
        message(STATUS "Reusing backward-cpp source ${MUSTARD_BUILTIN_BACKWARD_SRC_DIR}")
    else()
        FetchContent_Declare(Backward SOURCE_DIR "${MUSTARD_BUILTIN_BACKWARD_SRC_DIR}"
                                      URL "${MUSTARD_BUILTIN_BACKWARD_URL}")
        message(STATUS "backward-cpp will be downloaded from ${MUSTARD_BUILTIN_BACKWARD_URL} to ${MUSTARD_BUILTIN_BACKWARD_SRC_DIR}")
    endif()
    # set options
    set(BACKWARD_SHARED ${BUILD_SHARED_LIBS} CACHE INTERNAL "")
    set(BACKWARD_TESTS OFF CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring backward-cpp (version: ${MUSTARD_BUILTIN_BACKWARD_VERSION})")
    FetchContent_MakeAvailable(Backward)
    message(STATUS "Downloading (if required) and configuring backward-cpp (version: ${MUSTARD_BUILTIN_BACKWARD_VERSION}) - done")
    # check download
    if(NOT EXISTS "${MUSTARD_BUILTIN_BACKWARD_SRC_DIR}/CMakeLists.txt")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/backward-build")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/backward-subbuild")
        message(FATAL_ERROR "It seems that the download of backward-cpp is not successful. You can try to run cmake again, or manually download backward-cpp from ${MUSTARD_BUILTIN_BACKWARD_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try to clean the build tree and restart the build.")
    endif()
endif()

if(NOT MUSTARD_BUILTIN_BACKWARD)
    message(STATUS "Looking for backward-cpp - found")
else()
    message(STATUS "Looking for backward-cpp - built-in (version: ${MUSTARD_BUILTIN_BACKWARD_VERSION})")
endif()
