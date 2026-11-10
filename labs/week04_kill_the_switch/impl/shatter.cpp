#include "abilities.h"

namespace {

class Shatter final : public Ability {
public:
    const char* name()      const override { return "Shatter"; }
    int         mana_cost() const override { return 7; }
    bool        is_hostile() const override { return true; }

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int amount = self.strength + lab_rand_range(1, 6);
        hero.hp -= amount;
        if (hero.hp < 0) hero.hp = 0;
        hero.armor -= 1;
        if (hero.armor < 0) hero.armor = 0;
        announce(verbose, self, "shatters ................", amount);
        return amount;
    }
};

[[maybe_unused]] const Shatter kShatter;
[[maybe_unused]] const AbilityRegistration kShatterRegistration(ABIL_SHATTER,
                                                                &kShatter);

}  // namespace
