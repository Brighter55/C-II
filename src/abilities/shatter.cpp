// The open/closed test. This file was added after the refactor and it touches
// no existing file: the tag is reserved in the header and the build globs
// src/abilities/*.cpp.

#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Shatter final : public HostileAbility {
public:
    const char* name() const override { return "Shatter"; }
    std::string describe() const override
    {
        return "Shatter: raw damage that strips armor";
    }

protected:
    int effect(Monster& self, Hero& hero, Wave&, int verbose) const override
    {
        const int amount = self.strength + dd_rand_range(1, 6);
        hero.hp -= amount;
        if (hero.hp < 0) hero.hp = 0;
        hero.armor -= 1;
        if (hero.armor < 0) hero.armor = 0;
        if (verbose)
            std::printf("  %s shatters ................ %d damage\n",
                        self.name().c_str(), amount);
        return amount;
    }
};

[[maybe_unused]] const Shatter kShatter;
[[maybe_unused]] const AbilityRegistration kShatterRegistration(ABIL_SHATTER,
                                                                &kShatter);

}  // namespace
