add_executable(CopyableOrMoveable CopyableOrMoveable.c++)
target_link_libraries(CopyableOrMoveable Mustard::Mustard)

add_executable(VectorConcept VectorConcept.c++)
target_link_libraries(VectorConcept Mustard::Mustard)
