message(STATUS "Looking for Eigen")

set(MUSTARD_EIGEN_MINIMUM_REQUIRED 3.4.0)
find_package(Eigen3 ${MUSTARD_EIGEN_MINIMUM_REQUIRED} REQUIRED)

message(STATUS "Looking for Eigen - found (version: ${Eigen3_VERSION})")
