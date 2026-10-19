/* ===========================================================================
   dungeon_delve.h

   Somebody finally pulled the declarations out of the .cpp so the tests would
   stop duplicating them. That is as far as it got.

   READ THIS BEFORE BUILD 1.

   The three aliases below -- MonsterPtr, HeroPtr, MerchantPtr -- are the
   ownership model of this entire engine, written down in three lines. Right
   now all three are raw pointers, and nothing in the program deletes them.

   Your Build 1 conversion happens HERE first. Change an alias to a smart
   pointer and the whole engine follows, including the tests. That is the
   point of putting them in one place.

   Do not change the function signatures to match a particular alias. They
   are written in terms of references and the aliases so that they keep
   compiling whichever way you take the ownership.
   =========================================================================== */

#ifndef DUNGEON_DELVE_H
#define DUNGEON_DELVE_H

#include <memory>
#include <string>
#include <vector>

/* ---------------------------------------------------------------------------
   Entities. Three separate structs. They share most of their fields but a
   hero is not a monster and a monster is not a merchant, so three structs.
   --------------------------------------------------------------------------- */

struct Hero {
    char name[32];
    int  hp;
    int  max_hp;
    int  strength;
    int  armor;
    int  gold;
    int  xp;
    int  level;
    int  potions;
};

struct Monster {
    char name[32];
    char species[24];
    int  hp;
    int  max_hp;
    int  strength;
    int  armor;
    int  gold_drop;
    int  ability_tag;
    int  is_dead;
    int  summon_budget;
};

struct Merchant {
    char name[32];
    int  gold;
    int  markup_percent;
    int  stock_tags[8];
    int  stock_count;
};

/* ---------------------------------------------------------------------------
   THE OWNERSHIP MODEL.

   Change these three lines in Build 1. Nothing else about the shape of the
   program has to change with them.
   --------------------------------------------------------------------------- */

using MonsterPtr  = Monster*;
using HeroPtr     = Hero*;
using MerchantPtr = Merchant*;

using Wave     = std::vector<MonsterPtr>;
using Bestiary = std::vector<MonsterPtr>;

/* Produces a plain non-owning Monster* from whatever MonsterPtr currently is.
   Use this when a function needs to hand back something to look at without
   handing over ownership. Both overloads are here on purpose: one of them
   stops compiling being a problem when you change the alias. */
inline Monster* raw(Monster* p)                            { return p; }
inline Monster* raw(const std::unique_ptr<Monster>& p)     { return p.get(); }
inline Monster* raw(const std::shared_ptr<Monster>& p)     { return p.get(); }

/* ---------------------------------------------------------------------------
   Ability tags.

   Every ability in the game is an int. resolve_ability() switches on it.
   Adding an ability means adding a tag here and a case in every switch that
   looks at tags, and there are a few of those now.
   --------------------------------------------------------------------------- */

#define ABIL_NONE    0
#define ABIL_CLEAVE  1
#define ABIL_BITE    2
#define ABIL_MEND    3
#define ABIL_HEX     4
#define ABIL_SUMMON  5
#define ABIL_GUARD   6
#define ABIL_RALLY   7

/* ---------------------------------------------------------------------------
   The engine.
   --------------------------------------------------------------------------- */

/* RNG -- deterministic for a given seed, so a bug you can reproduce once you
   can reproduce every time. */
void dd_seed(unsigned s);
int  dd_rand();
int  dd_rand_range(int lo, int hi);      /* inclusive on both ends */
int  roll_dice(int count, int sides);

/* Loot. The table is a raw C array; the content team edits it by hand. */
const char* loot_at(int i);
int         loot_count();
const char* roll_loot();
int         loot_value_of(const char* loot_name);

/* Construction. */
HeroPtr     make_hero(const char* name, int hp, int strength, int armor);
MonsterPtr  make_monster(const char* name, const char* species, int hp,
                         int strength, int armor, int gold_drop, int ability_tag);
MerchantPtr make_merchant(const char* name, int gold, int markup);

/* Damage. */
int compute_raw_damage(int strength, int dice_count, int dice_sides);
int apply_armor(int raw_damage, int armor);
int apply_damage_to_monster(Monster& m, int incoming);
int apply_damage_to_hero(Hero& h, int incoming);
int resolve_attack_on_monster(Hero& attacker, Monster& target);
int resolve_attack_on_hero(Monster& attacker, Hero& target);

/* Abilities. */
const char* ability_name(int tag);
int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave, int verbose);

/* Bestiary. */
Bestiary   build_bestiary();
Monster*   find_in_bestiary(Bestiary& bestiary, const char* species);
int        count_species_in_bestiary(Bestiary& bestiary, const char* species);
MonsterPtr spawn_from_bestiary(Bestiary& bestiary, const char* species);

/* Encounters. */
int  wave_is_cleared(Wave& wave);
int  living_in_wave(Wave& wave);
int  run_monster_round(Wave& wave, Hero& hero, int verbose,
                       std::vector<std::string>* acted_names);
int  run_hero_round(Wave& wave, Hero& hero, int verbose);
int  run_encounter(Hero& hero, Wave& wave, int verbose);
Wave build_wave(Bestiary& bestiary, const char* a, const char* b, const char* c);

/* Progression and the merchant. */
int  xp_for_next_level(int level);
void check_level_up(Hero& h, int verbose);
int  merchant_price(Merchant& mer, int base_value);
void visit_merchant(Merchant& mer, Hero& hero, int verbose);
void quaff_potion(Hero& h, int verbose);

/* Save and load. Plain text, one field per line, order matters. */
int     save_hero(Hero& h, const char* path);
HeroPtr load_hero(const char* path);

/* Display. Printing is done wherever it is convenient. */
void print_banner();
void print_hero(Hero& h);
void print_wave(Wave& wave);
void print_bestiary(Bestiary& bestiary);
void print_loot_table();

/* The campaign. */
int run_campaign(int verbose);

#endif /* DUNGEON_DELVE_H */
