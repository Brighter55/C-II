// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//
//  The supplied tests. DO NOT MODIFY THIS FILE.
//
//  One test file, linked twice: once against impl_inherit/, once against
//  impl_compose/. That is the point of the lab -- the tests cannot tell which
//  design they are talking to, so any difference you measure between the two
//  is a difference in the code you wrote, not in what the code does.
//
//      ./build/course-debug/same_problem_twice_inherit_tests
//      ./build/course-debug/same_problem_twice_compose_tests
//
//  Both must print "ALL TESTS PASSED". On the untouched scaffold both fail,
//  which is how you know the tests are actually checking something.
//
//  If you think a test is wrong, say so in ANALYSIS.md and bring it to the
//  discussion. Do not quietly edit the test until it agrees with your code.
// ===========================================================================
#include "spellbook.h"

#include <cstdio>

// ---- a very small test harness ------------------------------------------

static int g_failures = 0;
static int g_checks   = 0;

static void check(bool condition, const char* what)
{
    ++g_checks;
    if (condition) {
        std::printf("  ok    %s\n", what);
    } else {
        std::printf("  FAIL  %s\n", what);
        ++g_failures;
    }
}

static void check_eq(int actual, int expected, const char* what)
{
    ++g_checks;
    if (actual == expected) {
        std::printf("  ok    %s\n", what);
    } else {
        std::printf("  FAIL  %s  (expected %d, got %d)\n", what, expected, actual);
        ++g_failures;
    }
}

// Fresh combatants for every test, so no test depends on another's leftovers.
static Caster fresh_caster(int hp, int max_hp, int mana, int budget)
{
    return Caster{"Gorthak", hp, max_hp, mana, budget, 0};
}

static Target fresh_target(int hp, int ward)
{
    return Target{"Ash", hp, 50, 8, ward};
}

// =========================================================================
//  TEST 1 -- the spellbook itself
//
//  Three spells ship. Later you add a fourth, so this asks for "at least
//  three" rather than "exactly three": adding Hex in Step 4 must not break
//  the supplied tests.
// =========================================================================
static void test_registration()
{
    std::printf("\n[registration]\n");

    check(spell_count() >= 3, "spell_count() reports at least 3 spells");

    const int fire   = find_spell("Fireball");
    const int mend   = find_spell("Mend");
    const int summon = find_spell("Summon");

    check(fire   >= 0, "a spell named exactly \"Fireball\" is registered");
    check(mend   >= 0, "a spell named exactly \"Mend\" is registered");
    check(summon >= 0, "a spell named exactly \"Summon\" is registered");

    if (fire   >= 0) check_eq(spell_mana_cost(fire),   6, "Fireball costs 6 mana");
    if (mend   >= 0) check_eq(spell_mana_cost(mend),   4, "Mend costs 4 mana");
    if (summon >= 0) check_eq(spell_mana_cost(summon), 8, "Summon costs 8 mana");
}

// =========================================================================
//  TEST 2 -- Fireball
//
//  Damage is warded, floors at 1, and the target's HP floors at 0. The
//  reported damage is the computed damage, not the HP actually removed --
//  a target with 3 HP left still takes 9 damage on the books.
// =========================================================================
static void test_fireball()
{
    std::printf("\n[fireball]\n");

    const int fire = find_spell("Fireball");
    if (fire < 0) { check(false, "Fireball must be registered before it can be tested"); return; }

    {   // unwarded
        Caster c = fresh_caster(30, 40, 10, 1);
        Target t = fresh_target(50, 0);
        CastResult r = cast_spell(fire, c, t);
        check(r.cast,                     "unwarded Fireball is cast");
        check_eq(r.mana_spent,        6,  "unwarded Fireball spends 6 mana");
        check_eq(c.mana,              4,  "caster mana drops by exactly the cost");
        check_eq(r.damage_dealt,      9,  "unwarded Fireball deals 9");
        check_eq(t.hp,               41,  "unwarded Fireball removes 9 HP");
        check_eq(r.healing_done,      0,  "Fireball heals nothing");
        check_eq(r.minions_added,     0,  "Fireball summons nothing");
        check_eq(r.strength_drained,  0,  "Fireball drains no strength");
        check_eq(c.hp,               30,  "Fireball leaves the caster's HP alone");
    }
    {   // warded
        Caster c = fresh_caster(30, 40, 10, 1);
        Target t = fresh_target(50, 4);
        CastResult r = cast_spell(fire, c, t);
        check_eq(r.damage_dealt,      5,  "ward 4 reduces Fireball to 5");
        check_eq(t.hp,               45,  "ward 4 leaves the target at 45");
    }
    {   // ward larger than the damage -- the floor of 1
        Caster c = fresh_caster(30, 40, 10, 1);
        Target t = fresh_target(50, 20);
        CastResult r = cast_spell(fire, c, t);
        check_eq(r.damage_dealt,      1,  "a fully warded target still takes 1");
        check_eq(t.hp,               49,  "the floor of 1 reaches the target's HP");
    }
    {   // HP floors at zero, damage does not
        Caster c = fresh_caster(30, 40, 10, 1);
        Target t = fresh_target(3, 0);
        CastResult r = cast_spell(fire, c, t);
        check_eq(r.damage_dealt,      9,  "overkill still reports the full 9");
        check_eq(t.hp,                0,  "target HP floors at 0, never negative");
    }
}

