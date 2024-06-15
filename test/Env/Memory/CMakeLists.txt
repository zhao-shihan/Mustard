add_executable(TestSingleton TestSingleton.c++)
target_link_libraries(TestSingleton Mustard::Mustard)

add_executable(TestPassiveSingleton TestPassiveSingleton.c++)
target_link_libraries(TestPassiveSingleton Mustard::Mustard)
