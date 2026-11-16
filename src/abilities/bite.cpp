#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Bite final : public HostileAbility {
public:
    const char* name() const override { return "Bite"; }

protected:
    int effect(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        int amount = resolve_attack_on_hero(self, hero);
        if (dd_rand_range(1, 100) > 70) {
            apply_damage_to_hero(hero, 2);
            amount += 2;
            if (verbose)
                std::printf("  %s bites deep ............. %d damage\n",
                            self.name().c_str(), amount);
        } else {
            if (verbose)
                std::printf("  %s bites ................... %d damage\n",
                            self.name().c_str(), amount);
        }
        return amount;
    }
};

[[maybe_unused]] const Bite kBite;
[[maybe_unused]] const AbilityRegistration kBiteRegistration(ABIL_BITE,
                                                              &kBite);

}  // namespace
