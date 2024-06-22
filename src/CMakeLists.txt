file(GLOB_RECURSE MUSTARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/*.c++)
add_library(Mustard ${MUSTARD_SRC})
add_library(Mustard::Mustard ALIAS Mustard)

# Mustrad
target_include_directories(Mustard PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(Mustard PUBLIC ${MUSTARD_COMPILE_OPTIONS})
target_compile_definitions(Mustard PUBLIC ${MUSTARD_COMPILE_DEFINITIONS})

# envparse
target_link_libraries(Mustard PUBLIC envparse::envparse)

# TimSort
target_link_libraries(Mustard PUBLIC timsort)

# muc
target_link_libraries(Mustard PUBLIC muc::muc)

# EFM
target_link_libraries(Mustard PUBLIC EFM::EFM)

# argparse
target_link_libraries(Mustard PUBLIC argparse::argparse)

# Microsoft.GSL
target_link_libraries(Mustard PUBLIC Microsoft.GSL::GSL)

# backward-cpp
target_include_directories(Mustard PRIVATE $<BUILD_INTERFACE:${BACKWARD_INCLUDE_DIRS}>)
target_compile_definitions(Mustard PRIVATE $<BUILD_INTERFACE:${BACKWARD_DEFINITIONS}>)
if(LIBDW_LIBRARY)
    target_link_libraries(Mustard PRIVATE $<BUILD_INTERFACE:${LIBDW_LIBRARY}>)
endif()
if(LIBBFD_LIBRARY)
    target_link_libraries(Mustard PRIVATE $<BUILD_INTERFACE:${LIBBFD_LIBRARY}>)
endif()
if(LIBDWARF_LIBRARY AND LIBELF_LIBRARY)
    target_link_libraries(Mustard PRIVATE $<BUILD_INTERFACE:${LIBDWARF_LIBRARY}> $<BUILD_INTERFACE:${LIBELF_LIBRARY}>)
endif()

# fmt
target_link_libraries(Mustard PUBLIC fmt::fmt)

# Eigen
target_link_libraries(Mustard PUBLIC Eigen3::Eigen)

# yaml-cpp
target_link_libraries(Mustard PUBLIC yaml-cpp::yaml-cpp)

# MPI
target_link_libraries(Mustard PUBLIC MPI::MPI_C)

# Geant4
target_link_libraries(Mustard PUBLIC ${Geant4_LIBRARIES})

# ROOT
target_link_libraries(Mustard PUBLIC ROOT::Core ROOT::Hist ROOT::Tree ROOT::MathCore ROOT::ROOTDataFrame ROOT::Geom)

# Install
if(MUSTARD_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set_target_properties(Mustard PROPERTIES VERSION ${MUSTARD_VERSION} SOVERSION ${MUSTARD_VERSION})

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Mustard DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            PATTERN *.h++ PATTERN *.inl)
    install(TARGETS Mustard EXPORT MustardConfig
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(EXPORT MustardConfig NAMESPACE Mustard:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
    export(TARGETS Mustard NAMESPACE Mustard:: APPEND FILE MustardConfig.cmake)

    set(MUSTARD_CONFIG_VERSION ${CMAKE_CURRENT_BINARY_DIR}/MustardConfigVersion.cmake)
    write_basic_package_version_file(${MUSTARD_CONFIG_VERSION} COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
    install(FILES ${MUSTARD_CONFIG_VERSION} DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
endif()
