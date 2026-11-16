#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Mend final : public Ability {
public:
    const char* name() const override { return "Mend"; }
    std::string describe() const override { return "Mend: heals the caster"; }

    int apply(Monster& self, Hero&, Wave&, int verbose) const override
    {
        const int amount = roll_dice(2, 4);
        self.hp += amount;
        if (self.hp > self.max_hp) self.hp = self.max_hp;
        if (verbose)
            std::printf("  %s mends ................... +%d HP\n",
                        self.name().c_str(), amount);
        return amount;
    }
};

[[maybe_unused]] const Mend kMend;
[[maybe_unused]] const AbilityRegistration kMendRegistration(ABIL_MEND,
                                                              &kMend);

}  // namespace
