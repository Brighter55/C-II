#include "abilities.h"

namespace {

class Summon final : public Ability {
public:
    const char* name()      const override { return "Summon"; }
    int         mana_cost() const override { return 8; }
    bool        is_hostile() const override { return false; }

    int apply(Monster& self, Hero& hero, Wave& wave, int verbose) const override
    {
        if (self.summon_budget > 0) {
            self.summon_budget--;
            spawn_into_wave(wave, "Whelp", 6, 2, 0);
            announce(verbose, self, "summons a Whelp .........", 1);
            return 1;
        }

        const int amount = attack_hero(self, hero);
        announce(verbose, self, "is out of whelps ........", amount);
        return amount;
    }
};

[[maybe_unused]] const Summon kSummon;
[[maybe_unused]] const AbilityRegistration kSummonRegistration(ABIL_SUMMON,
                                                                &kSummon);

}  // namespace
