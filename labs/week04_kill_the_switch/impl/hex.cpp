#include "abilities.h"

namespace {

class Hex final : public Ability {
public:
    const char* name()      const override { return "Hex"; }
    int         mana_cost() const override { return 5; }
    bool        is_hostile() const override { return true; }

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int before = hero.strength;
        hero.strength -= roll_dice(1, 4);
        if (hero.strength < 1) hero.strength = 1;
        const int amount = before - hero.strength;
        announce(verbose, self, "hexes ...................", amount);
        return amount;
    }
};

[[maybe_unused]] const Hex kHex;
[[maybe_unused]] const AbilityRegistration kHexRegistration(ABIL_HEX, &kHex);

}  // namespace
