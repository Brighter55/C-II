# Concept Lab 3: Same Problem, Twice

Name: Peter Sriphrakhun    Date: 2026-11-06

## Step 1. Get both versions passing

| | Output |
|:--|:--|
| `./build/course-debug/same_problem_twice_inherit_tests` final line | `ALL TESTS PASSED  (90 checks)` |
| `./build/course-debug/same_problem_twice_compose_tests` final line | `ALL TESTS PASSED  (90 checks)` |
| `diff a.txt b.txt` of the two drivers | empty |

If the diff was not empty, say what differed and what you changed.

The diff was empty. Both folders register the spells in the same order: Fireball, Mend, Summon, Hex.

## Step 2. The measurement table

### Line count

| | Version A, inheritance | Version B, composition |
|:--|:--:|:--:|
| Files in the folder | 1 | 1 |
| Raw lines, `wc -l` total | 153 | 111 |
| Code lines only, not blank or `//` | 117 | 83 |

### Files touched to add Hex

| | Version A, inheritance | Version B, composition |
|:--|:--:|:--:|
| Existing files I edited | 1 | 1 |
| New files I added | 0 | 0 |
| Separate places I changed inside those files | 2 | 2 |

In both designs the two places were the spell itself and its registration line. In A that meant the `Hex` class and the `g_hex` entry in the table. In B it meant `hex_effect()` and the `{"Hex", 3, &hex_effect}` row.

Paste the `diff -rq` output you counted from.

```
Files /tmp/week03_before/impl_compose/spells.cpp and labs/week03_same_problem_twice/impl_compose/spells.cpp differ
Files /tmp/week03_before/impl_inherit/spells.cpp and labs/week03_same_problem_twice/impl_inherit/spells.cpp differ
```

### Depth of the structure

| | Version A, inheritance | Version B, composition |
|:--|:--:|:--:|
| Number reported | 2 | 0 inheritance, 1 indirection |
| What I counted to get it | Longest `: public` chain from the `Spell` base the table holds a pointer to, down to `Hex::effect()` that runs. Both ends counted. | There is no inheritance chain, so 0. The dispatcher calls through one function pointer in the `Spell` table, so 1 indirection. |

### Where the shared mana rule ended up

| | Where the mana rule lives |
|:--|:--|
| Version A, inheritance | `impl_inherit/spells.cpp`, in `Spell::cast()`. Every derived spell goes through it. |
| Version B, composition | `impl_compose/spells.cpp`, in `cast_spell()`. There is no base class, so the rule sits on the dispatcher. |

Both versions write the rule once. Neither pastes it into each spell.

## Step 3. What the numbers do not show

None of the numbers show how much the compiler helped. In A, a spell that forgot `name()`, `mana_cost()`, or `effect()` would not build. The mana rule could not be skipped because `cast()` calls it for every spell. In B you can leave a spell out of the table or point its entry at the wrong effect function, and everything still compiles until a test asks for that spell. A was more repetitive to write. Each spell needs a class, an override, and a registration line, which is where the extra 34 code lines came from.

## Step 4. The recommendation. This is the graded artifact.

Which one would you ship as the ability system for Build 4?

I would ship: [x] Version A, inheritance   [ ] Version B, composition

Why, in terms of the numbers above.

The line-count row argued against A. B is 34 code lines shorter, 83 against 117. The depth row also favors B. A is a two-class chain, while B has zero classes and one indirection. I still chose A because the files-touched row was a tie. Adding Hex cost one edited file and two places in both designs. B's smaller size did not make the next spell cheaper. A's base class gives the shared mana rule one place that cannot be skipped, and it makes a missing name, cost, or effect a compile error instead of a missing table row. I am paying 34 lines and one hop for enforcement, not for brevity.

Name something the design you rejected does better.

The entire spellbook is one table of four rows, so every spell is visible in one place. It has no vtable and no per-spell class code, and it is 34 code lines shorter and one hop shallower. It would also be the easier design to load at runtime from a data file, because the table is data and a class hierarchy is not.

One change that would flip your recommendation.

> I would ship the other design instead if the requirements changed so that abilities became data rather than behavior. For example, if designers had to add or retune spells from a data file without recompiling, the composition table is the thing you can load, and a class hierarchy is not.

## What is being graded

| | |
|:--|:--|
| Both versions pass the supplied tests, unchanged | required |
| The two drivers produce identical output | required |
| Every number in Step 2 came from a command, not an estimate | required |
| The `diff -rq` output is pasted in | required |
| The recommendation cites a row of the measurement table | required |
| Something the rejected design does better is named, concretely | required |
| Steps 3 and 4 together | the graded artifact, not the table alone |

Submit this file, completed, as directed on the assignment page.
