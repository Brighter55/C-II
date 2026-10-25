# TRIAGE.md

Build 1 — baseline and triage.

## 1. The three defects

### Defect 1 — armor applied twice

**How I found it:** I did ctest --preset course-debug -R encounter_resolves --output-on-failure in my terminal and it
printed unarmored: reported 19, HP removed 19 armored(3): reported 16, HP
removed 13, then failed with expected gap vs actual gap: 3 vs 6. The
sanitizer stayed quiet, which told me the memory was valid and the defect was
arithmetic.

**What was actually wrong:** resolve_attack_on_monster subtracted the target's
armor from the raw damage, then passed the already subtracted value to
apply_damage_to_monster, which subtracted armor again. Armor was removed twice
in one attack, and the number returned to the caller did not match the HP the
monster actually lost.

**What I changed:** resolve_attack_on_monster now passes raw damage to
apply_damage_to_monster and returns its result. resolve_attack_on_hero also got
the same fix. Armor is applied in exactly one place, inside the
apply_damage_to_* functions.

### Defect 2 — loot roll reads past the table

**How I found it:** I ran ctest -R loot_table_intact --output-on-failure, and it printed
2000 rolls, 147 landed outside the table. Under the sanitizer the same read is
a global-buffer-overflow on LOOT_TABLE[12].

**What was actually wrong:** dd_rand_range is inclusive on both ends
(lines 46-51). roll_loot called dd_rand_range(0, LOOT_COUNT) with
LOOT_COUNT equal to 12, so index 12 could be produced for an array indexed 0
through 11. About one roll in thirteen read the pointer just past the table and
handed back whatever was there.

**What I changed:** roll_loot now calls dd_rand_range(0, LOOT_COUNT - 1), so
the index stays inside the table.

### Defect 3 — iterator invalidated by a summon

**How I found it:** I did ctest -R roster_survives_reallocation and it crashed. Under AddressSanitizer it is a
heap-use-after-free in run_monster_round. The test builds a four-monster wave,
which is exactly full, and lets the summoner act first.

**What was actually wrong:** run_monster_round looped with a Wave::iterator
and called resolve_ability with the wave by reference. The ABIL_SUMMON case
calls wave.push_back (line 314). That push_back reallocates the vector and
frees the old storage, so the iterator pointed into freed memory and ++it was
undefined behavior.

**What I changed:** the loop uses a size_t index and re-reads wave.size()
each pass (line 432). A whelp summoned during a round can now take its turn in
that same round. That behavior is explicit instead of accidental, and the
ownership conversion below did not change the output.

## 2. Ownership table

| Allocation site | Who owned it before | Converted to | Why that choice |
|:--|:--|:--|:--|
| `make_hero` (line 119) | Nobody. `run_campaign` kept the pointer as `hero_owner` and never deleted it. | `std::unique_ptr<Hero>` (`HeroPtr`) | One owner for the campaign's lifetime; nothing shares it. |
| `make_monster` (line 137) | The `Bestiary` or `Wave` vector held the pointer, but nothing ever deleted an element. | `std::unique_ptr<Monster>` (`MonsterPtr`) | Each container is the sole owner of its own monsters; `spawn_from_bestiary` creates a new object rather than sharing one. |
| `make_merchant` (line 155) | Nobody. `yarrow_owner` was never deleted. | `std::unique_ptr<Merchant>` (`MerchantPtr`) | One owner, destroyed with the campaign. |
| `load_hero` (line 609) | Nobody on success; the bad-read path deleted the object by hand. | `std::unique_ptr<Hero>` (`HeroPtr`) | The smart pointer frees it on both paths, so the manual `delete` goes away. |


Non-owning:

- `Hero& hero` and `Merchant& yarrow` in `run_campaign` (lines 718-719), owned
  by `hero_owner` and `yarrow_owner`.
- `Monster& self` in `resolve_ability` (line 254), owned by the wave.
- `find_in_bestiary` returns a `Monster*` (line 360) borrowed from the bestiary.
- The `raw()` overloads in `dungeon_delve.h` (lines 85-87), which turn an owner
  into a plain pointer for the few places that need one.

A reader can tell an owner from an observer because every owner is a
`unique_ptr` alias and every observer is a reference or a raw pointer with a
comment saying it is non-owning.

## 3. The three worst things about this codebase

1. **Names are fixed-size arrays.** `char name[32]` and `char species[24]` are
   filled with `strncpy` at lines 120-121, 138-141, 156-157, and 615-616.
   Anything longer than 31 characters gets cut off, so two different monsters
   can end up looking the same, and the save file depends on the name. I would
   use `std::string`. That is Week 2.

2. **The structs repeat each other, and abilities are chosen by an int.**
   `Hero`, `Monster`, and `Merchant` share most of their fields but are written
   out three times in `dungeon_delve.h` (lines 34-65). `resolve_ability` picks
   behavior from a number (lines 254-354), and an unknown number quietly falls
   through to a basic attack. Adding a field means editing three places, and
   adding an ability means editing every switch. I would give the entities real
   types and attach the behavior to each type. That is Weeks 3 and 4.

3. **Every spawn scans the whole bestiary.** `find_in_bestiary` (lines 360-367)
   compares the species string against every entry, and `spawn_from_bestiary`
   calls it for every monster in every wave. Nine entries is nothing today, but
   the work grows with the content. I would build a lookup keyed by species
   once. That is Week 5, the same week the loot table moves to a standard
   container.

