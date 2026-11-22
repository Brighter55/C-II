/* ===========================================================================
   IS 267 — Concept Lab 5:  Generic and Non-Generic, Side by Side

   legacy_loot.h -- THE RAW SIDE, and the bridge between the two sides.

   LOOT_TABLE is the engine's own loot table, lifted out of
   src/dungeon_delve.cpp unchanged: a plain C array of const char*, a
   #define for its length, and code that indexes it directly. The comment
   that ships with it in the engine says "the content team edits it by hand
   in a text editor and they do not want it moved into a database or a map
   or anything clever. Leave it alone."

   Take that literally. This table is the part of the program you are NOT
   allowed to improve. Your bridge works against it as it is.
   =========================================================================== */

#ifndef DD_LAB05_LEGACY_LOOT_H
#define DD_LAB05_LEGACY_LOOT_H

#include <string>

#include "satchel.h"

/* --------------------------------------------------------------------------
   The table itself. Pre-template style, and staying that way: an array, a
   separate count, and a second parallel array of values that the first one
   has no idea exists.
   -------------------------------------------------------------------------- */

#define LOOT_COUNT 12

extern const char* LOOT_TABLE[LOOT_COUNT];
extern int         LOOT_VALUE[LOOT_COUNT];

/* The engine indexes LOOT_TABLE directly, in several places, and still does.
   These two are supplied; they are what "directly" looks like. */
const char* loot_at(int i);
int         loot_count(void);
void        print_loot_table(void);

/* --------------------------------------------------------------------------
   THE OPERATION, the raw way.

   Same contract as Satchel<T>::remove_at: take out the entry at `index`,
   close the gap, preserve the order of the survivors, keep the count
   honest, return false on a bad index.

   Note what the signature has to carry that the template version did not:
   the slots and the count travel separately, the capacity is not knowable
   from either of them, and nothing here says the two arguments belong
   together. That is not an accident of this lab -- that is the style.

   `slots` points at a WORKING COPY of the table, not at LOOT_TABLE itself.
   Never call this on LOOT_TABLE.
   -------------------------------------------------------------------------- */
bool raw_remove_at(const char** slots, int* count, int index);

/* --------------------------------------------------------------------------
   LegacyLootBridge -- the adapter, in both directions.

   This is the same job Build 5's LegacyLootBridge does inside the engine,
   and it works under the same constraint: the legacy side cannot be
   changed, so the adapter absorbs the mismatch.

   THE RULE: the bridge PRESERVES the raw table. It does not write to it,
   reorder it, reallocate it, or free it. LOOT_TABLE holds pointers to
   string literals in read-only storage; delete[] on it is undefined
   behaviour, the sanitizer will say so, and the lab is failed. Rebuilding
   the table out of freshly allocated copies of the same text fails too --
   the tests compare the pointer values, not just the characters.
   -------------------------------------------------------------------------- */
struct LegacyLootBridge {

    /* raw -> template.
       Append one entry per raw slot to `out`, in order, and return how many
       were appended. Return -1 and append nothing if `out` cannot hold them.

       WITHOUT COPYING TWICE: the characters land in the satchel's own
       storage once. Use Satchel<T>::add_slot() and assign the raw
       const char* straight into the slot it hands you. Do NOT build a
       std::vector<std::string> (or a second Satchel, or an array of
       std::string) as a staging area and copy that in, and do not go
       through add(), which constructs a std::string at the call site and
       then copies it into the slot. One pass, one copy of the text.

       This half is checked by reading your code, not by a test -- see
       README.md. The store() half below is checked by a test. */
    static int load(const char* const* slots, int count,
                    Satchel<std::string>& out);

    /* template -> raw.
       Write one const char* per entry of `satchel` into `dest`, in order,
       and return how many were written. Return -1 and write nothing if
       `dest_capacity` is too small.

       WITHOUT COPYING TWICE: the pointers you write must point at the
       satchel's own strings -- c_str() of the string already living in the
       satchel. Do not allocate new buffers and do not strcpy the text
       anywhere. The test for this compares the pointer you wrote against
       satchel.at(i).c_str(), so a copy of the right characters still fails.

       The caller owns `dest` and the satchel outlives it. */
    static int store(const Satchel<std::string>& satchel,
                     const char** dest, int dest_capacity);
};

#endif /* DD_LAB05_LEGACY_LOOT_H */
