message(STATUS "Looking for Eigen")

find_package(Eigen3 3.4.0 REQUIRED)

message(STATUS "Looking for Eigen - found (version: ${Eigen3_VERSION})")
