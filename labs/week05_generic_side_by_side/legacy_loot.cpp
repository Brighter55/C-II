/* ===========================================================================
   IS 267 — Concept Lab 5:  Generic and Non-Generic, Side by Side

   legacy_loot.cpp

   Two parts to this file:

     SUPPLIED  -- the loot table and the code that indexes it. Do not edit.
     YOURS     -- raw_remove_at(), and the two halves of LegacyLootBridge.

   The dividing line is marked. Everything above it is the legacy you are
   bridging to; everything below it is the bridge.
   =========================================================================== */

#include <cstdio>
#include <cstring>

#include "legacy_loot.h"

/* ====================== SUPPLIED -- DO NOT EDIT ========================== */

/* Lifted from src/dungeon_delve.cpp, unchanged. */
const char* LOOT_TABLE[LOOT_COUNT] = {
    "Rusted Dagger",
    "Cracked Buckler",
    "Mouldy Rations",
    "Copper Ring",
    "Tin Whistle",
    "Bent Lockpick",
    "Waterlogged Scroll",
    "Chipped Whetstone",
    "Iron Spur",
    "Glass Eye",
    "Knotted Rope",
    "Tarnished Locket"
};

int LOOT_VALUE[LOOT_COUNT] = {
    4, 6, 1, 12, 3, 8, 15, 5, 7, 2, 3, 20
};

const char* loot_at(int i)
{
    if (i < 0 || i >= LOOT_COUNT) return nullptr;
    return LOOT_TABLE[i];          /* indexed directly, as the engine does */
}

int loot_count(void)
{
    return LOOT_COUNT;
}

void print_loot_table(void)
{
    std::printf("Loot table (%d entries, indexed directly):\n", LOOT_COUNT);
    for (int i = 0; i < LOOT_COUNT; i++)
        std::printf("  [%2d] %-22s %3d gold\n", i, LOOT_TABLE[i], LOOT_VALUE[i]);
}

/* ========================= YOURS -- BELOW HERE =========================== */

/* ---------------------------------------------------------------------------
   TODO: the raw half of THE OPERATION.

   Remove the entry at `index` from `slots`, which currently holds `*count`
   entries. Shift the survivors down one slot each, in order. Decrement
   `*count`. Return true.

   Return false and change nothing if `index` is out of range, or if either
   pointer is null.

   Clear the slot that falls off the end -- set it to nullptr -- so that a
   stale pointer cannot be read back out past the count. The tests check it.
   Nothing forced you to do that on the template side; ask yourself why.
   --------------------------------------------------------------------------- */
bool raw_remove_at(const char** slots, int* count, int index)
{
    if (slots == nullptr || count == nullptr) return false;
    if (index < 0 || index >= *count) return false;

    for (int i = index; i < *count - 1; ++i)
        slots[i] = slots[i + 1];

    --*count;
    slots[*count] = nullptr;
    return true;
}

/* ---------------------------------------------------------------------------
   TODO: raw -> template.

   Append one entry per raw slot to `out`, in order; return how many you
   appended. Return -1 and append nothing if `out` cannot hold `count`
   entries, or if `slots` is null or `count` is negative.

   Check the capacity BEFORE you append anything -- "append nothing" means
   nothing, not eleven of twelve and then a -1.

   One copy of the text per entry, straight into the slot that add_slot()
   hands you. No staging area, and not through add().
   --------------------------------------------------------------------------- */
int LegacyLootBridge::load(const char* const* slots, int count,
                           Satchel<std::string>& out)
{
    if (slots == nullptr || count < 0) return -1;
    if (count > out.capacity - out.size()) return -1;

    for (int i = 0; i < count; ++i) {
        std::string* slot = out.add_slot();
        *slot = slots[i];
    }
    return count;
}

/* ---------------------------------------------------------------------------
   TODO: template -> raw.

   Write one const char* per entry of `satchel` into `dest`, in order;
   return how many you wrote. Return -1 and write nothing if `dest` is null
   or `dest_capacity` is smaller than satchel.size().

   Each pointer must be the satchel's own string buffer -- c_str() of the
   string that is already in there. No new allocations, no strcpy.
   --------------------------------------------------------------------------- */
int LegacyLootBridge::store(const Satchel<std::string>& satchel,
                            const char** dest, int dest_capacity)
{
    if (dest == nullptr || dest_capacity < satchel.size()) return -1;

    for (int i = 0; i < satchel.size(); ++i)
        dest[i] = satchel.at(i).c_str();

    return satchel.size();
}
