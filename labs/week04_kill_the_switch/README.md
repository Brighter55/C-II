# Concept Lab 4 — Kill the Switch

**Due Saturday.** Do this lab *before* Build 4, not after. The order is the
instruction: Build 4 removes the engine's `resolve_ability()` switch, and the
switch in this lab is that function cut down to a quarter of its size with
nothing else going wrong around it. Replace the small one on Saturday and you
spend Sunday on the engine instead of on the idea.

## What is here

| | |
|:--|:--|
| `kill_the_switch.h` | **The contract. Do not modify.** The six functions, and the exact behavior and RNG cost of every ability. |
| `lab_support.cpp` | **Do not modify.** The RNG, the damage arithmetic, the wave helper. |
| `kill_the_switch.cpp` | **Do not modify.** The driver. |
| `test_harness.h` | **Do not modify.** Seven lines of checking and the fixed scene. |
| `tests.cpp` | **Do not modify.** The behavior tests. These **pass right now**. |
| `variant_tests.cpp` | **Do not modify.** The Step 4 tests. These **fail right now**. |
| `impl/dispatch.cpp` | **The file you replace.** A working five-site type-tag dispatch. |
| `ANALYSIS.md` | Your line counts, your file counts, and the cost you name. **This is what you submit.** |

`impl/` is globbed by the build. You may delete `dispatch.cpp` and add as many
files as you like in there and never touch `CMakeLists.txt`. That is
deliberate: Step 4 asks how many existing files adding an ability costs you,
and the build system is not allowed to pad that number.

## How to build and run it

From the top of the repository:

```bash
cmake --preset course-debug
cmake --build --preset course-debug

./build/course-debug/kill_the_switch                # the driver
./build/course-debug/kill_the_switch_tests          # passes now, must keep passing
./build/course-debug/kill_the_switch_variant_tests  # fails now, must pass by Step 4
```

## Step 0 — establish the baseline. Do this first.

Before you change one character:

```bash
cd /path/to/dungeon-delve-seed
./build/course-debug/kill_the_switch_tests          # must say ALL TESTS PASSED
./build/course-debug/kill_the_switch > before.txt
wc -l labs/week04_kill_the_switch/impl/*            # the "before" line count
cp -r labs/week04_kill_the_switch /tmp/week04_before
```

Keep `before.txt`. When you are done, `diff before.txt after.txt` must print
nothing except the lines Shatter adds in Step 4.

A green test run you did not look at is not a baseline. If you skip Step 0 and
the tests fail later, you will not know whether you broke it or it was always
broken.

## The one rule

**Behavior does not change, and that includes the random numbers.**

`lab_support.cpp` counts every RNG draw, and the tests check the count. Two
implementations can agree on the damage for one call and then be different
games forever after, because one of them drew a number the other did not. The
usual way to earn that bug is to make `Bite`'s d100 roll conditional — it is
drawn whether or not it matters, and it has to stay that way. `kill_the_switch.h`
lists the draw count for every ability.

## What you are replacing, and with what

`impl/dispatch.cpp` dispatches on an integer tag in **five separate places**,
marked `SITE 1` through `SITE 5` in the file:

1. the `ALL_TAGS` roster table
2. the `ability_name()` switch
3. the `ability_mana_cost()` switch
4. the `ability_is_hostile()` switch
5. the `resolve_ability()` switch

Adding one ability means editing all five, and missing one still compiles.
Miss site 3 and the new ability is free to cast. Miss site 1 and it never
appears in the roster. That is the problem, not the line count.

Replace it with an **abstract base class and one concrete derived class per
ability**. When you are done, the acceptance criterion is:

> **Zero type inspection remains in the dispatch path.** No `switch` on a
> tag, no `if (tag == ABIL_...)`, no chain of comparisons deciding which
> behavior to run.

The tags themselves stay — they are how the engine and the tests address an
ability, and `kill_the_switch.h` does not change. Keeping a tag in the public
API is not the same thing as dispatching on it, and the second is what you
are removing. Looking an ability up by tag in one table, once, and then
calling a virtual function on what you found is not type inspection; it is a
lookup followed by dispatch.

## Step 4 — add an ability

`ABIL_SHATTER` is already in the header and nothing implements it.
`variant_tests.cpp` specifies it and currently fails. Add it **after** the
refactor is done and `kill_the_switch_tests` is green again.

**The target is zero edits to any file that already exists.** One new `.cpp`
in `impl/`, and nothing else. Measure it rather than claiming it:

```bash
diff -rq /tmp/week04_before labs/week04_kill_the_switch
```

- every `Only in ...` line is a file you **added**
- every `... differ` line is a file you **edited** — and each one of those is
  a point off the zero

If you cannot get to zero, say so in `ANALYSIS.md` and say exactly which file
you had to edit and why. An honest 2 with a reason is worth more than a 0 you
did not measure.

## How to measure the line counts

Raw lines, per file, before and after:

```bash
wc -l labs/week04_kill_the_switch/impl/*
```

Raw lines reward whoever wrote fewer comments, so also count code lines only
— every line that is neither blank nor a `//` comment:

```bash
for f in labs/week04_kill_the_switch/impl/*; do
  printf "%-60s %s\n" "$f" "$(grep -cv -e '^[[:space:]]*$' -e '^[[:space:]]*//' "$f")"
done
```

Report both, broken down by file, in the table in `ANALYSIS.md`. The total
will probably go **up**. That is a real result and it is one of the things
you are being asked to notice.
