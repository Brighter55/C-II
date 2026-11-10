// ===========================================================================
//  IS 267 — Concept Lab 4:  Kill the Switch
//
//  The supplied behavior tests. DO NOT MODIFY THIS FILE.
//
//  These PASS against the code as shipped. Run them before you change
//  anything -- that is the baseline. They must still pass, unchanged, when
//  the switch is gone. That is the entire claim the lab asks you to make,
//  and it is only a claim if the tests were green first.
//
//      ./build/course-debug/kill_the_switch_tests
//
//  The numbers below were recorded from the supplied implementation at seed
//  1337. They are not "the right answers" in any deeper sense -- they are
//  what this program does, which is what you are not allowed to change.
//
//  Draw counts are checked as well as damage. Two implementations can agree
//  on the first call and diverge forever after because one of them drew a
//  random number the other did not.
// ===========================================================================
#include "test_harness.h"

// =========================================================================
//  TEST 1 -- the metadata
//
//  Names, costs and hostility, plus the roster. Asks for "at least 8"
//  abilities so that adding Shatter in Step 4 does not break these tests.
// =========================================================================
static void test_metadata()
{
    std::printf("\n[metadata]\n");

    struct Row { int tag; const char* name; int cost; bool hostile; };
    const Row rows[] = {
        {ABIL_NONE,   "Flail",  0, true },
        {ABIL_CLEAVE, "Cleave", 3, true },
        {ABIL_BITE,   "Bite",   2, true },
        {ABIL_MEND,   "Mend",   4, false},
        {ABIL_HEX,    "Hex",    5, true },
        {ABIL_SUMMON, "Summon", 8, false},
        {ABIL_GUARD,  "Guard",  2, false},
        {ABIL_RALLY,  "Rally",  6, false},
    };

    for (const Row& r : rows) {
        char buf[96];
        std::snprintf(buf, sizeof buf, "tag %d is named \"%s\"", r.tag, r.name);
        named(ability_name(r.tag), r.name, buf);
        std::snprintf(buf, sizeof buf, "%s costs %d mana", r.name, r.cost);
        eq(ability_mana_cost(r.tag), r.cost, buf);
        std::snprintf(buf, sizeof buf, "%s hostility is %s",
                      r.name, r.hostile ? "true" : "false");
        check(ability_is_hostile(r.tag) == r.hostile, buf);
    }

    named(ability_name(99), "???", "an unknown tag is named \"???\"");
    eq(ability_mana_cost(99), 0,  "an unknown tag costs 0 mana");
    check(!ability_is_hostile(99), "an unknown tag is not hostile");

    check(ability_count() >= 8, "the roster holds at least 8 abilities");
    eq(ability_tag_at(-1), -1,  "ability_tag_at(-1) is -1");
    eq(ability_tag_at(ability_count()), -1, "one past the end is -1");

    bool ascending = true;
    for (int i = 1; i < ability_count(); ++i)
        if (ability_tag_at(i) <= ability_tag_at(i - 1)) ascending = false;
    check(ascending, "the roster is in ascending tag order");

    for (const Row& r : rows) {
        bool found = false;
        for (int i = 0; i < ability_count(); ++i)
            if (ability_tag_at(i) == r.tag) found = true;
        char buf[96];
        std::snprintf(buf, sizeof buf, "%s appears on the roster", r.name);
        check(found, buf);
    }
}

