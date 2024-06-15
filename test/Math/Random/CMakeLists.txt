add_executable(RandomNumberDistribution RandomNumberDistribution.c++)
target_link_libraries(RandomNumberDistribution Mustard::Mustard)

add_subdirectory(Distribution)
add_subdirectory(Generator)
