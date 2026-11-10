// ===========================================================================
//  IS 267 — Concept Lab 4:  Kill the Switch
//
//  The test harness, shared by tests.cpp and variant_tests.cpp.
//  DO NOT MODIFY THIS FILE.
//
//  Seven lines of checking and one fixed scene. Both test programs are a
//  single translation unit each, so the file-scope counters below are fine
//  where they are.
// ===========================================================================
#ifndef DD_LAB04_TEST_HARNESS_H
#define DD_LAB04_TEST_HARNESS_H

#include "kill_the_switch.h"

#include <cstdio>
#include <string_view>

inline int g_failures = 0;
inline int g_checks   = 0;

inline const unsigned SEED = 1337u;

inline void check(bool ok, const char* what)
{
    ++g_checks;
    if (ok) { std::printf("  ok    %s\n", what); }
    else    { std::printf("  FAIL  %s\n", what); ++g_failures; }
}

inline void eq(int actual, int expected, const char* what)
{
    ++g_checks;
    if (actual == expected) { std::printf("  ok    %s\n", what); }
    else { std::printf("  FAIL  %s  (expected %d, got %d)\n",
                       what, expected, actual); ++g_failures; }
}

inline void named(const char* actual, const char* expected, const char* what)
{
    check(std::string_view(actual ? actual : "") == expected, what);
}

inline int summary()
{
    std::printf("\n=======================================\n");
    if (g_failures == 0) std::printf("ALL TESTS PASSED  (%d checks)\n", g_checks);
    else std::printf("%d of %d checks FAILED\n", g_failures, g_checks);
    std::printf("=======================================\n");
    return g_failures == 0 ? 0 : 1;
}

// A fresh, identical encounter for every single check: one caster with mana
// and a summon budget, one living ally, and one corpse that Rally must skip.
struct Scene {
    Wave  wave;
    Hero  hero{"Ash", 60, 60, 9, 3};
    Monster* caster = nullptr;

    Scene()
    {
        caster = spawn_into_wave(wave, "Gorthak", 30, 7, 2);
        caster->mana          = 20;
        caster->summon_budget = 1;
        caster->hp            = 22;          // room for Mend to heal into
        spawn_into_wave(wave, "Snarl", 10, 3, 0);
        Monster* corpse = spawn_into_wave(wave, "Husk", 10, 3, 0);
        corpse->is_dead = true;
    }
};

#endif  // DD_LAB04_TEST_HARNESS_H