// =========================================================================
//  TEST 2 -- the golden table
//
//  Every ability, resolved once from seed 1337 against an identical scene.
//  Recorded from the supplied implementation. Every column must match.
// =========================================================================
static void test_golden_table()
{
    std::printf("\n[golden table, seed 1337]\n");

    struct Row {
        int tag, amount, draws;
        int c_hp, c_mana, c_armor, c_budget;
        int h_hp, h_str, h_armor;
        int wave_size;
    };
    const Row rows[] = {
        //                   amt  dr  chp cmana carm cbud  hhp hstr harm  wave
        {ABIL_NONE,           10,  1,  22,  20,   2,   1,   50,   9,   3,    3},
        {ABIL_CLEAVE,         15,  1,  22,  17,   2,   1,   45,   9,   3,    3},
        {ABIL_BITE,           12,  2,  22,  18,   2,   1,   48,   9,   3,    3},
        {ABIL_MEND,            5,  2,  27,  16,   2,   1,   60,   9,   3,    3},
        {ABIL_HEX,             4,  1,  22,  15,   2,   1,   60,   5,   3,    3},
        {ABIL_SUMMON,          1,  0,  22,  12,   2,   0,   60,   9,   3,    4},
        {ABIL_GUARD,           2,  0,  22,  18,   4,   1,   60,   9,   3,    3},
        {ABIL_RALLY,           1,  0,  22,  14,   2,   1,   60,   9,   3,    3},
        {99,                  10,  1,  22,  20,   2,   1,   50,   9,   3,    3},
    };

    for (const Row& r : rows) {
        Scene s;
        lab_seed(SEED);
        const int amount = resolve_ability(r.tag, *s.caster, s.hero, s.wave, 0);

        std::printf("  -- %s (tag %d)\n", ability_name(r.tag), r.tag);
        eq(amount,                           r.amount,    "return value");
        eq(static_cast<int>(lab_rand_calls()), r.draws,   "RNG draws");
        eq(s.caster->hp,                     r.c_hp,      "caster HP");
        eq(s.caster->mana,                   r.c_mana,    "caster mana");
        eq(s.caster->armor,                  r.c_armor,   "caster armor");
        eq(s.caster->summon_budget,          r.c_budget,  "caster summon budget");
        eq(s.hero.hp,                        r.h_hp,      "hero HP");
        eq(s.hero.strength,                  r.h_str,     "hero strength");
        eq(s.hero.armor,                     r.h_armor,   "hero armor");
        eq(static_cast<int>(s.wave.size()),  r.wave_size, "wave size");
    }
}

// =========================================================================
//  TEST 3 -- the cases that differ subtly
//
//  These are the ones a rewrite loses. Each is a branch or a clamp that is
//  easy to drop when you are busy congratulating yourself on deleting a
//  switch.
// =========================================================================
static void test_subtle_cases()
{
    std::printf("\n[subtle cases]\n");

    {   // Guard's cap at 8, approached from below
        Scene s; s.caster->armor = 7; lab_seed(SEED);
        eq(resolve_ability(ABIL_GUARD, *s.caster, s.hero, s.wave, 0), 1,
           "Guard from armor 7 gains only 1");
        eq(s.caster->armor, 8, "Guard clamps armor at 8");
    }
    {   // Guard already at the cap
        Scene s; s.caster->armor = 8; lab_seed(SEED);
        eq(resolve_ability(ABIL_GUARD, *s.caster, s.hero, s.wave, 0), 0,
           "Guard at the cap gains 0");
        eq(s.caster->armor, 8, "Guard at the cap leaves armor at 8");
        eq(s.caster->mana, 18, "Guard still costs 2 mana when it gains nothing");
    }
    {   // Mend on a full-HP caster still rolls
        Scene s; s.caster->hp = s.caster->max_hp; lab_seed(SEED);
        eq(resolve_ability(ABIL_MEND, *s.caster, s.hero, s.wave, 0), 0,
           "Mend on a full caster heals 0");
        eq(s.caster->hp, 30, "a full caster stays at max_hp");
        eq(static_cast<int>(lab_rand_calls()), 2,
           "Mend rolls 2d4 even when it cannot heal");
        eq(s.caster->mana, 16, "Mend costs its mana even when it heals 0");
    }
    {   // Mend with exactly 1 HP of room reports 1, not the roll
        Scene s; s.caster->hp = s.caster->max_hp - 1; lab_seed(SEED);
        eq(resolve_ability(ABIL_MEND, *s.caster, s.hero, s.wave, 0), 1,
           "Mend reports the HP actually added, not the dice roll");
        eq(s.caster->hp, 30, "Mend never overshoots max_hp");
    }
    {   // Hex against a hero already at strength 1
        Scene s; s.hero.strength = 1; lab_seed(SEED);
        eq(resolve_ability(ABIL_HEX, *s.caster, s.hero, s.wave, 0), 0,
           "Hex on a strength-1 hero drains 0");
        eq(s.hero.strength, 1, "Hex never pushes strength below 1");
        eq(static_cast<int>(lab_rand_calls()), 1, "Hex rolls anyway");
    }
    {   // Hex clipped by the floor reports the clipped amount
        Scene s; s.hero.strength = 2; lab_seed(SEED);
        eq(resolve_ability(ABIL_HEX, *s.caster, s.hero, s.wave, 0), 1,
           "Hex reports the strength actually drained");
        eq(s.hero.strength, 1, "Hex stops at 1");
    }
    {   // Cleave's follow-up has its own floor of 1 and no second draw
        Scene s; s.hero.armor = 40; lab_seed(SEED);
        eq(resolve_ability(ABIL_CLEAVE, *s.caster, s.hero, s.wave, 0), 2,
           "Cleave through heavy armor is 1 + a follow-up floored at 1");
        eq(s.hero.hp, 58, "Cleave's follow-up ignores armor");
        eq(static_cast<int>(lab_rand_calls()), 1,
           "Cleave draws once, not twice");
    }
    {   // Summon's fallback: no whelp, a real attack, same mana
        Scene s; s.caster->summon_budget = 0; lab_seed(SEED);
        eq(resolve_ability(ABIL_SUMMON, *s.caster, s.hero, s.wave, 0), 10,
           "Summon with no budget falls back to an attack");
        eq(static_cast<int>(s.wave.size()), 3, "the fallback spawns nothing");
        eq(s.caster->summon_budget, 0, "the budget does not go negative");
        eq(s.hero.hp, 50, "the fallback deals real damage");
        eq(s.caster->mana, 12, "the fallback still costs the full 8 mana");
        eq(static_cast<int>(lab_rand_calls()), 1, "the fallback draws once");
    }
    {   // Rally skips self and skips the dead
        Scene s; lab_seed(SEED);
        eq(resolve_ability(ABIL_RALLY, *s.caster, s.hero, s.wave, 0), 1,
           "Rally counts only the monsters it actually rallied");
        eq(s.wave[0]->strength, 7, "Rally does not rally the caster");
        eq(s.wave[1]->strength, 4, "Rally rallies the living ally");
        eq(s.wave[2]->strength, 3, "Rally does not rally the dead");
        eq(static_cast<int>(lab_rand_calls()), 0, "Rally draws nothing");
    }
    {   // The Whelp a Summon spawns has fixed stats
        Scene s; lab_seed(SEED);
        resolve_ability(ABIL_SUMMON, *s.caster, s.hero, s.wave, 0);
        check(s.wave.size() == 4, "Summon appends to the wave");
        if (s.wave.size() == 4) {
            named(s.wave[3]->name, "Whelp", "the summon is a Whelp");
            eq(s.wave[3]->hp,       6, "the Whelp has 6 HP");
            eq(s.wave[3]->strength, 2, "the Whelp has 2 strength");
            eq(s.wave[3]->armor,    0, "the Whelp has 0 armor");
            check(!s.wave[3]->is_dead, "the Whelp arrives alive");
        }
    }
}

