// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//
//  THE CONTRACT.  DO NOT MODIFY THIS FILE.
//
//  This header is the boundary between your two designs and the tests. The
//  tests include it, the driver includes it, and both of your implementations
//  must satisfy it. Everything behind these five functions is yours to shape
//  however you like -- that is the whole lab.
//
//  You will write two implementations of this header:
//
//      impl_inherit/   an inheritance hierarchy
//      impl_compose/   composition, no inheritance at all
//
//  Same header, same tests, same observable behavior. Then you measure both
//  and recommend one.
// ===========================================================================
#ifndef DD_LAB03_SPELLBOOK_H
#define DD_LAB03_SPELLBOOK_H

// ---------------------------------------------------------------- the domain
//
// A monster casts an ability at a hero. Three abilities ship: Fireball, Mend,
// Summon. This is the same domain Build 4 rebuilds, deliberately: an ability
// has a name, a mana cost, and an effect on a target.

struct Caster {
    const char* name;          // points at a literal; nothing owns anything
    int hp;
    int max_hp;
    int mana;
    int summon_budget;         // how many more minions this caster may raise
    int minions;               // how many it has raised so far
};

struct Target {
    const char* name;
    int hp;
    int max_hp;
    int strength;
    int ward;                  // flat damage reduction
};

// What a cast reports back. Every field is the ACTUAL amount applied, not the
// amount attempted: a Mend that only had 5 HP of room reports healing_done 5.
struct CastResult {
    bool cast;                 // false means the caster could not pay the mana
    int  mana_spent;
    int  damage_dealt;
    int  healing_done;
    int  minions_added;
    int  strength_drained;
};

// ------------------------------------------------------------ the behavior
//
// This is the specification. Both implementations must produce exactly this.
// There is no randomness anywhere in this lab; every number below is exact.
//
// THE RULE THAT APPLIES TO EVERY SPELL
//   If caster.mana < the spell's mana cost, nothing happens at all: no field
//   of Caster or Target changes, and the result is all zeroes with
//   cast == false. Otherwise caster.mana loses exactly the cost, once, and
//   mana_spent is that cost.
//
//   Where this rule lives is one of the things you are comparing. In an
//   inheritance design it has an obvious home. In a composition design it has
//   a different obvious home. Note which you found more natural.
//
// "Fireball"  cost 6   hostile
//   Raw damage 9. Dealt = max(1, 9 - target.ward)  -- a fully warded target
//   still takes 1. target.hp drops by dealt and floors at 0 (never negative).
//   damage_dealt = dealt.
//
// "Mend"      cost 4   not hostile
//   Heals the CASTER by 7, capped so caster.hp never exceeds caster.max_hp.
//   healing_done = the amount actually added, which may be less than 7.
//
// "Summon"    cost 8   not hostile
//   If caster.summon_budget > 0: budget drops by 1, minions rises by 1,
//   minions_added = 1, and no damage is dealt.
//   Otherwise it falls back to a weak poke: dealt = max(1, 3 - target.ward),
//   applied to target.hp exactly as Fireball applies its damage,
//   damage_dealt = dealt, minions_added = 0. The mana is spent either way.
//
// In Step 4 you add a fourth spell:
//
// "Hex"       cost 3   hostile
//   target.strength drops by 2 but never below 1.
//   strength_drained = the amount actually removed, which may be less than 2.

// ------------------------------------------------- what you must implement
//
// Implement these four in impl_inherit/ and again in impl_compose/.
//
// Spells are addressed by index, 0 .. spell_count()-1. The order is yours.

int         spell_count();
const char* spell_name(int index);
int         spell_mana_cost(int index);
CastResult  cast_spell(int index, Caster& caster, Target& target);

// Supplied for you in lab_support.cpp, written once in terms of the four
// above, so you do not implement it twice. Returns -1 if no spell has that
// name. The tests find spells this way, so your registration order is free
// and adding a fourth spell does not break the supplied tests.
int find_spell(const char* name);

#endif  // DD_LAB03_SPELLBOOK_H
