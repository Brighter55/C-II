// ===========================================================================
//  Concept Lab 4 -- the public functions, the registry, and the shared rule.
//
//  The tags are still how the engine addresses an ability. The lookup below
//  turns a tag into a pointer once; from there everything is a virtual call.
//  Nothing here asks what kind of ability it is holding.
// ===========================================================================
#include "abilities.h"

#include <cstdio>
#include <iterator>

void announce(int verbose, const Monster& self, const char* phrase, int amount)
{
    if (verbose)
        std::printf("  %s %s %d\n", self.name, phrase, amount);
}

std::map<int, const Ability*>& ability_registry()
{
    static std::map<int, const Ability*> registry;
    return registry;
}

const Ability* find_ability(int tag)
{
    const auto it = ability_registry().find(tag);
    return (it == ability_registry().end()) ? nullptr : it->second;
}

const char* ability_name(int tag)
{
    const Ability* ability = find_ability(tag);
    return ability ? ability->name() : "???";
}

int ability_mana_cost(int tag)
{
    const Ability* ability = find_ability(tag);
    return ability ? ability->mana_cost() : 0;
}

bool ability_is_hostile(int tag)
{
    const Ability* ability = find_ability(tag);
    return ability ? ability->is_hostile() : false;
}

int ability_count()
{
    return static_cast<int>(ability_registry().size());
}

int ability_tag_at(int index)
{
    if (index < 0 || index >= ability_count()) return -1;
    auto it = ability_registry().begin();
    std::advance(it, index);
    return it->first;
}

int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave,
                    int verbose)
{
    const Ability* ability = find_ability(tag);
    const int cost = ability ? ability->mana_cost() : 0;

    // The one rule every ability shares, including one nobody implemented.
    if (self.mana < cost) {
        if (verbose)
            std::printf("  %s tries to %s and fizzles\n",
                        self.name, ability_name(tag));
        return 0;
    }
    self.mana -= cost;

    // An unknown tag behaves exactly like the old switch's default case.
    if (!ability) {
        const int amount = attack_hero(self, hero);
        announce(verbose, self, "does something ..........", amount);
        return amount;
    }

    return ability->apply(self, hero, wave, verbose);
}
