// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//
//  The driver. DO NOT MODIFY THIS FILE.
//
//  It is compiled twice -- once against impl_inherit/, once against
//  impl_compose/ -- and it must print the same thing both times. If the two
//  transcripts differ, the two designs are not modelling the same domain yet,
//  and the measurement step would be comparing two different programs.
//
//      ./build/course-debug/same_problem_twice_inherit > a.txt
//      ./build/course-debug/same_problem_twice_compose > b.txt
//      diff a.txt b.txt        # must print nothing
// ===========================================================================
#include "spellbook.h"

#include <cstdio>

static void show(const char* label, const Caster& c, const Target& t)
{
    std::printf("    %-14s caster %d/%d hp, %d mana, %d minions (%d left) | "
                "target %d/%d hp, %d str\n",
                label, c.hp, c.max_hp, c.mana, c.minions, c.summon_budget,
                t.hp, t.max_hp, t.strength);
}

static void report(const CastResult& r)
{
    if (!r.cast) {
        std::printf("    => not cast (not enough mana)\n");
        return;
    }
    std::printf("    => cast for %d mana: %d damage, %d healed, "
                "%d summoned, %d strength drained\n",
                r.mana_spent, r.damage_dealt, r.healing_done,
                r.minions_added, r.strength_drained);
}

static void cast_one(int index, Caster& caster, Target& target)
{
    const char* name = spell_name(index);
    std::printf("\n--- %s (cost %d) ---\n",
                name ? name : "(null)", spell_mana_cost(index));
    show("before", caster, target);
    CastResult r = cast_spell(index, caster, target);
    report(r);
    show("after", caster, target);
}

int main()
{
    std::printf("IS 267 Concept Lab 3 - Same Problem, Twice\n");
    std::printf("%d spell(s) registered.\n", spell_count());

    if (spell_count() == 0) {
        std::printf("\nNothing registered yet. That is expected on the "
                    "untouched scaffold:\n"
                    "start with the TODOs in impl_inherit/spells.cpp.\n");
        return 0;
    }

    // One caster, one target, shared across every cast in order, so the
    // transcript shows the state accumulating.
    Caster gorthak{"Gorthak", 30, 40, 20, 1, 0};
    Target ash{"Ash", 50, 50, 8, 2};

    for (int i = 0; i < spell_count(); ++i)
        cast_one(i, gorthak, ash);

    // Deliberately drain the caster and try again: the shared "not enough
    // mana" rule is part of the behavior both designs must reproduce.
    std::printf("\n--- the same spells again, with 1 mana left ---\n");
    gorthak.mana = 1;
    for (int i = 0; i < spell_count(); ++i) {
        std::printf("  %s:\n", spell_name(i));
        report(cast_spell(i, gorthak, ash));
    }
    show("final", gorthak, ash);

    std::printf("\nNow run the tests, then fill in ANALYSIS.md.\n");
}
