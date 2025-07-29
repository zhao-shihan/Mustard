add_executable(MT1993732Engine MT1993732Engine.c++)
target_link_libraries(MT1993732Engine Mustard::Mustard)

add_executable(MT1993764Engine MT1993764Engine.c++)
target_link_libraries(MT1993764Engine Mustard::Mustard)

add_executable(Xoshiro256SSEngine Xoshiro256SSEngine.c++)
target_link_libraries(Xoshiro256SSEngine Mustard::Mustard)

add_executable(Xoshiro512SSEngine Xoshiro512SSEngine.c++)
target_link_libraries(Xoshiro512SSEngine Mustard::Mustard)
