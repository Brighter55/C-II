#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Cleave final : public HostileAbility {
public:
    const char* name() const override { return "Cleave"; }

    // Second override of describe(): HostileAbility supplies one, this class
    // replaces it. The virtual keyword is inherited, not repeated.
    std::string describe() const override
    {
        return std::string(name()) + " (wide attack)";
    }

protected:
    int effect(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        int amount = resolve_attack_on_hero(self, hero);
        amount = amount + (amount / 2);
        apply_damage_to_hero(hero, amount / 2);
        if (verbose)
            std::printf("  %s cleaves ................. %d damage\n",
                        self.name().c_str(), amount);
        return amount;
    }
};

[[maybe_unused]] const Cleave kCleave;
[[maybe_unused]] const AbilityRegistration kCleaveRegistration(ABIL_CLEAVE,
                                                               &kCleave);

}  // namespace
