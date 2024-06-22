message(STATUS "Looking for yaml-cpp")

set(MUSTARD_YAML_CPP_MINIMUM_REQUIRED 0.8.0)

if(NOT MUSTARD_BUILTIN_YAML_CPP)
    find_package(yaml-cpp ${MUSTARD_YAML_CPP_MINIMUM_REQUIRED})
    if(NOT yaml-cpp_FOUND)
        set(MUSTARD_BUILTIN_YAML_CPP ON)
        message(NOTICE "***Notice: yaml-cpp not found (minimum required is ${MUSTARD_YAML_CPP_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_YAML_CPP")
    endif()
endif()

if(MUSTARD_BUILTIN_YAML_CPP)
    message(STATUS "Mustard will use built-in yaml-cpp")
    # check built-in version
    if(MUSTARD_BUILTIN_YAML_CPP_VERSION VERSION_LESS MUSTARD_YAML_CPP_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_YAML_CPP_VERSION is ${MUSTARD_BUILTIN_YAML_CPP_VERSION}, which is less than the requirement (${MUSTARD_YAML_CPP_MINIMUM_REQUIRED}). Changing to ${MUSTARD_YAML_CPP_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_YAML_CPP_VERSION ${MUSTARD_YAML_CPP_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_YAML_CPP_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/yaml-cpp-${MUSTARD_BUILTIN_YAML_CPP_VERSION}")
    set(MUSTARD_BUILTIN_YAML_CPP_URL "https://github.com/jbeder/yaml-cpp/archive/refs/tags/${MUSTARD_BUILTIN_YAML_CPP_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_YAML_CPP_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(yaml-cpp SOURCE_DIR "${MUSTARD_BUILTIN_YAML_CPP_SRC_DIR}")
        message(STATUS "Reusing yaml-cpp source ${MUSTARD_BUILTIN_YAML_CPP_SRC_DIR}")
    else()
        FetchContent_Declare(yaml-cpp SOURCE_DIR "${MUSTARD_BUILTIN_YAML_CPP_SRC_DIR}"
                                      URL "${MUSTARD_BUILTIN_YAML_CPP_URL}")
        message(STATUS "yaml-cpp will be downloaded from ${MUSTARD_BUILTIN_YAML_CPP_URL} to ${MUSTARD_BUILTIN_YAML_CPP_SRC_DIR}")
    endif()
    # set options
    set(YAML_CPP_BUILD_CONTRIB ON CACHE INTERNAL "")
    set(YAML_CPP_BUILD_TOOLS OFF CACHE INTERNAL "")
    set(YAML_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE INTERNAL "")
    set(YAML_CPP_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    set(YAML_CPP_FORMAT_SOURCE OFF CACHE INTERNAL "")
    set(YAML_CPP_BUILD_TESTS OFF CACHE INTERNAL "")
    set(YAML_MSVC_SHARED_RT ${MUSTARD_USE_SHARED_MSVC_RT} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring yaml-cpp (version: ${MUSTARD_BUILTIN_YAML_CPP_VERSION})")
    FetchContent_MakeAvailable(yaml-cpp)
    message(STATUS "Downloading (if required) and configuring yaml-cpp (version: ${MUSTARD_BUILTIN_YAML_CPP_VERSION}) - done")
    # check download
    if(NOT EXISTS "${MUSTARD_BUILTIN_YAML_CPP_SRC_DIR}/CMakeLists.txt")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/yaml-cpp-build")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/yaml-cpp-subbuild")
        message(FATAL_ERROR "It seems that the download of yaml-cpp has failed. You can try running cmake again, or manually download yaml-cpp from ${MUSTARD_BUILTIN_YAML_CPP_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    endif()
endif()

if(NOT MUSTARD_BUILTIN_YAML_CPP)
    message(STATUS "Looking for yaml-cpp - found (version: ${yaml-cpp_VERSION})")
else()
    message(STATUS "Looking for yaml-cpp - built-in (version: ${MUSTARD_BUILTIN_YAML_CPP_VERSION})")
endif()
