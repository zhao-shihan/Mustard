message(STATUS "Looking for EFM")

set(MUSTARD_EFM_MINIMUM_REQUIRED 0.1.0)

if(NOT MUSTARD_BUILTIN_EFM)
    find_package(EFM ${MUSTARD_EFM_MINIMUM_REQUIRED})
    if(NOT EFM_FOUND)
        set(MUSTARD_BUILTIN_EFM ON)
        message(NOTICE "***Notice: EFM not found (minimum required is ${MUSTARD_EFM_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_EFM")
    endif()
endif()

if(MUSTARD_BUILTIN_EFM)
    message(STATUS "Mustard will use built-in EFM")
    # check built-in version
    if(MUSTARD_BUILTIN_EFM_VERSION VERSION_LESS MUSTARD_EFM_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_EFM_VERSION is ${MUSTARD_BUILTIN_EFM_VERSION}, which is less than the requirement (${MUSTARD_EFM_MINIMUM_REQUIRED}). Changing to ${MUSTARD_EFM_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_EFM_VERSION ${MUSTARD_EFM_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_EFM_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/EFM-${MUSTARD_BUILTIN_EFM_VERSION}")
    set(MUSTARD_BUILTIN_EFM_URL "https://github.com/zhao-shihan/EFM/archive/refs/tags/v${MUSTARD_BUILTIN_EFM_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_EFM_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(EFM SOURCE_DIR "${MUSTARD_BUILTIN_EFM_SRC_DIR}")
        message(STATUS "Reusing EFM source ${MUSTARD_BUILTIN_EFM_SRC_DIR}")
    else()
        FetchContent_Declare(EFM SOURCE_DIR "${MUSTARD_BUILTIN_EFM_SRC_DIR}"
                                 URL "${MUSTARD_BUILTIN_EFM_URL}")
        message(STATUS "EFM will be downloaded from ${MUSTARD_BUILTIN_EFM_URL} to ${MUSTARD_BUILTIN_EFM_SRC_DIR}")
    endif()
    # set options
    set(EFM_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring EFM (version: ${MUSTARD_BUILTIN_EFM_VERSION})")
    FetchContent_MakeAvailable(EFM)
    message(STATUS "Downloading (if required) and configuring EFM (version: ${MUSTARD_BUILTIN_EFM_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_EFM_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/efm-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/efm-subbuild")
    #     message(FATAL_ERROR "It seems that the download of EFM has failed. You can try running cmake again, or manually download EFM from ${MUSTARD_BUILTIN_EFM_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_EFM)
    message(STATUS "Looking for EFM - found (version: ${EFM_VERSION})")
else()
    message(STATUS "Looking for EFM - built-in (version: ${MUSTARD_BUILTIN_EFM_VERSION})")
endif()
