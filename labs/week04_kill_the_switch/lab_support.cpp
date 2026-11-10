// ===========================================================================
//  IS 267 — Concept Lab 4:  Kill the Switch
//
//  Supplied support. DO NOT MODIFY THIS FILE.
//
//  The RNG, the damage arithmetic, and the wave helper. None of this is what
//  the lab is about; it is here so that replacing the dispatch does not also
//  mean reimplementing the engine's arithmetic.
//
//  lab_rand_calls() is the instrumentation that makes "behavior is identical"
//  mean something. Two implementations can produce the same damage numbers
//  for one call and then diverge forever because one of them drew a random
//  number the other did not. The tests count the draws.
// ===========================================================================
#include "kill_the_switch.h"

namespace {

unsigned g_state = 1u;
unsigned g_calls = 0u;

unsigned next_raw()
{
    ++g_calls;
    g_state = g_state * 1103515245u + 12345u;
    return (g_state >> 16) & 0x7fffu;
}

}  // namespace

void lab_seed(unsigned s)
{
    g_state = (s == 0u) ? 1u : s;
    g_calls = 0u;
}

int lab_rand_range(int lo, int hi)
{
    if (hi <= lo) return lo;
    const unsigned span = static_cast<unsigned>(hi - lo + 1);
    return lo + static_cast<int>(next_raw() % span);
}

int roll_dice(int count, int sides)
{
    int total = 0;
    for (int i = 0; i < count; ++i)
        total += lab_rand_range(1, sides);
    return total;
}

unsigned lab_rand_calls() { return g_calls; }

int attack_hero(Monster& self, Hero& target)
{
    const int raw = self.strength + lab_rand_range(1, 6);
    int dealt = raw - target.armor;
    if (dealt < 1) dealt = 1;
    target.hp -= dealt;
    if (target.hp < 0) target.hp = 0;
    return dealt;
}

Monster* spawn_into_wave(Wave& wave, const char* name,
                         int hp, int strength, int armor)
{
    auto m = std::make_unique<Monster>(
        Monster{name, hp, hp, strength, armor, 0, 0, false});
    Monster* raw_ptr = m.get();
    wave.push_back(std::move(m));
    return raw_ptr;
}

int living_in_wave(const Wave& wave)
{
    int n = 0;
    for (const auto& m : wave)
        if (m && !m->is_dead) ++n;
    return n;
}
