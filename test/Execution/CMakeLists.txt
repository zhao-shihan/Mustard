add_executable(TestExecutor TestExecutor.c++)
target_link_libraries(TestExecutor Mustard::Mustard)

add_executable(TestExecutorSequential TestExecutorSequential.c++)
target_link_libraries(TestExecutorSequential Mustard::Mustard)
