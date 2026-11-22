# Concept Lab 5 — Generic and Non-Generic, Side by Side
add_executable(generic_side_by_side
    generic_side_by_side.cpp
    legacy_loot.cpp)
target_include_directories(generic_side_by_side PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(generic_side_by_side PRIVATE dd_sanitize_flags)

add_executable(generic_side_by_side_tests
    tests.cpp
    legacy_loot.cpp)
target_include_directories(generic_side_by_side_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(generic_side_by_side_tests PRIVATE dd_sanitize_flags)
