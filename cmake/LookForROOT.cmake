message(STATUS "Looking for ROOT")

find_package(ROOT 6.30.00 REQUIRED Geom)

message(STATUS "Looking for ROOT - found (version: ${ROOT_VERSION})")
