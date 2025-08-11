add_executable(TestCreateTemporaryFile TestCreateTemporaryFile.c++)
target_link_libraries(TestCreateTemporaryFile Mustard::Mustard)

add_executable(TestFile TestFile.c++)
target_link_libraries(TestFile Mustard::Mustard)
