#include "abilities.h"

namespace {

class Rally final : public Ability {
public:
    const char* name()      const override { return "Rally"; }
    int         mana_cost() const override { return 6; }
    bool        is_hostile() const override { return false; }

    int apply(Monster& self, Hero&, Wave& wave, int verbose) const override
    {
        int amount = 0;
        for (const auto& m : wave) {
            if (m && !m->is_dead && m.get() != &self) {
                m->strength += 1;
                ++amount;
            }
        }
        announce(verbose, self, "rallies the pack ........", amount);
        return amount;
    }
};

[[maybe_unused]] const Rally kRally;
[[maybe_unused]] const AbilityRegistration kRallyRegistration(ABIL_RALLY,
                                                               &kRally);

}  // namespace
