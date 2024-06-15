message(STATUS "Looking for MPI (C interface)")

find_package(MPI 2.0 REQUIRED C)

message(STATUS "Looking for MPI (C interface) - found (version: ${MPI_C_VERSION})")
