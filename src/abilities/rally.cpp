#include "dungeon_delve.h"

#include <cstdio>

namespace {

class Rally final : public Ability {
public:
    const char* name() const override { return "Rally"; }
    std::string describe() const override
    {
        return "Rally: raises the strength of the pack";
    }

    int apply(Monster& self, Hero&, Wave& wave, int verbose) const override
    {
        for (std::size_t i = 0; i < wave.size(); i++) {
            if (!wave[i]->is_dead && raw(wave[i]) != &self)
                wave[i]->strength += 1;
        }
        if (verbose)
            std::printf("  %s rallies the pack ........ +1 STR each\n",
                        self.name().c_str());
        return 1;
    }
};

[[maybe_unused]] const Rally kRally;
[[maybe_unused]] const AbilityRegistration kRallyRegistration(ABIL_RALLY,
                                                               &kRally);

}  // namespace
