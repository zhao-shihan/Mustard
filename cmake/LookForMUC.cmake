message(STATUS "Looking for muc")

set(MUSTARD_MUC_MINIMUM_REQUIRED 0.25.628)

if(NOT MUSTARD_BUILTIN_MUC)
    find_package(muc ${MUSTARD_MUC_MINIMUM_REQUIRED})
    if(NOT muc_FOUND)
        set(MUSTARD_BUILTIN_MUC ON)
        message(NOTICE "***Notice: muc not found (minimum required is ${MUSTARD_MUC_MINIMUM_REQUIRED}). For the time turning on MUSTARD_BUILTIN_MUC")
    endif()
endif()

if(MUSTARD_BUILTIN_MUC)
    message(STATUS "Mustard will use built-in muc")
    # check built-in version
    if(MUSTARD_BUILTIN_MUC_VERSION VERSION_LESS MUSTARD_MUC_MINIMUM_REQUIRED)
        message(NOTICE "***Notice: Provided MUSTARD_BUILTIN_MUC_VERSION is ${MUSTARD_BUILTIN_MUC_VERSION}, which is less than the requirement (${MUSTARD_MUC_MINIMUM_REQUIRED}). Changing to ${MUSTARD_MUC_MINIMUM_REQUIRED}")
        set(MUSTARD_BUILTIN_MUC_VERSION ${MUSTARD_MUC_MINIMUM_REQUIRED})
    endif()
    # set download dest and URL
    set(MUSTARD_BUILTIN_MUC_SRC_DIR "${MUSTARD_PROJECT_3RDPARTY_DIR}/muc-${MUSTARD_BUILTIN_MUC_VERSION}")
    set(MUSTARD_BUILTIN_MUC_URL "https://github.com/zhao-shihan/muc/archive/refs/tags/v${MUSTARD_BUILTIN_MUC_VERSION}.tar.gz")
    # reuse or download
    include(FetchContent)
    if(EXISTS "${MUSTARD_BUILTIN_MUC_SRC_DIR}/CMakeLists.txt")
        FetchContent_Declare(muc SOURCE_DIR "${MUSTARD_BUILTIN_MUC_SRC_DIR}")
        message(STATUS "Reusing muc source ${MUSTARD_BUILTIN_MUC_SRC_DIR}")
    else()
        FetchContent_Declare(muc SOURCE_DIR "${MUSTARD_BUILTIN_MUC_SRC_DIR}"
                                 URL "${MUSTARD_BUILTIN_MUC_URL}")
        message(STATUS "muc will be downloaded from ${MUSTARD_BUILTIN_MUC_URL} to ${MUSTARD_BUILTIN_MUC_SRC_DIR}")
    endif()
    # set options
    set(MUC_INSTALL ${MUSTARD_INSTALL} CACHE INTERNAL "")
    # configure it
    message(STATUS "Downloading (if required) and configuring muc (version: ${MUSTARD_BUILTIN_MUC_VERSION})")
    FetchContent_MakeAvailable(muc)
    message(STATUS "Downloading (if required) and configuring muc (version: ${MUSTARD_BUILTIN_MUC_VERSION}) - done")
    # # check download
    # if(NOT EXISTS "${MUSTARD_BUILTIN_MUC_SRC_DIR}/CMakeLists.txt")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/muc-build")
    #     file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/_deps/muc-subbuild")
    #     message(FATAL_ERROR "It seems that the download of muc has failed. You can try running cmake again, or manually download muc from ${MUSTARD_BUILTIN_MUC_URL} and extract it to ${MUSTARD_PROJECT_3RDPARTY_DIR} (and keep the directory structure). If the error persists, you can try cleaning the build tree and restarting the build.")
    # endif()
endif()

if(NOT MUSTARD_BUILTIN_MUC)
    message(STATUS "Looking for muc - found (version: ${muc_VERSION})")
else()
    message(STATUS "Looking for muc - built-in (version: ${MUSTARD_BUILTIN_MUC_VERSION})")
endif()
