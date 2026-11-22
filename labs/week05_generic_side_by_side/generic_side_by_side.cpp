// ===========================================================================
//  IS 267 — Concept Lab 5:  Generic and Non-Generic, Side by Side
//
//  generic_side_by_side.cpp -- the driver. You do not have to edit this file.
//
//  It runs the same operation three ways and prints all three next to each
//  other: once through Satchel<std::string>, once through Satchel<int, 12>
//  (the second instantiation), and once through raw_remove_at() on a working
//  copy of LOOT_TABLE. Then it round-trips the table through the bridge and
//  checks that LOOT_TABLE came out the far side untouched.
//
//  On the untouched scaffold this prints empty satchels and a row of
//  failures. That is correct. Build it, run it, read it, then start filling
//  in the TODOs in satchel.h and legacy_loot.cpp.
//
//  Build and run, from the top of the repository:
//      cmake --preset course-debug
//      cmake --build --preset course-debug
//      ./build/course-debug/generic_side_by_side
//      ./build/course-debug/generic_side_by_side_tests
// ===========================================================================
#include <cstdio>
#include <cstring>
#include <string>

#include "legacy_loot.h"
#include "satchel.h"

static void banner(const char* label)
{
    std::printf("\n--- %s ---\n", label);
}

// The index this program removes, on every one of the three sides.
static constexpr int kRemoveIndex = 2;   // "Mouldy Rations"

int main()
{
    std::printf("IS 267 Concept Lab 5 - Generic and Non-Generic, Side by Side\n");
    std::printf("One operation, implemented twice, with a bridge between.\n\n");

    print_loot_table();

    // Snapshot the raw table BEFORE anything touches it: both the pointer
    // values and the characters they point at. Everything below has to leave
    // both of these alone.
    const char* snapshot_ptr[LOOT_COUNT];
    std::string snapshot_txt[LOOT_COUNT];
    for (int i = 0; i < LOOT_COUNT; i++) {
        snapshot_ptr[i] = LOOT_TABLE[i];
        snapshot_txt[i] = LOOT_TABLE[i];
    }

    // ---- the template side, instantiation 1: names ------------------------
    banner("Satchel<std::string> -- loot names");
    Satchel<std::string> names;
    int loaded = LegacyLootBridge::load(LOOT_TABLE, LOOT_COUNT, names);
    std::printf("  bridge loaded %d of %d entries\n", loaded, LOOT_COUNT);
    std::printf("  before remove_at(%d): size %d\n", kRemoveIndex, names.size());
    bool removed_t = names.remove_at(kRemoveIndex);
    std::printf("  remove_at(%d) returned %s, size now %d\n",
                kRemoveIndex, removed_t ? "true" : "false", names.size());
    for (int i = 0; i < names.size(); i++)
        std::printf("    [%2d] %s\n", i, names.at(i).c_str());

    // ---- the template side, instantiation 2: values -----------------------
    // Same class template, different T. If this one does not work, the
    // template is not generic -- it is a container that happens to compile.
    banner("Satchel<int, 12> -- loot values (second instantiation)");
    Satchel<int, 12> values;
    for (int i = 0; i < LOOT_COUNT; i++)
        values.add(LOOT_VALUE[i]);
    std::printf("  size after %d adds: %d\n", LOOT_COUNT, values.size());
    bool removed_v = values.remove_at(kRemoveIndex);
    std::printf("  remove_at(%d) returned %s, size now %d\n",
                kRemoveIndex, removed_v ? "true" : "false", values.size());
    std::printf("    ");
    for (int i = 0; i < values.size(); i++)
        std::printf("%d ", values.at(i));
    std::printf("\n");

    // ---- the raw side -----------------------------------------------------
    // A working copy. raw_remove_at never runs on LOOT_TABLE itself.
    banner("raw const char*[] -- the same operation, pre-template style");
    const char* work[LOOT_COUNT];
    int work_count = LOOT_COUNT;
    for (int i = 0; i < LOOT_COUNT; i++)
        work[i] = LOOT_TABLE[i];

    std::printf("  before raw_remove_at(%d): count %d\n", kRemoveIndex, work_count);
    bool removed_r = raw_remove_at(work, &work_count, kRemoveIndex);
    std::printf("  raw_remove_at(%d) returned %s, count now %d\n",
                kRemoveIndex, removed_r ? "true" : "false", work_count);
    for (int i = 0; i < work_count; i++)
        std::printf("    [%2d] %s\n", i, work[i] ? work[i] : "(null)");

    // ---- do the two sides agree? -----------------------------------------
    banner("Do the two implementations of the operation agree?");
    bool agree = (names.size() == work_count);
    for (int i = 0; i < work_count && agree; i++) {
        if (i >= names.size() || work[i] == nullptr) { agree = false; break; }
        agree = (names.at(i) == work[i]);
    }
    std::printf("  %s\n", agree ? "yes -- same count, same order, same text"
                                : "NO -- they disagree (expected, on the scaffold)");

    // ---- the round trip ---------------------------------------------------
    banner("Round trip: raw -> Satchel -> raw");
    Satchel<std::string> trip;
    int in  = LegacyLootBridge::load(LOOT_TABLE, LOOT_COUNT, trip);
    const char* back[LOOT_COUNT] = {};
    int out = LegacyLootBridge::store(trip, back, LOOT_COUNT);
    std::printf("  load returned %d, store returned %d\n", in, out);

    bool identical = (in == LOOT_COUNT && out == LOOT_COUNT);
    for (int i = 0; i < LOOT_COUNT && identical; i++)
        identical = (back[i] != nullptr &&
                     std::strcmp(back[i], LOOT_TABLE[i]) == 0);
    std::printf("  round trip identical to the original: %s\n",
                identical ? "yes" : "no");

    bool no_copies = (out == LOOT_COUNT);
    for (int i = 0; i < out && no_copies; i++)
        no_copies = (back[i] == trip.at(i).c_str());
    std::printf("  store handed out the satchel's own buffers (no copies): %s\n",
                no_copies ? "yes" : "no");

    // ---- is LOOT_TABLE still what it was? --------------------------------
    banner("Is LOOT_TABLE still intact?");
    bool preserved = true;
    for (int i = 0; i < LOOT_COUNT; i++) {
        if (LOOT_TABLE[i] != snapshot_ptr[i]) {
            std::printf("  slot %d: POINTER CHANGED -- the table was rewritten\n", i);
            preserved = false;
        } else if (snapshot_txt[i] != LOOT_TABLE[i]) {
            std::printf("  slot %d: TEXT CHANGED\n", i);
            preserved = false;
        }
    }
    std::printf("  %s\n", preserved
        ? "intact -- same pointers, same text"
        : "NOT INTACT -- the bridge modified the table. This fails the lab.");

    std::printf("\n=======================================\n");
    std::printf("Now run the tests, then write ANALYSIS.md.\n");
    std::printf("Both lists in the report must be non-empty.\n");
    std::printf("=======================================\n");
}
