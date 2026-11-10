// ===========================================================================
//  Concept Lab 4 -- the ability interface and the registry.
//
//  The abstract base replaces the three metadata switches and the dispatch
//  switch. Every concrete ability answers the same four questions, and the
//  registry is how the rest of the program finds an ability by tag without
//  asking what kind of ability it is.
//
//  A new ability adds one .cpp in impl/ that defines its class and registers
//  an instance. Nothing in this file or in abilities.cpp changes.
// ===========================================================================
#ifndef DD_LAB04_ABILITIES_H
#define DD_LAB04_ABILITIES_H

#include "kill_the_switch.h"

#include <map>

class Ability {
public:
    virtual ~Ability() = default;

    virtual const char* name()      const = 0;
    virtual int         mana_cost() const = 0;
    virtual bool        is_hostile() const = 0;

    // Run the effect and return the amount the caller reports. The mana has
    // already been spent by resolve_ability(); this is only the effect.
    virtual int apply(Monster& self, Hero& hero, Wave& wave,
                      int verbose) const = 0;
};

// One verbose line per ability, shared so each apply() is about its effect.
void announce(int verbose, const Monster& self, const char* phrase,
              int amount);

// The tag-to-ability table. The function-local map keeps registration safe
// against static-initialisation order across translation units.
std::map<int, const Ability*>& ability_registry();
const Ability* find_ability(int tag);

// Construct one of these at namespace scope in an ability's .cpp file.
struct AbilityRegistration {
    AbilityRegistration(int tag, const Ability* ability)
    {
        ability_registry()[tag] = ability;
    }
};

#endif  // DD_LAB04_ABILITIES_H
