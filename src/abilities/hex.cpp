#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Hex final : public HostileAbility {
public:
    const char* name() const override { return "Hex"; }

protected:
    int effect(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        int amount = roll_dice(1, 4);
        hero.strength -= amount;
        if (hero.strength < 1) hero.strength = 1;
        if (verbose)
            std::printf("  %s hexes ................... -%d STR\n",
                        self.name().c_str(), amount);
        return amount;
    }
};

[[maybe_unused]] const Hex kHex;
[[maybe_unused]] const AbilityRegistration kHexRegistration(ABIL_HEX, &kHex);

}  // namespace
