add_executable(MT1993732 MT1993732.c++)
target_link_libraries(MT1993732 Mustard::Mustard)

add_executable(MT1993764 MT1993764.c++)
target_link_libraries(MT1993764 Mustard::Mustard)

add_executable(Xoshiro256Plus Xoshiro256Plus.c++)
target_link_libraries(Xoshiro256Plus Mustard::Mustard)

add_executable(Xoshiro256PP Xoshiro256PP.c++)
target_link_libraries(Xoshiro256PP Mustard::Mustard)

add_executable(Xoshiro256SS Xoshiro256SS.c++)
target_link_libraries(Xoshiro256SS Mustard::Mustard)

add_executable(Xoshiro512Plus Xoshiro512Plus.c++)
target_link_libraries(Xoshiro512Plus Mustard::Mustard)

add_executable(Xoshiro512PP Xoshiro512PP.c++)
target_link_libraries(Xoshiro512PP Mustard::Mustard)

add_executable(Xoshiro512SS Xoshiro512SS.c++)
target_link_libraries(Xoshiro512SS Mustard::Mustard)
