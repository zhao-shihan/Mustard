file(GLOB_RECURSE MUSTARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/*.c++)
add_library(Mustard ${MUSTARD_SRC})
add_library(Mustard::Mustard ALIAS Mustard)

# Mustard
target_include_directories(Mustard PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_OPTIONS}
                               PRIVATE ${MUSTARD_PRIVATE_COMPILE_OPTIONS})
target_compile_definitions(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_DEFINITIONS})

# envparse
target_link_libraries(Mustard PUBLIC envparse::envparse)

# muc
target_link_libraries(Mustard PUBLIC muc::muc)

# EFM
target_link_libraries(Mustard PUBLIC EFM::EFM)

# argparse
target_link_libraries(Mustard PUBLIC argparse::argparse)

# Microsoft.GSL
target_link_libraries(Mustard PUBLIC Microsoft.GSL::GSL)

# backward-cpp
target_link_libraries(Mustard PUBLIC Backward::Interface)

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

    set_target_properties(Mustard PROPERTIES VERSION ${MUSTARD_VERSION}
                                             SOVERSION ${MUSTARD_VERSION}
                                             DEBUG_POSTFIX ${MUSTARD_DEBUG_POSTFIX})

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Mustard DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            PATTERN *.h++ PATTERN *.inl)
    install(TARGETS Mustard EXPORT MustardTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(EXPORT MustardTargets
            NAMESPACE Mustard::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)

    include(CMakePackageConfigHelpers)
    configure_package_config_file(${MUSTARD_PROJECT_CMAKE_DIR}/MustardConfig.cmake.in
                                  ${CMAKE_CURRENT_BINARY_DIR}/MustardConfig.cmake
                                  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/MustardConfigVersion.cmake
                                     COMPATIBILITY SameMajorVersion)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MustardConfig.cmake
                  ${CMAKE_CURRENT_BINARY_DIR}/MustardConfigVersion.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Mustard)
endif()