// =========================================================================
//  TEST 3 -- Mend
//
//  Heals the caster, not the target, and reports what it actually added.
// =========================================================================
static void test_mend()
{
    std::printf("\n[mend]\n");

    const int mend = find_spell("Mend");
    if (mend < 0) { check(false, "Mend must be registered before it can be tested"); return; }

    {   // room to spare
        Caster c = fresh_caster(50, 100, 10, 1);
        Target t = fresh_target(50, 0);
        CastResult r = cast_spell(mend, c, t);
        check(r.cast,                     "Mend is cast with mana to spare");
        check_eq(r.healing_done,      7,  "Mend with room heals the full 7");
        check_eq(c.hp,               57,  "Mend raises caster HP by 7");
        check_eq(c.mana,              6,  "Mend spends 4 mana");
        check_eq(t.hp,               50,  "Mend does not touch the target");
        check_eq(r.damage_dealt,      0,  "Mend deals no damage");
    }
    {   // capped by max_hp
        Caster c = fresh_caster(95, 100, 10, 1);
        Target t = fresh_target(50, 0);
        CastResult r = cast_spell(mend, c, t);
        check_eq(r.healing_done,      5,  "Mend reports 5 when only 5 HP of room exists");
        check_eq(c.hp,              100,  "Mend never pushes HP above max_hp");
    }
    {   // already full
        Caster c = fresh_caster(100, 100, 10, 1);
        Target t = fresh_target(50, 0);
        CastResult r = cast_spell(mend, c, t);
        check(r.cast,                     "Mend on a full-HP caster is still cast");
        check_eq(r.healing_done,      0,  "Mend on a full-HP caster heals 0");
        check_eq(c.hp,              100,  "a full-HP caster stays at max_hp");
        check_eq(c.mana,              6,  "Mend costs its mana even when it heals 0");
    }
}

// =========================================================================
//  TEST 4 -- Summon
//
//  Two branches behind one spell. The fallback is the interesting half: it
//  spends the mana, deals the weak poke, and summons nothing.
// =========================================================================
static void test_summon()
{
    std::printf("\n[summon]\n");

    const int summon = find_spell("Summon");
    if (summon < 0) { check(false, "Summon must be registered before it can be tested"); return; }

    {   // budget available
        Caster c = fresh_caster(30, 40, 10, 2);
        Target t = fresh_target(50, 0);
        CastResult r = cast_spell(summon, c, t);
        check(r.cast,                     "Summon with budget is cast");
        check_eq(r.minions_added,     1,  "Summon with budget raises 1 minion");
        check_eq(c.minions,           1,  "the caster's minion count rises");
        check_eq(c.summon_budget,     1,  "the summon budget drops by 1");
        check_eq(r.damage_dealt,      0,  "Summon with budget deals no damage");
        check_eq(t.hp,               50,  "Summon with budget leaves the target alone");
        check_eq(c.mana,              2,  "Summon spends 8 mana");
    }
    {   // budget exhausted -- the fallback poke
        Caster c = fresh_caster(30, 40, 10, 0);
        Target t = fresh_target(50, 0);
        CastResult r = cast_spell(summon, c, t);
        check(r.cast,                     "Summon without budget is still cast");
        check_eq(r.minions_added,     0,  "Summon without budget raises nothing");
        check_eq(c.minions,           0,  "the caster's minion count does not move");
        check_eq(c.summon_budget,     0,  "the budget does not go negative");
        check_eq(r.damage_dealt,      3,  "the fallback poke deals 3 unwarded");
        check_eq(t.hp,               47,  "the fallback poke removes 3 HP");
        check_eq(c.mana,              2,  "the fallback still spends the full 8 mana");
    }
    {   // the fallback is warded like any other damage
        Caster c = fresh_caster(30, 40, 10, 0);
        Target t = fresh_target(50, 1);
        CastResult r = cast_spell(summon, c, t);
        check_eq(r.damage_dealt,      2,  "ward 1 reduces the fallback poke to 2");
        check_eq(t.hp,               48,  "the warded poke removes 2 HP");
    }
    {   // and it has the same floor of 1
        Caster c = fresh_caster(30, 40, 10, 0);
        Target t = fresh_target(50, 9);
        CastResult r = cast_spell(summon, c, t);
        check_eq(r.damage_dealt,      1,  "the fallback poke also floors at 1");
    }
}

