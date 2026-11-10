#include "abilities.h"

namespace {

class Flail final : public Ability {
public:
    const char* name()      const override { return "Flail"; }
    int         mana_cost() const override { return 0; }
    bool        is_hostile() const override { return true; }

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int amount = attack_hero(self, hero);
        announce(verbose, self, "flails ..................", amount);
        return amount;
    }
};

[[maybe_unused]] const Flail kFlail;
[[maybe_unused]] const AbilityRegistration kFlailRegistration(ABIL_NONE,
                                                              &kFlail);

}  // namespace
