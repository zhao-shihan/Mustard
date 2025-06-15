add_executable(TestDynamicScheduler TestDynamicScheduler.c++)
target_link_libraries(TestDynamicScheduler Mustard::Mustard)

add_executable(TestStaticScheduler TestStaticScheduler.c++)
target_link_libraries(TestStaticScheduler Mustard::Mustard)

add_executable(TestTaskQueueScheduler TestTaskQueueScheduler.c++)
target_link_libraries(TestTaskQueueScheduler Mustard::Mustard)
