#include "abilities.h"

namespace {

class Bite final : public Ability {
public:
    const char* name()      const override { return "Bite"; }
    int         mana_cost() const override { return 2; }
    bool        is_hostile() const override { return true; }

    int apply(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int first = attack_hero(self, hero);
        const int roll  = lab_rand_range(1, 100);  // drawn either way
        int amount = first;
        if (roll > 70) {
            hero.hp -= 2;
            if (hero.hp < 0) hero.hp = 0;
            amount += 2;
        }
        announce(verbose, self, "bites ...................", amount);
        return amount;
    }
};

[[maybe_unused]] const Bite kBite;
[[maybe_unused]] const AbilityRegistration kBiteRegistration(ABIL_BITE,
                                                              &kBite);

}  // namespace