// =========================================================================
//  TEST 5 -- the shared rule
//
//  One rule applies to every spell: if the mana is not there, nothing
//  happens. Not "a reduced effect" -- nothing. Every field of both structs
//  is untouched.
//
//  This test is the one that tells you where the rule lives in each design.
//  If you had to write it three times, that is a finding; write it down.
// =========================================================================
static void test_insufficient_mana()
{
    std::printf("\n[insufficient mana]\n");

    struct Case { const char* name; int mana; };
    const Case cases[] = {
        {"Fireball", 5},   // costs 6
        {"Mend",     3},   // costs 4
        {"Summon",   7},   // costs 8
    };

    for (const Case& k : cases) {
        const int idx = find_spell(k.name);
        if (idx < 0) { check(false, "spell must be registered to test its mana rule"); continue; }

        Caster c = fresh_caster(50, 100, k.mana, 2);
        Target t = fresh_target(50, 0);
        CastResult r = cast_spell(idx, c, t);

        std::printf("  -- %s with %d mana\n", k.name, k.mana);
        check(!r.cast,                        "cast == false when the mana is short");
        check_eq(r.mana_spent,        0,      "no mana is spent");
        check_eq(r.damage_dealt,      0,      "no damage is dealt");
        check_eq(r.healing_done,      0,      "no healing is done");
        check_eq(r.minions_added,     0,      "nothing is summoned");
        check_eq(r.strength_drained,  0,      "no strength is drained");
        check_eq(c.mana,              k.mana, "caster mana is unchanged");
        check_eq(c.hp,                50,     "caster HP is unchanged");
        check_eq(c.summon_budget,     2,      "summon budget is unchanged");
        check_eq(c.minions,           0,      "minion count is unchanged");
        check_eq(t.hp,                50,     "target HP is unchanged");
        check_eq(t.strength,          8,      "target strength is unchanged");
    }
}

// =========================================================================
//  TEST 6 -- mana is spent once per cast, not twice
//
//  A classic bug in both designs: the shared rule deducts the cost and then
//  the specific effect deducts it again.
// =========================================================================
static void test_mana_spent_once()
{
    std::printf("\n[mana accounting]\n");

    const int fire = find_spell("Fireball");
    if (fire < 0) { check(false, "Fireball must be registered"); return; }

    Caster c = fresh_caster(30, 40, 20, 1);
    Target t = fresh_target(500, 0);

    cast_spell(fire, c, t);
    check_eq(c.mana, 14, "one Fireball costs exactly 6");
    cast_spell(fire, c, t);
    check_eq(c.mana,  8, "two Fireballs cost exactly 12");
    check_eq(t.hp,  482, "two Fireballs remove 18 HP");
}

int main()
{
    std::printf("IS 267 Concept Lab 3 - supplied tests\n");

    test_registration();
    test_fireball();
    test_mend();
    test_summon();
    test_insufficient_mana();
    test_mana_spent_once();

    std::printf("\n=======================================\n");
    if (g_failures == 0) {
        std::printf("ALL TESTS PASSED  (%d checks)\n", g_checks);
    } else {
        std::printf("%d of %d checks FAILED\n", g_failures, g_checks);
    }
    std::printf("=======================================\n");
    return g_failures == 0 ? 0 : 1;
}
