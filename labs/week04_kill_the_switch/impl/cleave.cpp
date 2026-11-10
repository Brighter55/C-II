#include "abilities.h"

namespace {

class Cleave final : public Ability {
public:
    const char* name()      const override { return "Cleave"; }
    int         mana_cost() const override { return 3; }
    bool        is_hostile() const override { return true; }

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int first  = attack_hero(self, hero);
        const int follow = (first / 2 > 1) ? first / 2 : 1;
        hero.hp -= follow;
        if (hero.hp < 0) hero.hp = 0;
        const int amount = first + follow;
        announce(verbose, self, "cleaves .................", amount);
        return amount;
    }
};

[[maybe_unused]] const Cleave kCleave;
[[maybe_unused]] const AbilityRegistration kCleaveRegistration(ABIL_CLEAVE,
                                                                &kCleave);

}  // namespace