// =========================================================================
//  TEST 4 -- the shared fizzle rule
//
//  One mana. Everything but Flail, which is free, must do absolutely
//  nothing: no state change, no RNG draw, return 0.
// =========================================================================
static void test_fizzle()
{
    std::printf("\n[fizzle on insufficient mana]\n");

    for (int i = 0; i < ability_count(); ++i) {
        const int tag = ability_tag_at(i);
        if (ability_mana_cost(tag) <= 1) continue;   // Flail is free

        Scene s; s.caster->mana = 1; lab_seed(SEED);
        const int amount = resolve_ability(tag, *s.caster, s.hero, s.wave, 0);

        std::printf("  -- %s with 1 mana\n", ability_name(tag));
        eq(amount, 0, "returns 0");
        eq(s.caster->mana, 1, "mana is untouched");
        eq(static_cast<int>(lab_rand_calls()), 0, "no RNG draw happens");
        eq(s.caster->hp, 22, "caster HP is untouched");
        eq(s.caster->armor, 2, "caster armor is untouched");
        eq(s.caster->summon_budget, 1, "summon budget is untouched");
        eq(s.hero.hp, 60, "hero HP is untouched");
        eq(s.hero.strength, 9, "hero strength is untouched");
        eq(static_cast<int>(s.wave.size()), 3, "the wave is untouched");
        eq(s.wave[1]->strength, 3, "nobody gets rallied");
    }

    {   // Flail costs nothing, so one mana is plenty
        Scene s; s.caster->mana = 1; lab_seed(SEED);
        eq(resolve_ability(ABIL_NONE, *s.caster, s.hero, s.wave, 0), 10,
           "Flail is free and resolves on 1 mana");
        eq(s.caster->mana, 1, "a free ability spends nothing");
    }
}

int main()
{
    std::printf("IS 267 Concept Lab 4 - supplied behavior tests\n");

    test_metadata();
    test_golden_table();
    test_subtle_cases();
    test_fizzle();

    return summary();
}
