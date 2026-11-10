# Concept Lab 4 — Kill the Switch
#
#   kill_the_switch                the driver
#   kill_the_switch_tests          the behavior tests: pass BEFORE and AFTER
#   kill_the_switch_variant_tests  the Step 4 tests: fail now, pass after
#
# impl/ is globbed on purpose. Adding an ability must cost you zero edits to
# existing files, and that claim is only true if the build does not need
# telling about a new .cpp. Delete impl/dispatch.cpp, add impl/whatever.cpp,
# re-run `cmake --preset course-debug`, and it builds.

set(DD_LAB04_SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/lab_support.cpp)

file(GLOB DD_LAB04_IMPL CONFIGURE_DEPENDS
     ${CMAKE_CURRENT_SOURCE_DIR}/impl/*.cpp)

add_executable(kill_the_switch
               kill_the_switch.cpp ${DD_LAB04_SHARED} ${DD_LAB04_IMPL})
target_include_directories(kill_the_switch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kill_the_switch PRIVATE dd_sanitize_flags)

add_executable(kill_the_switch_tests
               tests.cpp ${DD_LAB04_SHARED} ${DD_LAB04_IMPL})
target_include_directories(kill_the_switch_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kill_the_switch_tests PRIVATE dd_sanitize_flags)

add_executable(kill_the_switch_variant_tests
               variant_tests.cpp ${DD_LAB04_SHARED} ${DD_LAB04_IMPL})
target_include_directories(kill_the_switch_variant_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kill_the_switch_variant_tests PRIVATE dd_sanitize_flags)
