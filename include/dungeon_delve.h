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

#include <algorithm>
#include <cstddef>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "data_layer.h"

class Ability;

/* ---------------------------------------------------------------------------
   Entities: the Build 3 hierarchy.

   Diamond resolution. Entity, Combatant, and Lootable each inherit
   Describable virtually. Hero, Monster, and Merchant therefore contain one
   Describable subobject, not one per path. Without `virtual` on all three
   edges into Describable, Monster would get a Describable through Entity and
   another through Combatant and Lootable. describe() would be ambiguous and
   the shared base state would be duplicated. The virtual keyword belongs on
   the edge into Describable, because that is the base being collapsed.

   Ownership of state:
     Entity     owns the name and the description contract.
     Combatant  owns hit points, strength, and armor. Merchant has no hit
                points, so it does not inherit Combatant.
     Lootable   owns the "has gold that can change hands" capability.
   --------------------------------------------------------------------------- */

class Describable {
public:
    virtual ~Describable() = default;
    virtual std::string describe() const = 0;
    virtual std::string vitals_text() const = 0;
};

class Entity : public virtual Describable {
public:
    virtual ~Entity() = default;

    virtual std::string name() const { return name_; }
    void set_name(const char* name);

    virtual const char* kind() const = 0;
    virtual std::string capabilities() const = 0;

    std::string describe() const override;

protected:
    explicit Entity(const char* name = "");

private:
    std::string name_;
};

class Combatant : public virtual Describable {
public:
    virtual ~Combatant() = default;

    std::string vitals_text() const override;

    int hp       = 0;
    int max_hp   = 0;
    int strength = 0;
    int armor    = 0;
};

class Lootable : public virtual Describable {
public:
    virtual ~Lootable() = default;
    virtual int loot_gold() const = 0;
};

class Hero : public Entity, public Combatant {
public:
    Hero() = default;
    Hero(const char* name, int hp, int strength, int armor);
    ~Hero() override = default;

    const char* kind() const override { return "Hero"; }
    std::string capabilities() const override { return "combatant"; }

    int gold    = 0;
    int xp      = 0;
    int level   = 1;
    int potions = 2;
};

class Monster : public Entity, public Combatant, public Lootable {
public:
    Monster() = default;
    Monster(const char* name, const char* species, int hp, int strength,
            int armor, int gold_drop, int ability_tag);
    ~Monster() override = default;

    const char* kind() const override { return "Monster"; }
    std::string capabilities() const override { return "combatant, lootable"; }
    int loot_gold() const override { return gold_drop; }

    char species[24] = {};
    int  gold_drop     = 0;
    int  ability_tag   = 0;
    const Ability* ability = nullptr;   /* non-owning; the registry owns it */
    int  is_dead       = 0;
    int  summon_budget = 0;
};

class Merchant : public Entity, public Lootable {
public:
    Merchant() = default;
    Merchant(const char* name, int gold, int markup);
    ~Merchant() override = default;

    const char* kind() const override { return "Merchant"; }
    std::string capabilities() const override { return "lootable"; }
    std::string vitals_text() const override { return "--"; }
    int loot_gold() const override { return gold; }

    int gold           = 0;
    int markup_percent = 0;
    int stock_tags[8]  = { -1, -1, -1, -1, -1, -1, -1, -1 };
    int stock_count    = 0;
};

/* ---------------------------------------------------------------------------
   THE OWNERSHIP MODEL.

   Change these three lines in Build 1. Nothing else about the shape of the
   program has to change with them.
   --------------------------------------------------------------------------- */

using MonsterPtr  = std::unique_ptr<Monster>;
using HeroPtr     = std::unique_ptr<Hero>;
using MerchantPtr = std::unique_ptr<Merchant>;

using Wave = std::vector<MonsterPtr>;

/* ---------------------------------------------------------------------------
   The bestiary is the Week 5 data layer.

   It owns its monsters in a fixed-capacity Pool, so a monster's address does
   not move, and it keeps a Registry from species name to that address. The
   linear scan that used to find a species is gone: find is one ordered lookup
   at O(log n). count_species still works because the registry allows
   duplicate keys.
   --------------------------------------------------------------------------- */
