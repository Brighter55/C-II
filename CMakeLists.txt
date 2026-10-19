cmake_minimum_required(VERSION 3.21)

project(dungeon_delve_core
        VERSION 0.1.0
        DESCRIPTION "Dungeon Delve encounter engine"
        LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Language standard.
#
# The course targets C++20 as the floor. Your textbook is written to C++23;
# where the book uses something your compiler does not have yet, the course
# materials say so. Do not lower this.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# ---------------------------------------------------------------------------
# Warnings. These are on from Week 1 and they stay on.
# ---------------------------------------------------------------------------
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Sanitizers.
#
# DD_SANITIZE is ON in the course-debug preset. The address sanitizer is what
# finds the memory problems in this codebase; it reports a leak, an
# out-of-bounds read, or a use-after-free at the moment it happens, with a
# stack trace, instead of as a mystery crash later.
#
# Not supported under MSVC in this configuration -- Windows students should
# use the course-debug-msvc preset and read the note in README.md.
# ---------------------------------------------------------------------------
option(DD_SANITIZE "Build with address and undefined-behavior sanitizers" OFF)

add_library(dd_sanitize_flags INTERFACE)

if(DD_SANITIZE AND NOT MSVC)
    target_compile_options(dd_sanitize_flags INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -g)

    # Clang can always detect reads of a dead stack frame. GCC does it at
    # runtime instead, via ASAN_OPTIONS=detect_stack_use_after_return=1 --
    # see README.md. Only add the flag if this compiler understands it.
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-fsanitize-address-use-after-return=always"
                            DD_HAS_UAR_FLAG)
    if(DD_HAS_UAR_FLAG)
        target_compile_options(dd_sanitize_flags INTERFACE
            -fsanitize-address-use-after-return=always)
    endif()
    target_link_options(dd_sanitize_flags INTERFACE
        -fsanitize=address,undefined)
endif()

# ---------------------------------------------------------------------------
# The engine, built twice.
#
# dd_engine is the engine compiled with DD_NO_MAIN so the tests can link it.
# dungeon_delve is the playable program.
#
# Yes, compiling the same source file two different ways is an odd thing to
# find in a codebase. Make a note of it for TRIAGE.md.
# ---------------------------------------------------------------------------
add_library(dd_engine STATIC src/dungeon_delve.cpp)
target_include_directories(dd_engine PUBLIC include)
target_compile_definitions(dd_engine PRIVATE DD_NO_MAIN)
target_link_libraries(dd_engine PUBLIC dd_sanitize_flags)

add_executable(dungeon_delve src/dungeon_delve.cpp)
target_include_directories(dungeon_delve PRIVATE include)
target_link_libraries(dungeon_delve PRIVATE dd_sanitize_flags)

# ---------------------------------------------------------------------------
# Concept Lab programs.
#
# These are small standalone programs, one per week, that practice the week's
# idea outside the engine. They are not part of the game and nothing in the
# engine depends on them.
# ---------------------------------------------------------------------------
add_executable(ownership_audit labs/week01_ownership_audit/ownership_audit.cpp)
target_link_libraries(ownership_audit PRIVATE dd_sanitize_flags)

# ---------------------------------------------------------------------------
# Tests.
# ---------------------------------------------------------------------------
enable_testing()

add_executable(regression tests/regression.cpp)
target_link_libraries(regression PRIVATE dd_engine dd_sanitize_flags)

add_test(NAME encounter_resolves
         COMMAND regression encounter_resolves)

add_test(NAME loot_table_intact
         COMMAND regression loot_table_intact)

add_test(NAME roster_survives_reallocation
         COMMAND regression roster_survives_reallocation)

# Leak detection is deliberately OFF for these three tests.
#
# Each test targets exactly one of the three seeded defects. This codebase
# leaks almost everything it allocates, and if leak reports failed these tests
# too you could not tell which problem you were looking at.
#
# The ownership half of Build 1 is checked separately, by running the game
# itself under the sanitizer:
#
#     ./build/course-debug/dungeon_delve --quiet
#
# That run must report no leaks and no invalid accesses before you submit.
set_tests_properties(encounter_resolves loot_table_intact roster_survives_reallocation
    PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0:abort_on_error=0"
        TIMEOUT 60)
