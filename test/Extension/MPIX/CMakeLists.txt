add_executable(TestMasterSlaveScheduler TestMasterSlaveScheduler.c++)
target_link_libraries(TestMasterSlaveScheduler Mustard::Mustard)

add_executable(TestStaticScheduler TestStaticScheduler.c++)
target_link_libraries(TestStaticScheduler Mustard::Mustard)

add_executable(TestTaskQueueScheduler TestTaskQueueScheduler.c++)
target_link_libraries(TestTaskQueueScheduler Mustard::Mustard)
