#include "abilities.h"

namespace {

class Guard final : public Ability {
public:
    const char* name()      const override { return "Guard"; }
    int         mana_cost() const override { return 2; }
    bool        is_hostile() const override { return false; }

    int apply(Monster& self, Hero&, Wave&, int verbose) const override
    {
        const int before = self.armor;
        self.armor = (self.armor + 2 > 8) ? 8 : self.armor + 2;  // the cap
        const int amount = self.armor - before;
        announce(verbose, self, "guards ..................", amount);
        return amount;
    }
};

[[maybe_unused]] const Guard kGuard;
[[maybe_unused]] const AbilityRegistration kGuardRegistration(ABIL_GUARD,
                                                               &kGuard);

}  // namespace
