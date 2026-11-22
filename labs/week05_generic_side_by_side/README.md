# Concept Lab 5 — Generic and Non-Generic, Side by Side

**Due Saturday.** Do this lab *before* Build 5, not after. The order is the
instruction: the lab isolates class templates in a small program where
nothing else is going wrong, so when Build 5's `LegacyLootBridge` misbehaves
you are debugging one thing instead of two.

## What is here

| | |
|:--|:--|
| `satchel.h` | **Yours.** The class template. Four `// TODO:` bodies. |
| `legacy_loot.h` | The raw table's declarations and the bridge's contract. Read it; do not edit it. |
| `legacy_loot.cpp` | Supplied above the line, **yours** below it: `raw_remove_at` and the two halves of `LegacyLootBridge`. |
| `generic_side_by_side.cpp` | The driver. Runs all three sides and prints them next to each other. You do not need to edit it. |
| `tests.cpp` | The supplied tests — the acceptance criteria as code. Do not edit. |
| `ANALYSIS.md` | Your report. **This is what you submit.** |

## How to build and run

From the top of the repository:

```bash
cmake --preset course-debug
cmake --build --preset course-debug
./build/course-debug/generic_side_by_side
./build/course-debug/generic_side_by_side_tests
```

The scaffold compiles and runs as delivered. It prints empty satchels and
`31 check(s) FAILED`, which is the correct starting state. Work the TODOs
until that line reads `All checks passed.`

Suggested order: `satchel.h` first (all four bodies), then `raw_remove_at`,
then `LegacyLootBridge::store`, then `LegacyLootBridge::load`.

## The one rule

**The bridge preserves the raw array. Deleting it fails the lab.**

`LOOT_TABLE` is the engine's loot table, lifted out of `src/dungeon_delve.cpp`
unchanged — twelve `const char*` pointing at string literals, a `#define` for
the count, and engine code that indexes it directly. It is the half of this
program you are not allowed to improve.

That means all of the following, not just the first:

- No `delete` or `delete[]` on `LOOT_TABLE` or on anything in it. Those are
  string literals in read-only storage; freeing them is undefined behaviour
  and the address sanitizer will say so loudly.
- No writing to the table. Not reordering it, not nulling a slot, not
  sorting it. `raw_remove_at` runs on a **working copy**, never on
  `LOOT_TABLE` itself.
- No replacing it with an equivalent. Rebuilding the table out of freshly
  allocated copies of the same twelve strings still fails, because
  `test_loot_table_preserved` compares the **pointer values**, not only the
  characters. The engine holds on to those pointers.

This is not a lab-flavoured restriction. It is the actual constraint Build 5
works under: the content team's table cannot change, so the adapter absorbs
the mismatch. An adapter that only works once you have been allowed to
rewrite the thing you were adapting to is not an adapter.

## What the tests check, and what they do not

The tests check: both instantiations, both implementations of the operation,
order preservation, the cleared tail on the raw side, the round trip, the
refusal cases, that `store()` hands out the satchel's own buffers rather than
copies, and that `LOOT_TABLE` came out the far side pointer-identical.

The tests **cannot** check the no-second-copy rule on the `load()` side —
whether the characters land in the satchel once or twice is not observable
from outside. That half is marked by reading your code, and Step 4 of
`ANALYSIS.md` asks you to name the line. Writing `out.add(slots[i])` in a
loop passes every test and copies twice; `add_slot()` exists so that you have
the alternative. Say which one you wrote.

## Scope

Small on purpose. This is not Build 5. One container, one operation, one
adapter, both directions. If you find yourself adding iterators, allocators,
or a second template parameter pack, you have left the lab.
