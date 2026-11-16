#include "dungeon_delve.h"

#include <cstddef>
#include <cstdio>

namespace {

constexpr std::size_t MAX_WAVE_SIZE = 32;

class Summon final : public Ability {
public:
    const char* name() const override { return "Summon"; }
    std::string describe() const override
    {
        return "Summon: calls a Whelp into the wave";
    }

    int apply(Monster& self, Hero& hero, Wave& wave, int verbose) const override
    {
        if (self.summon_budget > 0) {
            if (wave.size() >= MAX_WAVE_SIZE)
                throw WaveFull(name(), MAX_WAVE_SIZE);

            self.summon_budget--;
            wave.push_back(make_monster("Whelp", "whelp", 6, 2, 0, 3, ABIL_NONE));
            if (verbose)
                std::printf("  %s summons a Whelp!\n", self.name().c_str());
            return 1;
        }

        const int amount = resolve_attack_on_hero(self, hero);
        if (verbose)
            std::printf("  %s is out of whelps ....... %d damage\n",
                        self.name().c_str(), amount);
        return amount;
    }
};

[[maybe_unused]] const Summon kSummon;
[[maybe_unused]] const AbilityRegistration kSummonRegistration(ABIL_SUMMON,
                                                                &kSummon);

}  // namespace
