message(STATUS "Looking for timsort")

set(MUSTARD_TIMSORT_MINIMUM_REQUIRED 3.0.0)

if(NOT MUSTARD_BUILTIN_TIMSORT)
    find_package(timsort ${MUSTARD_TIMSORT_MINIMUM_REQUIRED})
    if(NOT timsort_FOUND)
        set(MUSTARD_BUILTIN_TIMSORT ON)
        message(NOTICE "***Notice: timsort not found (minimum required is ${MUSTARD_TIMSORT_MINIMUM_REQUIRED}). Turning on MUSTARD_BUILTIN_TIMSORT")
    endif()
endif()

if(MUSTARD_BUILTIN_TIMSORT)
    message(STATUS "Mustard will use built-in timsort")
    # check built-in version
    if(MUSTARD_BUILTIN_TIMSORT_VERSION VERSION_LESS MUSTARD_TIMSORT_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_TIMSORT_VERSION is ${MUSTARD_BUILTIN_TIMSORT_VERSION}, which is less than the requirement (${MUSTARD_TIMSORT_MINIMUM_REQUIRED}). Changing to ${MUSTARD_TIMSORT_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_TIMSORT_VERSION ${MUSTARD_TIMSORT_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_TIMSORT_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/cpp-TimSort-3.x.y")
    set(MUSTARD_BUILTIN_TIMSORT_URL "https://github.com/zhao-shihan/cpp-TimSort/archive/refs/heads/3.x.y.zip")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_TIMSORT_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(timsort SOURCE_DIR "${MUSTARD_BUILTIN_TIMSORT_SRC_DIR}")
        message(STATUS "Reusing timsort source ${MUSTARD_BUILTIN_TIMSORT_SRC_DIR}")
    else()
        FetchContent_Declare(timsort SOURCE_DIR "${MUSTARD_BUILTIN_TIMSORT_SRC_DIR}"
                                      URL "${MUSTARD_BUILTIN_TIMSORT_URL}")
        message(STATUS "timsort will be downloaded from ${MUSTARD_BUILTIN_TIMSORT_URL} to ${MUSTARD_BUILTIN_TIMSORT_SRC_DIR}")
    endif()
    # set options
    set(TIMSORT_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring timsort (version: ${MUSTARD_BUILTIN_TIMSORT_VERSION})")
    FetchContent_MakeAvailable(timsort)
    message(STATUS "Downloading (if required) and configuring timsort (version: ${MUSTARD_BUILTIN_TIMSORT_VERSION}) - done")
    # check download
    if(NOT EXISTS "${MUSTARD_BUILTIN_TIMSORT_SRC_DIR}/CMakeLists.txt")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/timsort-build")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/timsort-subbuild")
        message(FATAL_ERROR "It seems that the download of timsort is not successful. You can try to run cmake again, or manually download timsort from ${MUSTARD_BUILTIN_TIMSORT_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try to clean the build tree and restart the build.")
    endif()
endif()

if(NOT MUSTARD_BUILTIN_TIMSORT)
    message(STATUS "Looking for timsort - found (version: ${timsort_VERSION})")
else()
    message(STATUS "Looking for timsort - built-in (version: ${MUSTARD_BUILTIN_TIMSORT_VERSION})")
endif()
