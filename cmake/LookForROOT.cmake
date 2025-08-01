message(STATUS "Looking for ROOT")

set(MUSTARD_ROOT_MINIMUM_REQUIRED 6.30.00)
set(MUSTARD_ROOT_REQUIRED_COMPONENTS Core Hist Thread Tree MathCore ROOTDataFrame Geom)
find_package(ROOT 6.30.00 REQUIRED ${MUSTARD_ROOT_REQUIRED_COMPONENTS})

message(STATUS "Looking for ROOT - found (version: ${ROOT_VERSION})")
