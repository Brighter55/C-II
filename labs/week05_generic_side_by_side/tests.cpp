/* ===========================================================================
   IS 267 — Concept Lab 5:  Generic and Non-Generic, Side by Side

   tests.cpp -- the supplied tests. These are the acceptance criteria from
   the assignment page, written down as code.

   Every one of them FAILS on the scaffold as shipped. All of them must PASS
   before you submit.

   Run them with:
       ./build/course-debug/generic_side_by_side_tests

   You should not need to change this file. If you think a test is wrong,
   say so in ANALYSIS.md and bring it to the discussion -- do not quietly
   edit the test until it agrees with your code.
   =========================================================================== */

#include <cstdio>
#include <cstring>
#include <string>

#include "legacy_loot.h"
#include "satchel.h"

/* ---- a very small test harness ------------------------------------------ */

static int g_failures = 0;

static void check(bool condition, const char* what)
{
    if (condition) {
        std::printf("  ok    %s\n", what);
    } else {
        std::printf("  FAIL  %s\n", what);
        g_failures++;
    }
}

static void group(const char* label)
{
    std::printf("\n%s\n", label);
}

/* The pointer values and the characters of LOOT_TABLE, captured before any
   test has had a chance to touch it. */
static const char* g_snapshot_ptr[LOOT_COUNT];
static std::string g_snapshot_txt[LOOT_COUNT];

/* =========================================================================
   TEST 1 -- the class template works, for two different types

   Acceptance criterion: "The template instantiates for two or more types."
   Satchel<std::string> and Satchel<int, 12> are two instantiations of one
   class template, and the operation has to behave the same in both.
   ========================================================================= */
static void test_template_two_types(void)
{
    group("template side -- two instantiations");

    Satchel<std::string> names;
    check(names.size() == 0, "Satchel<std::string> starts empty");
    check(names.add("Rusted Dagger"),  "add returns true on a non-full satchel");
    check(names.add("Copper Ring"),    "add again");
    check(names.add("Glass Eye"),      "add a third");
    check(names.size() == 3,           "size tracks entries, not capacity");
    if (names.size() == 3) {
        check(names.at(0) == "Rusted Dagger", "at(0) is the first entry added");
        check(names.at(2) == "Glass Eye",     "at(2) is the third entry added");
    } else {
        check(false, "at(0) is the first entry added");
        check(false, "at(2) is the third entry added");
    }

    Satchel<int, 12> values;
    check(values.size() == 0, "Satchel<int, 12> starts empty");
    for (int i = 0; i < 12; i++) values.add(LOOT_VALUE[i]);
    check(values.size() == 12, "Satchel<int, 12> holds twelve ints");
    check(values.full(),       "a satchel at capacity reports full()");
    check(!values.add(99),     "add on a full satchel returns false");
    check(values.size() == 12, "a refused add does not change the size");
}

/* =========================================================================
   TEST 2 -- remove_at, the template way

   Order-preserving removal. Swapping the hole with the last entry is
   faster and fails here: the satchel is ordered.
   ========================================================================= */
static void test_template_remove_at(void)
{
    group("template side -- remove_at closes the gap, in order");

    Satchel<int, 8> s;
    for (int v = 10; v <= 50; v += 10) s.add(v);   /* 10 20 30 40 50 */

    check(s.size() == 5, "five entries before the removal");
    check(s.remove_at(2), "remove_at(2) returns true");
    check(s.size() == 4, "the count dropped by exactly one");

    bool order_ok = (s.size() == 4);
    const int want[4] = { 10, 20, 40, 50 };
    for (int i = 0; i < 4 && order_ok; i++) order_ok = (s.at(i) == want[i]);
    check(order_ok, "the survivors kept their original order (10 20 40 50)");

    check(!s.remove_at(4),  "remove_at past the end returns false");
    check(!s.remove_at(-1), "remove_at(-1) returns false");
    check(s.size() == 4,    "a refused removal does not change the size");

    check(s.remove_at(0), "remove_at(0) returns true");
    check(s.size() == 3 && s.at(0) == 20, "removing the front shifts the rest down");
}

/* =========================================================================
   TEST 3 -- remove_at, the raw way

   The same operation against a raw const char* array and a separate count.
   The extra clause is the tail: the slot that falls off the end is cleared,
   so a stale pointer cannot be read back out past the count. Nothing on
   the template side needed that clause.
   ========================================================================= */
static void test_raw_remove_at(void)
{
    group("raw side -- the same operation, pre-template style");

    const char* work[LOOT_COUNT];
    int count = LOOT_COUNT;
    for (int i = 0; i < LOOT_COUNT; i++) work[i] = LOOT_TABLE[i];

    check(raw_remove_at(work, &count, 2), "raw_remove_at(2) returns true");
    check(count == LOOT_COUNT - 1, "the count dropped by exactly one");

    bool order_ok = (count == LOOT_COUNT - 1);
    for (int i = 0; i < count && order_ok; i++) {
        const char* want = (i < 2) ? LOOT_TABLE[i] : LOOT_TABLE[i + 1];
        order_ok = (work[i] != nullptr && std::strcmp(work[i], want) == 0);
    }
    check(order_ok, "the survivors kept their original order");

    check(work[LOOT_COUNT - 1] == nullptr,
          "the slot that fell off the end was cleared to nullptr");

    check(!raw_remove_at(work, &count, count), "an index at the count returns false");
    check(!raw_remove_at(work, &count, -1),    "a negative index returns false");
    check(!raw_remove_at(nullptr, &count, 0),  "a null slots pointer returns false");
    check(!raw_remove_at(work, nullptr, 0),    "a null count pointer returns false");
    check(count == LOOT_COUNT - 1, "none of the refusals changed the count");
}

