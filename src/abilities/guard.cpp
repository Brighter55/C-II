#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Guard final : public Ability {
public:
    const char* name() const override { return "Guard"; }
    std::string describe() const override
    {
        return "Guard: raises the caster's armor";
    }

    int apply(Monster& self, Hero&, Wave&, int verbose) const override
    {
        self.armor += 2;
        if (verbose)
            std::printf("  %s guards .................. +2 armor\n",
                        self.name().c_str());
        return 2;
    }
};

[[maybe_unused]] const Guard kGuard;
[[maybe_unused]] const AbilityRegistration kGuardRegistration(ABIL_GUARD,
                                                               &kGuard);

}  // namespace
