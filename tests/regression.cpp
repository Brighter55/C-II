/* ===========================================================================
   regression.cpp -- the three regression tests for Build 1.

   These tests are the specification. All three FAIL on the code as shipped.
   All three must PASS before you submit Build 1, and they must keep passing
   in every later build.

   Run them with:
       ctest --preset course-debug --output-on-failure

   Each test is a separate ctest entry so you can work on one at a time:
       ctest --preset course-debug -R encounter_resolves --output-on-failure

   You should not need to change this file. If you think a test is wrong,
   say so in TRIAGE.md and bring it to the discussion -- do not quietly
   edit the test until it agrees with your code.
   =========================================================================== */

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

/* The engine's own header. Everything below is written in terms of the
   aliases it declares, so these tests keep compiling after you change the
   ownership model in Build 1. Do not re-declare engine functions here. */

#include "dungeon_delve.h"

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

static void note(const char* fmt, int a, int b)
{
    std::printf("        (%s: %d vs %d)\n", fmt, a, b);
}

/* =========================================================================
   TEST 1 -- encounter_resolves

   Armor should reduce incoming damage by exactly its own value, once.

   Two targets are identical except for armor: one has 0, one has 3. Given
   the same RNG seed, the same attacker, and the same raw roll, the damage
   that lands on them must differ by exactly 3.

   If armor is being applied more than once, the gap will be wider than the
   armor value. That is the defect.
   ========================================================================= */

static int test_encounter_resolves()
{
    std::printf("\n[encounter_resolves]\n");

    HeroPtr ash = make_hero("Ash", 100, 10, 3);

    const unsigned SEED = 90210u;
    const int ARMOR = 3;

    const int START_HP = 500;

    dd_seed(SEED);
    MonsterPtr soft = make_monster("Target A", "dummy", START_HP, 0, 0, 0, ABIL_NONE);
    int reported_unarmored = resolve_attack_on_monster(*ash, *raw(soft));
    int removed_unarmored  = START_HP - soft->hp;

    dd_seed(SEED);
    MonsterPtr armored = make_monster("Target B", "dummy", START_HP, 0, ARMOR, 0, ABIL_NONE);
    int reported_armored = resolve_attack_on_monster(*ash, *raw(armored));
    int removed_armored  = START_HP - armored->hp;

    std::printf("        unarmored:  reported %d, HP removed %d\n",
                reported_unarmored, removed_unarmored);
    std::printf("        armored(%d): reported %d, HP removed %d\n",
                ARMOR, reported_armored, removed_armored);

    /* Ground truth is the HP that actually left the monster, not the number
       the function chose to hand back. Measure the real effect. */
    int gap = removed_unarmored - removed_armored;

    check(gap == ARMOR,
          "armor reduces the HP actually lost by exactly its own value");
    if (gap != ARMOR) note("expected gap vs actual gap", ARMOR, gap);

    /* And the number reported to the caller must describe what happened. */
    check(removed_armored == reported_armored,
          "HP removed equals the damage the attack reported");
    if (removed_armored != reported_armored)
        note("reported vs removed", reported_armored, removed_armored);

    return g_failures;
}

/* =========================================================================
   TEST 2 -- loot_table_intact

   Every value roll_loot() hands back must be an entry that is actually in
   the loot table. Nothing else is a legal drop.

   The pointer is compared rather than the string, deliberately: if the
   engine hands back something from outside the table, dereferencing it
   would be undefined behavior and the test would crash before it could
   tell you anything useful.
   ========================================================================= */

static int test_loot_table_intact()
{
    std::printf("\n[loot_table_intact]\n");

    std::vector<const char*> valid;
    for (int i = 0; i < loot_count(); i++)
        valid.push_back(loot_at(i));

    std::printf("        loot table has %d entries\n", (int)valid.size());

    dd_seed(4242u);

    const int ROLLS = 2000;
    int off_table = 0;

    for (int r = 0; r < ROLLS; r++) {
        const char* drop = roll_loot();

        bool found = false;
        for (size_t i = 0; i < valid.size(); i++) {
            if (valid[i] == drop) { found = true; break; }
        }
        if (!found) off_table++;
    }

    std::printf("        %d rolls, %d landed outside the table\n",
                ROLLS, off_table);

    check(off_table == 0, "every drop came from inside the loot table");
    if (off_table != 0) note("expected off-table vs actual", 0, off_table);

    return g_failures;
}

/* =========================================================================
   TEST 3 -- roster_survives_reallocation

   A monster that summons adds to the wave while the wave is being iterated.
   Every monster recorded as having acted must still be a monster that is
   actually in the wave afterwards.

   The wave here holds four monsters so that it is exactly full when the
   summon happens, and the summoner acts first so the round continues after
   the wave grows.
   ========================================================================= */

static int test_roster_survives_reallocation()
{
    std::printf("\n[roster_survives_reallocation]\n");

    HeroPtr ash = make_hero("Ash", 100000, 10, 3);

    Wave wave;
    wave.push_back(make_monster("Brood Mother", "brood", 22, 5, 2, 18, ABIL_SUMMON));
    wave.push_back(make_monster("Goblin A", "goblin", 10, 3, 0, 5, ABIL_NONE));
    wave.push_back(make_monster("Goblin B", "goblin", 10, 3, 0, 5, ABIL_NONE));
    wave.push_back(make_monster("Goblin C", "goblin", 10, 3, 0, 5, ABIL_NONE));

    std::printf("        wave size %d, capacity %d before the round\n",
                (int)wave.size(), (int)wave.capacity());

    dd_seed(7u);

    std::vector<std::string> acted;
    run_monster_round(wave, *ash, 0, &acted);

    std::printf("        wave size %d after the round, %d monsters acted\n",
                (int)wave.size(), (int)acted.size());

    /* Collect the names that are genuinely in the wave now. */
    std::vector<std::string> real_names;
    for (size_t i = 0; i < wave.size(); i++)
        real_names.push_back(std::string(wave[i]->name));

    int unrecognized = 0;
    for (size_t a = 0; a < acted.size(); a++) {
        bool found = false;
        for (size_t r = 0; r < real_names.size(); r++) {
            if (acted[a] == real_names[r]) { found = true; break; }
        }
        if (!found) {
            unrecognized++;
            std::printf("        acted but not in the wave: \"%s\"\n",
                        acted[a].c_str());
        }
    }

    check(wave.size() == 5, "the summon added exactly one monster to the wave");
    if (wave.size() != 5) note("expected size vs actual", 5, (int)wave.size());

    check(unrecognized == 0,
          "every monster that acted is really in the wave");
    if (unrecognized != 0) note("expected strangers vs actual", 0, unrecognized);

    check(acted.size() >= 4,
          "all four original monsters got their turn");
    if (acted.size() < 4) note("expected at least vs actual", 4, (int)acted.size());

    return g_failures;
}

/* ---- entry point -------------------------------------------------------- */

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::printf("usage: regression <test-name>\n");
        return 2;
    }

    if (std::strcmp(argv[1], "encounter_resolves") == 0) {
        test_encounter_resolves();
    } else if (std::strcmp(argv[1], "loot_table_intact") == 0) {
        test_loot_table_intact();
    } else if (std::strcmp(argv[1], "roster_survives_reallocation") == 0) {
        test_roster_survives_reallocation();
    } else {
        std::printf("unknown test: %s\n", argv[1]);
        return 2;
    }

    std::printf("\n%s\n", g_failures == 0 ? "PASSED" : "FAILED");
    return g_failures == 0 ? 0 : 1;
}
