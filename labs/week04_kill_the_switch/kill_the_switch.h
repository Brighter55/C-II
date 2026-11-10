// ===========================================================================
//  IS 267 — Concept Lab 4:  Kill the Switch
//
//  THE CONTRACT.  DO NOT MODIFY THIS FILE.
//
//  The tests include this header, the driver includes this header, and
//  whatever you put in impl/ has to satisfy it. Replacing the switch is a
//  change behind these declarations, not to them: that is what makes
//  "behavior is identical" a checkable claim rather than an opinion.
//
//  The ability tags below stay. They are how the engine addresses abilities
//  and how the tests ask for one. Keeping a tag in the public API is not the
//  same thing as dispatching on it -- the lab is about the second.
// ===========================================================================
#ifndef DD_LAB04_KILL_THE_SWITCH_H
#define DD_LAB04_KILL_THE_SWITCH_H

#include <memory>
#include <vector>

// ---------------------------------------------------------------- the domain
//
// Cut down from the engine: a monster with mana and a summon budget, a hero
// with armor, and a wave the monster can add to. Names point at literals;
// nothing here owns a string.

struct Monster {
    const char* name;
    int  hp;
    int  max_hp;
    int  strength;
    int  armor;
    int  mana;
    int  summon_budget;
    bool is_dead;
};

struct Hero {
    const char* name;
    int hp;
    int max_hp;
    int strength;
    int armor;
};

// Build 1 already fixed the ownership model, so a wave owns its monsters.
using Wave = std::vector<std::unique_ptr<Monster>>;

// ------------------------------------------------------------- ability tags
//
// Straight out of the engine, `#define` and all. ABIL_SHATTER is reserved
// for the variant you add in Step 4; nothing implements it yet.

#define ABIL_NONE     0
#define ABIL_CLEAVE   1
#define ABIL_BITE     2
#define ABIL_MEND     3
#define ABIL_HEX      4
#define ABIL_SUMMON   5
#define ABIL_GUARD    6
#define ABIL_RALLY    7
#define ABIL_SHATTER  8   /* Step 4. Not implemented in the supplied code. */

// --------------------------------------------------- supplied in lab_support.cpp
//
// DO NOT reimplement these; use them. They are the parts of the engine the
// abilities lean on, and porting them by hand is busywork, not this lab.

// Deterministic RNG. Seed it and the whole run repeats exactly.
void     lab_seed(unsigned s);
int      lab_rand_range(int lo, int hi);   // inclusive on both ends, one draw
int      roll_dice(int count, int sides);  // `count` draws
unsigned lab_rand_calls();                 // raw draws since the last lab_seed()

// One plain attack: draws a single d6, adds the attacker's strength,
// subtracts the hero's armor, floors the result at 1, applies it to the
// hero's HP (which floors at 0), and returns the damage dealt.
// Exactly ONE RNG draw.
int attack_hero(Monster& self, Hero& target);

// Adds a monster to the wave and returns a non-owning pointer to it.
Monster* spawn_into_wave(Wave& wave, const char* name,
                         int hp, int strength, int armor);

int living_in_wave(const Wave& wave);

// --------------------------------------------------- you implement these in impl/
//
// Six functions. In the code as supplied, five of them are a switch or a
// table over the tag, which is the thing you are removing.
//
// An unknown tag is not an error: ability_name() answers "???", the cost is
// 0, it is not hostile, and resolving it behaves exactly like ABIL_NONE but
// prints the generic line. Keep that, because the tests check it.

const char* ability_name(int tag);
int         ability_mana_cost(int tag);
bool        ability_is_hostile(int tag);

// The abilities that exist, in ASCENDING TAG ORDER. ability_tag_at() returns
// -1 for an out-of-range index.
//
// Ascending order is part of the contract, not a coincidence of how the
// supplied table happens to be written. It is what makes the driver's output
// comparable before and after your refactor: if the roster came out in
// whatever order your translation units happened to initialise in, the
// before/after diff would be noise.
int ability_count();
int ability_tag_at(int index);

// ---------------------------------------------------------------------------
//  resolve_ability()
//
//  THE RULE THAT APPLIES TO EVERY ABILITY
//    If self.mana < ability_mana_cost(tag), the ability fizzles: it returns
//    0, changes nothing anywhere, draws NO random numbers, and prints the
//    fizzle line when verbose. Otherwise self.mana loses exactly the cost,
//    once, before the effect runs.
//
//  WHAT EACH ABILITY DOES, AND HOW MANY RNG DRAWS IT COSTS
//
//    ABIL_NONE    "Flail"   cost 0   hostile       1 draw
//        One attack_hero(). Returns the damage dealt.
//
//    ABIL_CLEAVE  "Cleave"  cost 3   hostile       1 draw
//        One attack_hero() for `first`. Then a follow-up hit of first/2
//        (integer division), floored at 1, applied straight to hero.hp with
//        NO armor reduction and NO second draw. Returns first + follow-up.
//
//    ABIL_BITE    "Bite"    cost 2   hostile       2 draws, always
//        One attack_hero() for `first`. Then ALWAYS one lab_rand_range(1,100)
//        draw. If that roll is > 70, 2 more damage goes straight to hero.hp
//        and the return value is first + 2; otherwise the return value is
//        first. The second draw happens either way -- that is what makes the
//        draw count 2 and not "1 or 2".
//
//    ABIL_MEND    "Mend"    cost 4   not hostile   2 draws
//        roll_dice(2, 4) healing on SELF, capped at self.max_hp. Returns the
//        amount actually added, which may be less than the roll.
//
//    ABIL_HEX     "Hex"     cost 5   hostile       1 draw
//        roll_dice(1, 4) off hero.strength, which floors at 1. Returns the
//        amount actually drained, which may be less than the roll.
//
//    ABIL_SUMMON  "Summon"  cost 8   not hostile   0 or 1 draws
//        If self.summon_budget > 0: spend 1 of it, spawn_into_wave() a
//        "Whelp" with 6 HP, 2 strength, 0 armor, return 1, and draw NOTHING.
//        Otherwise fall back to one attack_hero() and return its damage.
//
//    ABIL_GUARD   "Guard"   cost 2   not hostile   0 draws
//        self.armor += 2, CAPPED AT 8. Returns the armor actually gained,
//        which is 0 if it was already at the cap.
//
//    ABIL_RALLY   "Rally"   cost 6   not hostile   0 draws
//        +1 strength to every monster in the wave that is not dead and is
//        not self. Returns how many monsters were rallied.
//
//    unknown tag  "???"     cost 0   not hostile   1 draw
//        One attack_hero(), generic verbose line.
//
//  IN STEP 4 YOU ADD:
//
//    ABIL_SHATTER "Shatter" cost 7   hostile       1 draw
//        Raw damage of self.strength + lab_rand_range(1, 6), applied to
//        hero.hp with NO armor reduction at all and no floor adjustment
//        (the raw total is always at least 1). hero.hp floors at 0. Then
//        hero.armor drops by 1, floored at 0. Returns the damage dealt.
//        Note this does NOT call attack_hero(), because attack_hero()
//        subtracts armor. It makes its own single draw.
//
//  `verbose` is 0 or 1. Its exact text is not tested; the numbers are.
// ---------------------------------------------------------------------------
int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave,
                    int verbose);

#endif  // DD_LAB04_KILL_THE_SWITCH_H
