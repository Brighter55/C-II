// ===========================================================================
//  IS 267 — Concept Lab 4:  Kill the Switch
//
//  The driver. DO NOT MODIFY THIS FILE.
//
//  It walks the roster, resolves each ability from a fixed seed against a
//  fresh identical scenario, and prints what happened and how many random
//  draws it cost. Capture its output BEFORE you start:
//
//      ./build/course-debug/kill_the_switch > before.txt
//
//  and diff against it when you are done. Together with the tests, that is
//  your proof that behavior did not move.
// ===========================================================================
#include "kill_the_switch.h"

#include <cstdio>

namespace {

const unsigned FIXED_SEED = 1337u;

// A fresh, identical encounter for every ability, so one ability's effects
// never leak into the next one's numbers.
struct Scene {
    Wave  wave;
    Hero  hero{"Ash", 60, 60, 9, 3};
    Monster* caster = nullptr;

    Scene()
    {
        caster = spawn_into_wave(wave, "Gorthak", 30, 7, 2);
        caster->mana          = 20;
        caster->summon_budget = 1;
        caster->hp            = 22;      // room for Mend to heal into

        spawn_into_wave(wave, "Snarl", 10, 3, 0);
        Monster* corpse = spawn_into_wave(wave, "Husk", 10, 3, 0);
        corpse->is_dead = true;          // Rally must skip this one
    }
};

void run_one(int tag)
{
    Scene s;
    lab_seed(FIXED_SEED);

    std::printf("\n--- %s (tag %d, cost %d, %s) ---\n",
                ability_name(tag), tag, ability_mana_cost(tag),
                ability_is_hostile(tag) ? "hostile" : "not hostile");

    const int amount = resolve_ability(tag, *s.caster, s.hero, s.wave, 1);

    std::printf("  returned %d | rng draws %u\n", amount, lab_rand_calls());
    std::printf("  caster: %d/%d hp, %d mana, %d armor, %d budget\n",
                s.caster->hp, s.caster->max_hp, s.caster->mana,
                s.caster->armor, s.caster->summon_budget);
    std::printf("  hero:   %d/%d hp, %d str, %d armor\n",
                s.hero.hp, s.hero.max_hp, s.hero.strength, s.hero.armor);
    std::printf("  wave:   %zu monsters, %d living\n",
                s.wave.size(), living_in_wave(s.wave));
}

}  // namespace

int main()
{
    std::printf("IS 267 Concept Lab 4 - Kill the Switch\n");
    std::printf("%d abilities on the roster, seed %u, one fresh scene each.\n",
                ability_count(), FIXED_SEED);

    for (int i = 0; i < ability_count(); ++i)
        run_one(ability_tag_at(i));

    // A tag nobody implemented. The supplied code falls into the switch's
    // `default`; whatever you replace it with has to answer the same way.
    std::printf("\n--- an unknown tag (99) ---\n");
    run_one(99);

    // The shared fizzle rule, with the mana drained.
    std::printf("\n--- the whole roster again, with 1 mana ---\n");
    for (int i = 0; i < ability_count(); ++i) {
        Scene s;
        lab_seed(FIXED_SEED);
        s.caster->mana = 1;
        const int tag = ability_tag_at(i);
        const int amount = resolve_ability(tag, *s.caster, s.hero, s.wave, 1);
        std::printf("  %-8s returned %d | mana left %d | rng draws %u\n",
                    ability_name(tag), amount, s.caster->mana,
                    lab_rand_calls());
    }

    std::printf("\nNow run the tests, then fill in ANALYSIS.md.\n");
}
