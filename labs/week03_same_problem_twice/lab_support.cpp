// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//
//  Supplied support. DO NOT MODIFY THIS FILE.
//
//  find_spell() is written once, in terms of the four functions you
//  implement, and linked into both versions. You do not write it twice --
//  which is itself worth noticing: the part of the design that did not need
//  to know how spells are built did not have to be duplicated.
// ===========================================================================
#include "spellbook.h"

#include <cstring>

int find_spell(const char* name)
{
    if (name == nullptr) return -1;

    const int n = spell_count();
    for (int i = 0; i < n; ++i) {
        const char* candidate = spell_name(i);
        if (candidate != nullptr && std::strcmp(candidate, name) == 0)
            return i;
    }
    return -1;
}
