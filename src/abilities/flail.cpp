#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Flail final : public HostileAbility {
public:
    const char* name() const override { return "Flail"; }

protected:
    int effect(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int amount = resolve_attack_on_hero(self, hero);
        if (verbose)
            std::printf("  %s flails .................. %d damage\n",
                        self.name().c_str(), amount);
        return amount;
    }
};

[[maybe_unused]] const Flail kFlail;
[[maybe_unused]] const AbilityRegistration kFlailRegistration(ABIL_NONE,
                                                              &kFlail);

}  // namespace
