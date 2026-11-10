# Concept Lab 4 — Kill the Switch

**Name:**  Piyawit Sriphrakhun    **Date:** 2026-11-10

---

## Step 0 — The baseline

Fill this in **before** you change anything. If this section is blank, nothing
below it is a measurement.

| | |
|:--|:--|
| `kill_the_switch_tests` final line, before I touched anything | `ALL TESTS PASSED  (237 checks)` |
| `kill_the_switch_variant_tests` final line, before | `13 of 22 checks FAILED` |
| Files in `impl/` before | 1 (`dispatch.cpp`) |
| `wc -l` total for `impl/` before | 215 |
| Code lines only (neither blank nor `//`) for `impl/` before | 152 |

---

## Step 1 — Line counts, broken down by file

### Before

| File | Raw lines | Code lines only |
|:--|:--:|:--:|
| `impl/dispatch.cpp` | 215 | 152 |
| **total** | **215** | **152** |

### After

Measured after the refactor and before Shatter was added.

| File | Raw lines | Code lines only |
|:--|:--:|:--:|
| `impl/abilities.h` | 50 | 24 |
| `impl/abilities.cpp` | 85 | 63 |
| `impl/flail.cpp` | 23 | 18 |
| `impl/cleave.cpp` | 27 | 22 |
| `impl/bite.cpp` | 30 | 25 |
| `impl/mend.cpp` | 26 | 21 |
| `impl/hex.cpp` | 25 | 20 |
| `impl/summon.cpp` | 30 | 24 |
| `impl/guard.cpp` | 25 | 20 |
| `impl/rally.cpp` | 29 | 24 |
| **total** | **350** | **261** |

**Did the total go up or down, and by how much?**

*My answer:* Up 135 raw lines and 109 code lines.

If it went up, do not apologise for it. Say what you got in exchange for the
extra lines, in one sentence, and make it a specific thing rather than
"better design".

*My answer:* Adding an ability went from five edits in one file to one new
file and zero edits to existing files.

---

## Step 2 — Behavior is unchanged. Prove it.

| | |
|:--|:--|
| `kill_the_switch_tests` final line, after the refactor | `ALL TESTS PASSED  (237 checks)` |
| `diff before.txt after.txt` on the driver output | empty |
| Did you modify `tests.cpp`, `test_harness.h` or `kill_the_switch.h`? | No |

**Any test that failed on the way, and what caused it.** This is the most
useful row on the page for next week, so write a real answer. "None" is
allowed only if it is true.

*My answer:* None.

---

## Step 3 — Zero type inspection in the dispatch path

**Where does an ability tag still appear in your code, and what is it doing
there?** List every place. A tag in a lookup is fine; a tag deciding what
happens next is not, and this is where you show which yours are.

*My answer:* A tag appears in three places, all lookups.

1. `AbilityRegistration` at the bottom of each ability's `.cpp` file stores
   that ability's tag in the registry. This is how the ability joins the
   roster.
2. `find_ability(tag)` in `abilities.cpp` turns a tag into an `Ability*`.
3. `ability_name`, `ability_mana_cost`, and `ability_is_hostile` take a tag,
   look up the object, and call its virtual method. They do not branch on the
   tag.


**Confirm, by searching rather than remembering:** no `switch` and no
comparison against an `ABIL_` constant decides behavior anywhere in `impl/`.
Paste what you searched and what it found.

```
$ grep -rn 'ABIL_' labs/week04_kill_the_switch/impl/
impl/bite.cpp:27:[[maybe_unused]] const AbilityRegistration kBiteRegistration(ABIL_BITE,
impl/cleave.cpp:24:[[maybe_unused]] const AbilityRegistration kCleaveRegistration(ABIL_CLEAVE,
impl/flail.cpp:20:[[maybe_unused]] const AbilityRegistration kFlailRegistration(ABIL_NONE,
impl/guard.cpp:22:[[maybe_unused]] const AbilityRegistration kGuardRegistration(ABIL_GUARD,
impl/hex.cpp:23:[[maybe_unused]] const AbilityRegistration kHexRegistration(ABIL_HEX, &kHex);
impl/mend.cpp:23:[[maybe_unused]] const AbilityRegistration kMendRegistration(ABIL_MEND,
impl/rally.cpp:26:[[maybe_unused]] const AbilityRegistration kRallyRegistration(ABIL_RALLY,
impl/shatter.cpp:24:[[maybe_unused]] const AbilityRegistration kShatterRegistration(ABIL_SHATTER,
impl/summon.cpp:27:[[maybe_unused]] const AbilityRegistration kSummonRegistration(ABIL_SUMMON,

I also searched for `switch`, `case ABIL_`, and `if` lines containing `ABIL_`.
None matched.
```

