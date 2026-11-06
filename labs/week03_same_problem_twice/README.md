# Concept Lab 3 — Same Problem, Twice

**Due Saturday.** Do this lab *before* Build 3, not after. The order is the
instruction: the lab isolates the inheritance-versus-composition choice in a
small program where nothing else is going wrong, so when Build 3 misbehaves
you are debugging one thing instead of two.

The domain is monster abilities — a Fireball, a Mend, a Summon, each with a
name, a mana cost, and an effect on a target. That is on purpose. Build 4
rebuilds exactly this, so whichever design you talk yourself into here is the
one you will be living with next week.

## What is here

| | |
|:--|:--|
| `spellbook.h` | **The contract. Do not modify.** The three functions you implement, and the exact behavior of every spell. |
| `lab_support.cpp` | **Do not modify.** `find_spell()`, written once for both versions. |
| `same_problem_twice.cpp` | **Do not modify.** The driver. Built twice; must print the same thing both times. |
| `tests.cpp` | **Do not modify.** The supplied tests. One file, linked against both of your implementations. |
| `impl_inherit/spells.cpp` | **Version A — yours to write.** An inheritance hierarchy. |
| `impl_compose/spells.cpp` | **Version B — yours to write.** Composition, no inheritance at all. |
| `ANALYSIS.md` | Your measurements and your recommendation. **This is what you submit.** |

You may add as many `.cpp` files as you like inside `impl_inherit/` and
`impl_compose/`. The build globs both folders, so you never edit
`CMakeLists.txt` — which matters, because one of the things you are measuring
is how many files a change costs you, and the build system should not be
padding that number.

## How to build and run it

From the top of the repository:

```bash
cmake --preset course-debug
cmake --build --preset course-debug

./build/course-debug/same_problem_twice_inherit         # version A, the driver
./build/course-debug/same_problem_twice_compose         # version B, the driver

./build/course-debug/same_problem_twice_inherit_tests   # version A, the tests
./build/course-debug/same_problem_twice_compose_tests   # version B, the tests
```

Both test runs must end in `ALL TESTS PASSED`. On the untouched scaffold both
fail — that is how you know the tests are checking something real.

The two drivers must also agree with each other:

```bash
./build/course-debug/same_problem_twice_inherit > a.txt
./build/course-debug/same_problem_twice_compose > b.txt
diff a.txt b.txt          # must print nothing
```

A non-empty `diff` means your two designs are not modelling the same domain
yet, and any measurement you take is comparing two different programs.
Register your spells in the same order in both folders.

## The one rule

**Both implementations satisfy the same `spellbook.h` and the same
`tests.cpp`, unchanged.** If a design would be easier with a different header,
that is a finding to write down in `ANALYSIS.md`, not a header to edit. The
whole value of the lab is that the tests cannot tell the two designs apart —
so every difference you measure is a difference in the code you had to write,
not in what the code does.

Two further rules, one per folder:

- `impl_inherit/` — inheritance does the dispatch. No `switch`, no `if`-chain,
  no tag deciding which effect runs.
- `impl_compose/` — **no base classes and no virtual functions.** Not one.

## How to measure, for real

The spec says the numbers must be real, not estimated. Here is how to get
each one.

### 1. Line count

Raw lines, per file and totalled:

```bash
cd /path/to/dungeon-delve-seed
wc -l labs/week03_same_problem_twice/impl_inherit/*.cpp \
      labs/week03_same_problem_twice/impl_compose/*.cpp
```

Raw lines reward whoever wrote fewer comments, so also count code lines only
— every line that is neither blank nor a `//` comment:

```bash
for f in labs/week03_same_problem_twice/impl_*/*.cpp; do
  printf "%-50s %s\n" "$f" "$(grep -cv -e '^[[:space:]]*$' -e '^[[:space:]]*//' "$f")"
done
```

Report both numbers. If they disagree about which design is smaller, say so —
that is a more interesting result than either number alone.

### 2. Files touched to add one new variant

The new variant is **Hex**, cost 3, specified at the bottom of the behavior
block in `spellbook.h`. Take a snapshot first, add Hex to **both**
implementations, then let `diff` tell you what you touched:

```bash
cp -r labs/week03_same_problem_twice /tmp/week03_before

# ... now add Hex to impl_inherit/ and to impl_compose/ ...

diff -rq /tmp/week03_before labs/week03_same_problem_twice
```

Every `files ... differ` line is a file you edited. Every `Only in
labs/...` line is a file you added. Count them separately, per design — an
edit to an existing file and a brand-new file are not the same cost, and the
difference is the point.

Re-run both test binaries after adding Hex. They must still pass: the
supplied tests ask for *at least* three spells, so a fourth spell is allowed
to exist but is not allowed to break anything.

### 3. Depth of the resulting structure

Count the longest chain of hops from the thing the dispatcher holds to the
code that actually performs the effect, counting both ends.

- For `impl_inherit/`: the number of classes in the longest `: public` chain,
  from the base the table holds a pointer to, down to the class whose
  `effect()` runs. A base plus one derived class is **2**.
- For `impl_compose/`: there is no inheritance chain, so this is **0**.
  Instead report the number of indirections — a `Spell` struct holding a
  function pointer the dispatcher calls through is **1**.

Report the number *and* which of the two definitions you used for each
column. Comparing a 2 against a 1 without saying what either counted is not a
measurement.