/* =========================================================================
   TEST 4 -- the bridge, raw -> template
   ========================================================================= */
static void test_bridge_load(void)
{
    group("bridge -- raw into the template container");

    Satchel<std::string> s;
    int n = LegacyLootBridge::load(LOOT_TABLE, LOOT_COUNT, s);
    check(n == LOOT_COUNT, "load reports every entry it appended");
    check(s.size() == LOOT_COUNT, "the satchel holds all twelve entries");

    bool same = (s.size() == LOOT_COUNT);
    for (int i = 0; i < LOOT_COUNT && same; i++) same = (s.at(i) == LOOT_TABLE[i]);
    check(same, "the entries arrived in table order, text intact");

    /* Appending twelve more would overflow a sixteen-slot satchel. */
    int again = LegacyLootBridge::load(LOOT_TABLE, LOOT_COUNT, s);
    check(again == -1, "load returns -1 when the satchel cannot hold the batch");
    check(s.size() == LOOT_COUNT, "a refused load appended nothing");

    Satchel<std::string> empty_dest;
    check(LegacyLootBridge::load(nullptr, LOOT_COUNT, empty_dest) == -1,
          "load returns -1 on a null table");
    check(LegacyLootBridge::load(LOOT_TABLE, -1, empty_dest) == -1,
          "load returns -1 on a negative count");
    check(empty_dest.size() == 0, "neither refusal appended anything");
}

/* =========================================================================
   TEST 5 -- the bridge, template -> raw, and the round trip

   Acceptance criterion: "Round-trip conversion produces identical data."
   ========================================================================= */
static void test_bridge_round_trip(void)
{
    group("bridge -- round trip, and no second copy on the way out");

    Satchel<std::string> s;
    int in = LegacyLootBridge::load(LOOT_TABLE, LOOT_COUNT, s);

    const char* back[LOOT_COUNT] = {};
    int out = LegacyLootBridge::store(s, back, LOOT_COUNT);
    check(in == LOOT_COUNT && out == LOOT_COUNT,
          "load and store both report all twelve entries");

    bool identical = (out == LOOT_COUNT);
    for (int i = 0; i < LOOT_COUNT && identical; i++)
        identical = (back[i] != nullptr &&
                     std::strcmp(back[i], LOOT_TABLE[i]) == 0);
    check(identical, "raw -> Satchel -> raw is identical to the original");

    /* WITHOUT COPYING TWICE. Each pointer written out must be the satchel's
       own buffer, not a copy of the characters somewhere else. */
    bool aliased = (out == LOOT_COUNT);
    for (int i = 0; i < out && aliased; i++)
        aliased = (back[i] == s.at(i).c_str());
    check(aliased, "store handed out the satchel's own buffers, not copies");

    /* A removal on the template side shows up on the raw side, in order. */
    check(s.remove_at(0), "remove the front entry of the satchel");
    const char* after[LOOT_COUNT] = {};
    int out2 = LegacyLootBridge::store(s, after, LOOT_COUNT);
    check(out2 == LOOT_COUNT - 1, "store now reports eleven entries");
    bool shifted = (out2 == LOOT_COUNT - 1);
    for (int i = 0; i < out2 && shifted; i++)
        shifted = (after[i] != nullptr &&
                   std::strcmp(after[i], LOOT_TABLE[i + 1]) == 0);
    check(shifted, "the round trip after a removal is still in order");

    const char* tiny[2] = {};
    check(LegacyLootBridge::store(s, tiny, 2) == -1,
          "store returns -1 when dest_capacity is too small");
    check(tiny[0] == nullptr && tiny[1] == nullptr,
          "a refused store wrote nothing");
    check(LegacyLootBridge::store(s, nullptr, LOOT_COUNT) == -1,
          "store returns -1 on a null dest");
}

/* =========================================================================
   TEST 6 -- the raw table survived

   Acceptance criterion: "The bridge preserves the raw array -- deleting it
   fails the lab."

   This compares the POINTER VALUES, not only the characters. Rebuilding the
   table out of freshly allocated copies of the same text fails this test,
   and so it should: the engine holds on to those pointers.
   ========================================================================= */
static void test_loot_table_preserved(void)
{
    group("the legacy side -- LOOT_TABLE is exactly what it was");

    bool pointers_ok = true;
    bool text_ok     = true;
    for (int i = 0; i < LOOT_COUNT; i++) {
        if (LOOT_TABLE[i] != g_snapshot_ptr[i]) pointers_ok = false;
        if (LOOT_TABLE[i] == nullptr ||
            g_snapshot_txt[i] != LOOT_TABLE[i]) text_ok = false;
    }
    check(pointers_ok, "every slot still holds the pointer it started with");
    check(text_ok,     "every entry still reads the same");
    check(loot_count() == LOOT_COUNT, "loot_count() is unchanged");
    check(loot_at(0) != nullptr &&
          std::strcmp(loot_at(0), "Rusted Dagger") == 0,
          "the engine can still index the table directly");
}

int main()
{
    for (int i = 0; i < LOOT_COUNT; i++) {
        g_snapshot_ptr[i] = LOOT_TABLE[i];
        g_snapshot_txt[i] = LOOT_TABLE[i];
    }

    std::printf("IS 267 Concept Lab 5 tests -- Generic and Non-Generic, Side by Side\n");

    test_template_two_types();
    test_template_remove_at();
    test_raw_remove_at();
    test_bridge_load();
    test_bridge_round_trip();
    test_loot_table_preserved();

    std::printf("\n=======================================\n");
    if (g_failures == 0) {
        std::printf("All checks passed.\n");
    } else {
        std::printf("%d check(s) FAILED.\n", g_failures);
    }
    std::printf("=======================================\n");
    return g_failures == 0 ? 0 : 1;
}