Every `ABIL_` hit is a registration lookup. None of them decides what happens
next.

---

## Step 4 — Add one ability

`ABIL_SHATTER`, specified in `kill_the_switch.h` and tested by
`variant_tests.cpp`.

| | |
|:--|:--|
| `kill_the_switch_variant_tests` final line | `ALL TESTS PASSED  (22 checks)` |
| `kill_the_switch_tests` still passing afterwards? | Yes, `ALL TESTS PASSED  (247 checks)` |
| Files I **added** | `impl/shatter.cpp` (27 raw lines, 22 code lines) |
| Existing files I **edited** | 0 |

Paste the `diff -rq /tmp/week04_before labs/week04_kill_the_switch` output you
counted from:

```
$ diff -rq /tmp/week04_before labs/week04_kill_the_switch
Only in labs/week04_kill_the_switch/impl: shatter.cpp
```

Here `/tmp/week04_before` is the snapshot taken after the refactor and before
Shatter was added. Against the true original snapshot, the refactor's added
files and the deleted `dispatch.cpp` also appear, but no supplied file
differs.

After Shatter, `diff before.txt after.txt` shows three changes: the roster
count line goes from 8 abilities to 9, a new Shatter section appears, and the
fizzle walk gains the Shatter line. No other driver line changes.

**If the number of existing files you edited is not zero, name each one and
say why.** An honest 2 with a reason scores better than a 0 you did not
measure.

*My answer:* Zero. Adding Shatter was one new `.cpp` and no edits.

**For comparison: how many of the five sites in the old `dispatch.cpp` would
Shatter have needed?** Count them from the `SITE 1`–`SITE 5` markers in the
original file.

*My answer:* All five: the roster table, the name switch, the cost switch, the
hostility switch, and the resolve switch.

---

## Step 5 — Name the thing that got worse. This is the graded artifact.

Something got worse. A submission claiming nothing got worse loses the
analysis marks — not because the claim is rude, but because it means the
comparison was not done.

It has to be a concrete cost in the new design: something that is now harder
to find, harder to read, slower, longer, more scattered, easier to get wrong,
or impossible to see in one place. Not "it took longer to write" and not "it
is less familiar."

*The thing that got worse:* The ability metadata is scattered. The old file
showed every name, cost, and hostility flag in three short switches, and all
eight costs fit in fourteen lines. Now a cost is a one-line method on a class
in its own file, and there is no single place to read the mana economy or
compare two abilities. The old file also showed every ability's whole effect
in one pass; now `resolve_ability()` shows only the fizzle rule and a virtual
call, and you have to know which file holds the ability to see what it does.

*Who pays that cost, and when:* Whoever balances or audits the ability set
pays it every time they compare costs or effects, and a new reader pays it
once when learning where behavior lives. A cross-ability rule is also harder
to see: the old switch made it obvious that every case lived under one shared
fizzle check.

---

**And the honest counterweight: what got better, stated as a measurement.**
Point at a row from Step 1 or Step 4, not at an adjective.

*My answer:* Adding Shatter cost one new file and zero edits to existing
files, against five edits in the old design. The behavior tests stayed at
`ALL TESTS PASSED`, first at 237 checks after the refactor and at 247 after
Shatter.

---

**Would you make this change to the engine's own `resolve_ability()` in Build
4?** Yes or no, and one reason drawn from something you measured in this lab.

*My answer:* Yes. The measured result is the edit count: one new file and
zero edits for a new ability, against five edits in the tag-switch design,
with every supplied behavior check still passing.

---

## What is being graded

| | |
|:--|:--|
| Step 0 filled in before anything changed | required |
| `kill_the_switch_tests` passes, unmodified, after the refactor | required |
| Driver output matches `before.txt` apart from Shatter | required |
| No `switch` or `ABIL_` comparison decides behavior in `impl/` | required |
| `kill_the_switch_variant_tests` passes | required |
| Existing files edited for Shatter, measured with `diff -rq` | required |
| Line counts are per file and came from a command | required |
| A genuine cost of the new design is named | **the graded artifact** — the line counts alone are not |

Submit this file, completed, as directed on the assignment page.