class Bestiary {
public:
    void push_back(MonsterPtr monster);

    std::size_t size() const noexcept;
    const Monster* operator[](std::size_t index) const;

    Monster* find(const char* species) const;
    int count_species(const char* species) const;

private:
    Pool<Monster, 64>                            monsters_;
    Registry<Monster*, std::string, BySpecies>   by_species_;
};

/* ---------------------------------------------------------------------------
   Abilities: the Build 4 polymorphism layer.

   An ability is a class, not an integer case. The tag is only an address used
   to find the object. The resolver calls apply() through an Ability& and never
   asks which concrete type it is holding.
   --------------------------------------------------------------------------- */

class AbilityError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class TargetDefeated : public AbilityError {
public:
    TargetDefeated(const std::string& ability, const std::string& target)
        : AbilityError(ability + " cannot target " + target +
                       ": target is already defeated") {}
};

class WaveFull : public AbilityError {
public:
    WaveFull(const std::string& ability, std::size_t capacity)
        : AbilityError(ability + " cannot summon: wave is full at " +
                       std::to_string(capacity)) {}
};

class AbilityFailure : public AbilityError {
public:
    AbilityFailure(const std::string& ability, const std::string& caster,
                   const std::string& reason)
        : AbilityError("ability '" + ability + "' used by " + caster +
                       " failed: " + reason) {}
};

class Ability {
public:
    virtual ~Ability() = default;

    virtual const char* name() const = 0;
    virtual std::string describe() const = 0;
    virtual int apply(Monster& self, Hero& hero, Wave& wave,
                      int verbose) const = 0;
};

/* The second level of the hierarchy. It checks the shared precondition for
   every hostile ability, then hands the effect to the concrete class. Cleave
   overrides describe() again, which is the inherited-virtual demonstration. */
class HostileAbility : public Ability {
public:
    std::string describe() const override
    {
        return std::string(name()) + " (hostile)";
    }

    int apply(Monster& self, Hero& hero, Wave& wave, int verbose) const override
    {
        if (hero.hp <= 0)
            throw TargetDefeated(name(), hero.name());
        return effect(self, hero, wave, verbose);
    }

protected:
    virtual int effect(Monster& self, Hero& hero, Wave& wave,
                       int verbose) const = 0;
};

struct AbilityRegistration {
    AbilityRegistration(int tag, const Ability* ability);
};

const Ability* ability_for_tag(int tag);
const Ability* default_ability();
int ability_count();
void register_ability(int tag, const Ability* ability);

/* Produces a plain non-owning Monster* from whatever MonsterPtr currently is.
   Use this when a function needs to hand back something to look at without
   handing over ownership. Both overloads are here on purpose: one of them
   stops compiling being a problem when you change the alias. */
inline Monster* raw(Monster* p)                            { return p; }
inline Monster* raw(const std::unique_ptr<Monster>& p)     { return p.get(); }
inline Monster* raw(const std::shared_ptr<Monster>& p)     { return p.get(); }

/* A collection of entities that owns them through the Entity interface.
   describe_all() prints the roster and returns the same text. */
class Roster {
public:
    void add(HeroPtr hero);
    void add(MonsterPtr monster);
    void add(MerchantPtr merchant);
    void add(std::unique_ptr<Entity> entity);

    std::string describe_all() const;
    std::size_t size() const noexcept { return entities_.size(); }

    // A sorted view over the roster. The comparator decides the order, so one
    // body serves the function pointer, the function object, and the lambda.
    template <typename Compare>
    std::vector<const Entity*> sorted_by(Compare compare) const
    {
        std::vector<const Entity*> out;
        out.reserve(entities_.size());
        for (const auto& entity : entities_)
            out.push_back(entity.get());
        std::sort(out.begin(), out.end(), compare);
        return out;
    }

private:
    std::vector<std::unique_ptr<Entity>> entities_;
};

/* ---------------------------------------------------------------------------
   Week 2 domain vocabulary.

   Each type owns one invariant and enforces it in its constructor, so no
   caller has to check the value again. The money, harm and dice in this
   engine were plain ints before this; now they are their own types and the
   compiler will not let you add a purse to a hit-point total.
   --------------------------------------------------------------------------- */

