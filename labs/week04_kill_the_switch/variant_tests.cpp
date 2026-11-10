// ===========================================================================
//  IS 267 — Concept Lab 4:  Kill the Switch — STEP 4
//
//  The new-variant tests. DO NOT MODIFY THIS FILE.
//
//  These FAIL against the code as shipped, because nothing implements
//  ABIL_SHATTER yet. They pass once you add it. Do not add it until the
//  switch is gone and kill_the_switch_tests is green -- the whole point of
//  Step 4 is what adding an ability costs you AFTER the refactor.
//
//      ./build/course-debug/kill_the_switch_variant_tests
//
//  The rule you are measuring: adding Shatter must cost you ZERO edits to
//  any file that already exists. One new .cpp in impl/ and nothing else.
//  The build globs impl/*.cpp and ABIL_SHATTER is already in the header, so
//  nothing is standing in your way except the design.
// ===========================================================================
#include "test_harness.h"

int main()
{
    std::printf("IS 267 Concept Lab 4 - Step 4, the new variant\n");

    // ---- the metadata -----------------------------------------------------
    std::printf("\n[shatter metadata]\n");
    named(ability_name(ABIL_SHATTER), "Shatter",
          "ABIL_SHATTER is named \"Shatter\"");
    eq(ability_mana_cost(ABIL_SHATTER), 7, "Shatter costs 7 mana");
    check(ability_is_hostile(ABIL_SHATTER), "Shatter is hostile");

    eq(ability_count(), 9, "the roster now holds 9 abilities");
    bool on_roster = false;
    for (int i = 0; i < ability_count(); ++i)
        if (ability_tag_at(i) == ABIL_SHATTER) on_roster = true;
    check(on_roster, "Shatter appears on the roster");

    // ---- the effect -------------------------------------------------------
    std::printf("\n[shatter effect, seed 1337]\n");
    {
        Scene s;
        lab_seed(SEED);
        const int amount =
            resolve_ability(ABIL_SHATTER, *s.caster, s.hero, s.wave, 0);

        // strength 7 + the first d6 from seed 1337, which is 6, and no armor
        // reduction at all: 13 damage against a hero wearing 3 armor.
        eq(amount, 13, "Shatter ignores armor entirely");
        eq(s.hero.hp, 47, "Shatter removes the full raw total");
        eq(s.hero.armor, 2, "Shatter strips 1 armor");
        eq(s.caster->mana, 13, "Shatter costs 7 mana");
        eq(static_cast<int>(lab_rand_calls()), 1, "Shatter draws exactly once");
        eq(s.caster->hp, 22, "Shatter leaves the caster's HP alone");
        eq(static_cast<int>(s.wave.size()), 3, "Shatter does not touch the wave");
    }
    {   // the armor floor
        Scene s; s.hero.armor = 0; lab_seed(SEED);
        const int amount =
            resolve_ability(ABIL_SHATTER, *s.caster, s.hero, s.wave, 0);
        eq(amount, 13, "an unarmored hero takes the same raw total");
        eq(s.hero.armor, 0, "Shatter never pushes armor below 0");
        eq(s.hero.hp, 47, "an unarmored hero loses 13 HP");
    }
    {   // hero HP floors at 0
        Scene s; s.hero.hp = 4; lab_seed(SEED);
        const int amount =
            resolve_ability(ABIL_SHATTER, *s.caster, s.hero, s.wave, 0);
        eq(amount, 13, "overkill still reports the full total");
        eq(s.hero.hp, 0, "hero HP floors at 0, never negative");
    }
    {   // the shared fizzle rule applies to the new ability too, for free
        Scene s; s.caster->mana = 6; lab_seed(SEED);
        eq(resolve_ability(ABIL_SHATTER, *s.caster, s.hero, s.wave, 0), 0,
           "Shatter fizzles on 6 mana");
        eq(s.caster->mana, 6, "a fizzled Shatter spends nothing");
        eq(s.hero.hp, 60, "a fizzled Shatter does nothing to the hero");
        eq(s.hero.armor, 3, "a fizzled Shatter strips no armor");
        eq(static_cast<int>(lab_rand_calls()), 0, "a fizzled Shatter draws nothing");
    }

    return summary();
}
