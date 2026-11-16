// The tag registry and the resolver. A tag is an address, not a branch.
// Every concrete ability registers itself at startup; resolve_ability() does
// one lookup and one virtual call.

#include "dungeon_delve.h"

#include <cstdio>
#include <map>

namespace {

using Registry = std::map<int, const Ability*>;

Registry& registry()
{
    static Registry instance;
    return instance;
}

// The supplied engine treated an unknown tag like the switch's default case.
// Keep that behavior without putting a type check back in the resolver.
class UnknownAbility final : public Ability {
public:
    const char* name() const override { return "???"; }
    std::string describe() const override { return "unknown ability"; }

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int amount = resolve_attack_on_hero(self, hero);
        if (verbose)
            std::printf("  %s does something ......... %d damage\n",
                        self.name().c_str(), amount);
        return amount;
    }
};

UnknownAbility& unknown()
{
    static UnknownAbility instance;
    return instance;
}

}  // namespace

void register_ability(int tag, const Ability* ability)
{
    registry()[tag] = ability;
}

const Ability* ability_for_tag(int tag)
{
    const auto it = registry().find(tag);
    return (it == registry().end()) ? nullptr : it->second;
}

const Ability* default_ability()
{
    return &unknown();
}

AbilityRegistration::AbilityRegistration(int tag, const Ability* ability)
{
    register_ability(tag, ability);
}

int ability_count()
{
    return static_cast<int>(registry().size());
}

const char* ability_name(int tag)
{
    const Ability* ability = ability_for_tag(tag);
    return ability ? ability->name() : "???";
}

int resolve_ability(const Ability& ability, Monster& self, Hero& hero,
                    Wave& wave, int verbose)
{
    try {
        return ability.apply(self, hero, wave, verbose);
    } catch (const AbilityError& e) {
        // Add the ability and caster to the failure, then let the encounter
        // loop decide what to do with it. Catch by const& so the derived
        // exception is not sliced.
        throw AbilityFailure(ability.name(), self.name(), e.what());
    }
}

int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave,
                    int verbose)
{
    const Ability* ability = ability_for_tag(tag);
    if (ability == nullptr) ability = default_ability();
    return resolve_ability(*ability, self, hero, wave, verbose);
}
