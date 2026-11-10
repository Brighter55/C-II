#include "abilities.h"

namespace {

class Mend final : public Ability {
public:
    const char* name()      const override { return "Mend"; }
    int         mana_cost() const override { return 4; }
    bool        is_hostile() const override { return false; }

    int apply(Monster& self, Hero&, Wave&, int verbose) const override
    {
        const int rolled = roll_dice(2, 4);
        const int room   = (self.max_hp > self.hp) ? self.max_hp - self.hp : 0;
        const int amount = (rolled < room) ? rolled : room;
        self.hp += amount;
        announce(verbose, self, "mends ...................", amount);
        return amount;
    }
};

[[maybe_unused]] const Mend kMend;
[[maybe_unused]] const AbilityRegistration kMendRegistration(ABIL_MEND,
                                                              &kMend);

}  // namespace
