add_executable(Uniform Uniform.c++)
target_link_libraries(Uniform Mustard::Mustard)

add_executable(Gaussian Gaussian.c++)
target_link_libraries(Gaussian Mustard::Mustard)
