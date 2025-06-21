add_executable(TestSharedMemoryScheduler TestSharedMemoryScheduler.c++)
target_link_libraries(TestSharedMemoryScheduler Mustard::Mustard)

add_executable(TestMasterWorkerScheduler TestMasterWorkerScheduler.c++)
target_link_libraries(TestMasterWorkerScheduler Mustard::Mustard)

add_executable(TestStaticScheduler TestStaticScheduler.c++)
target_link_libraries(TestStaticScheduler Mustard::Mustard)

# add_executable(TestHierarchicalScheduler TestHierarchicalScheduler.c++)
# target_link_libraries(TestHierarchicalScheduler Mustard::Mustard)
