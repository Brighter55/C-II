# Concept Lab 3 — Same Problem, Twice
#
# Four targets, because one lab builds two implementations of one header:
#
#   same_problem_twice_inherit         the driver, inheritance version
#   same_problem_twice_compose         the driver, composition version
#   same_problem_twice_inherit_tests   the supplied tests, inheritance version
#   same_problem_twice_compose_tests   the supplied tests, composition version
#
# The two implementation folders are globbed, so you can split either design
# across as many .cpp files as it needs without ever editing this file. That
# matters: "how many files did I have to touch" is one of the things this lab
# measures, and the build should not add a file to that count.

set(DD_LAB03_SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/lab_support.cpp)

file(GLOB DD_LAB03_INHERIT CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/impl_inherit/*.cpp)
file(GLOB DD_LAB03_COMPOSE CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/impl_compose/*.cpp)

# ---- the driver, built twice ---------------------------------------------
add_executable(same_problem_twice_inherit
               same_problem_twice.cpp ${DD_LAB03_SHARED} ${DD_LAB03_INHERIT})
target_include_directories(same_problem_twice_inherit
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(same_problem_twice_inherit PRIVATE dd_sanitize_flags)

add_executable(same_problem_twice_compose
               same_problem_twice.cpp ${DD_LAB03_SHARED} ${DD_LAB03_COMPOSE})
target_include_directories(same_problem_twice_compose
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(same_problem_twice_compose PRIVATE dd_sanitize_flags)

# ---- the supplied tests, built twice -------------------------------------
add_executable(same_problem_twice_inherit_tests
               tests.cpp ${DD_LAB03_SHARED} ${DD_LAB03_INHERIT})
target_include_directories(same_problem_twice_inherit_tests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(same_problem_twice_inherit_tests PRIVATE dd_sanitize_flags)

add_executable(same_problem_twice_compose_tests
               tests.cpp ${DD_LAB03_SHARED} ${DD_LAB03_COMPOSE})
target_include_directories(same_problem_twice_compose_tests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(same_problem_twice_compose_tests PRIVATE dd_sanitize_flags)
