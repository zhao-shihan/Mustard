file(GLOB_RECURSE MUSTARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/*.c++)
add_library(Mustard ${MUSTARD_SRC})
add_library(Mustard::Mustard ALIAS Mustard)

# Mustard
target_include_directories(Mustard PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_options(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_OPTIONS}
                               PRIVATE ${MUSTARD_PRIVATE_COMPILE_OPTIONS})
target_compile_definitions(Mustard PUBLIC ${MUSTARD_PUBLIC_COMPILE_DEFINITIONS})

# Generate version header
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Mustard/Version.inl.in
                              ${CMAKE_CURRENT_SOURCE_DIR}/Mustard/Version.inl
                              INSTALL_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Mustard)

# envparse
target_link_libraries(Mustard PUBLIC envparse::envparse)

# muc
target_link_libraries(Mustard PUBLIC muc::muc)

# EFM
target_link_libraries(Mustard PUBLIC EFM::EFM)

# backward-cpp
target_link_libraries(Mustard PUBLIC Backward::Interface)

# argparse
target_link_libraries(Mustard PUBLIC argparse::argparse)

# indicators
target_link_libraries(Mustard PUBLIC indicators::indicators)

# MPLR
target_link_libraries(Mustard PUBLIC mplr::mplr)

# Microsoft.GSL
target_link_libraries(Mustard PUBLIC Microsoft.GSL::GSL)

# fmt
target_link_libraries(Mustard PUBLIC fmt::fmt)

# Eigen
target_link_libraries(Mustard PUBLIC Eigen3::Eigen)

# yaml-cpp
target_link_libraries(Mustard PUBLIC yaml-cpp::yaml-cpp)

# Geant4
target_link_libraries(Mustard PUBLIC ${Geant4_LIBRARIES})

# ROOT
target_link_libraries(Mustard PUBLIC ROOT::Core ROOT::Hist ROOT::Thread ROOT::Tree ROOT::MathCore ROOT::ROOTDataFrame ROOT::Geom)