class Coin {
public:
    explicit Coin(int amount);                 /* throws if amount < 0 */
    int value() const noexcept;

    Coin& operator+=(const Coin& rhs);
    Coin& operator-=(const Coin& rhs);         /* throws if the result < 0 */

    friend Coin operator+(Coin lhs, const Coin& rhs);
    friend Coin operator-(Coin lhs, const Coin& rhs);
    friend bool operator==(const Coin& lhs, const Coin& rhs);
    friend bool operator<(const Coin& lhs, const Coin& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Coin& c);

private:
    int amount_{0};                            /* invariant: never negative */
};

class Damage {
public:
    explicit Damage(int amount);               /* throws if amount < 0 */
    int value() const noexcept;

    Damage& operator+=(const Damage& rhs);

    friend Damage operator+(Damage lhs, const Damage& rhs);
    friend bool operator==(const Damage& lhs, const Damage& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Damage& d);

private:
    int amount_{0};                            /* invariant: never negative */
};

class Dice {
public:
    Dice(int count, int sides);                /* throws if either < 1 */
    int count() const noexcept;
    int sides() const noexcept;

private:
    int count_;
    int sides_;
};

class Vec2 {
public:
    Vec2(int x, int y);
    int x() const noexcept;
    int y() const noexcept;

    Vec2& operator+=(const Vec2& rhs);

    friend Vec2 operator+(Vec2 lhs, const Vec2& rhs);
    friend bool operator==(const Vec2& lhs, const Vec2& rhs);

private:
    int x_;
    int y_;
};

/* Stateless by design: there is nothing to construct, so the deleted
   constructor makes the compiler agree with us. */
class EncounterMath {
public:
    EncounterMath() = delete;

    static Damage roll(const Dice& dice);
    static Damage raw_damage(int strength, const Dice& dice);
    static Damage apply_armor(Damage raw, int armor);
};

/* The instance-level counterpoint to EncounterMath: a purse has a balance,
   so two callers can legitimately want different purses. Two purses cannot
   share one balance the way a static would force them to. */
class Purse {
public:
    void add(Coin amount);
    bool spend(Coin amount);                   /* false if the purse is short */
    Coin total() const noexcept;

private:
    Coin balance_{0};
};

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
#define ABIL_SHATTER 8   /* reserved for the open/closed test */

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
int apply_damage(Combatant& combatant, int incoming);
int apply_damage_to_monster(Monster& m, int incoming);
int apply_damage_to_hero(Hero& h, int incoming);

/* The functions named in HIERARCHY.md. Each one takes a base class and works
   for every derived type that reaches it. */
int         loot_gold_of(const Lootable& lootable);
std::string describe_entity(const Entity& entity);
std::string describe_object(const Describable& object);
int resolve_attack_on_monster(Hero& attacker, Monster& target);
int resolve_attack_on_hero(Monster& attacker, Hero& target);

/* Three resolution paths. The cost of each choice, not just the syntax:

   resolve(const Hero&, const Monster&) takes both by const reference and
   forecasts the damage. Cost: no copies, but the caller must keep both
   objects alive for the call, and nothing can be mutated.

   resolve(Hero&, Monster&) takes both by non-const reference and applies
   the damage. Cost: no copies, but the signature announces that the
   caller's objects change.

   resolve(Hero&, Monster&, Damage&) writes the result into a slot the
   caller supplies and returns whether the hero is still standing. Cost:
   no copy of the result, but the caller must declare the Damage first and
   remember to check the bool.

   Passing either object by value instead would copy the whole struct,
   including the 32-byte name array, on every call. */
Damage resolve(Hero& hero, Monster& monster);
Damage resolve(const Hero& hero, const Monster& monster);
bool   resolve(Hero& hero, Monster& monster, Damage& out);

/* Abilities. */
const char* ability_name(int tag);
int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave, int verbose);
int resolve_ability(const Ability& ability, Monster& self, Hero& hero,
                    Wave& wave, int verbose);

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
