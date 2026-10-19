# Dungeon Delve — Core Engine

**IS 267 · C++ Programming II · St. Louis Community College**

This is the codebase you are inheriting. Over the next seven weeks you will rebuild it.

It is about 800 lines in one file, behind a header somebody clearly added in a hurry. It allocates with `new` and never frees anything. It stores a hero's name in a `char[32]`. It decides what an ability does by switching on an integer. Three separate structs duplicate most of their fields. There is a comment in it that says "do NOT touch resolve_ability()."

**It also works.** Build it in Release and it plays through all six stages and prints a win. That is the situation you are actually being hired into: code that ships, that people depend on, and that nobody wants to touch.

---

## Get it building first

Nothing else in the course matters until this works. **Do it in the first three days.**

```bash
git clone <your-repository-url>
cd dungeon-delve-core

cmake --preset course-debug
cmake --build --preset course-debug
```

Then run it:

```bash
./build/course-debug/dungeon_delve
```

And run the tests:

```bash
ctest --preset course-debug --output-on-failure
```

**All three tests fail right now.** That is correct. They are the specification for Build 1.

### Requirements

| | |
|:--|:--|
| Compiler | GCC 12+, Clang 15+, or MSVC 2022. C++20 is the floor. |
| CMake | 3.21 or newer (presets need it) |
| Git | any recent version |

### If you are on Windows with Visual Studio

Use the MSVC preset instead:

```bash
cmake --preset course-debug-msvc
cmake --build --preset course-debug-msvc
```

The address sanitizer is not enabled in that configuration. You will be working without the tool that makes this week's defects easy to see, so either use WSL or the campus virtual desktop for Week 1, or expect to lean harder on the debugger. Ask if you are stuck on this — do not spend three days fighting it.

---

## The two halves of Build 1

### Half one — find and fix three defects

Three defects are seeded in this code. Each one has a test:

| Test | What it checks |
|:--|:--|
| `encounter_resolves` | Armor reduces damage by exactly its own value, once |
| `loot_table_intact` | Every loot drop comes from inside the loot table |
| `roster_survives_reallocation` | Monsters that act are really in the wave |

Run one at a time while you work:

```bash
ctest --preset course-debug -R encounter_resolves --output-on-failure
```

Two of the three are memory problems and the sanitizer will point straight at them. One is not — it is a logic error, the tests will tell you the numbers are wrong, and you will have to reason about why.

**The sanitizer stops at the first problem it finds.** Fix it, rebuild, run again. Expect several rounds. That is not you being slow; that is debugging.

### Half two — make the ownership honest

Run the game with leak detection on:

```bash
ASAN_OPTIONS=detect_leaks=1 ./build/course-debug/dungeon_delve --quiet
```

You will get a wall of leak reports. Every `new` in this file is owned by nobody.

**Start in `include/dungeon_delve.h`.** The ownership model of this entire engine is three lines:

```cpp
using MonsterPtr  = Monster*;
using HeroPtr     = Hero*;
using MerchantPtr = Merchant*;
```

The rest of the engine, and these tests, are written in terms of those aliases and of
references. Change an alias to a smart pointer and the whole program follows. You should not
need to rewrite function signatures to make the conversion compile — if you find yourself
doing that, stop and look at `raw()` in the header first.

Your job:

- Replace every **owning** raw pointer with `std::unique_ptr` or `std::shared_ptr`.
- Justify every `std::shared_ptr` in a one-line comment. If `unique_ptr` "wouldn't compile," that error was telling you something true about the design — read it before reaching for `shared_ptr`.
- Leave **non-owning** observation as a reference or a raw pointer, and mark each one with a comment saying it is non-owning.
- The program's output must not change.

When you are done, that same command reports no leaks and no invalid accesses.

### On GCC, turn on one more check

GCC detects reads of a dead stack frame only when you ask it to:

```bash
ASAN_OPTIONS=detect_leaks=1:detect_stack_use_after_return=1 ./build/course-debug/dungeon_delve
```

Clang has this on automatically. Either way, **read your compiler warnings** — they are enabled for a reason and one of this week's problems shows up in a warning before you ever run the program.

---

## What to hand in

`TRIAGE.md`, at the repository root:

1. **For each defect:** how you found it, what the actual cause was, and what you changed. The diagnosis is worth more than the fix. "Changed line 412" earns almost nothing. "The loop held an iterator across a `push_back`, so the vector reallocated and the iterator pointed at freed memory" earns full marks.
2. **An ownership table:** every heap allocation in the original, who owned it, and what you converted it to.
3. **The three worst things** about this codebase, with line numbers, and what you would do about each.

Then tag it:

```bash
git add -A
git commit -m "Build 1: baseline and triage"
git tag build-01
git push origin main --tags
```

Submit the tag name in Canvas.

---

## What you may *not* do this week

**No new classes. No inheritance. No templates. No new containers.** Fix and convert only.

That restriction is real and it is graded. The architecture work starts in Week 2, and it starts from code you understand. Refactoring a codebase you have not yet diagnosed is how small problems become large ones — and it makes three findable defects very hard to find.

Keep a notes file. You will see a dozen things that want fixing. Most of them are a later week's build:

| You noticed | That is |
|:--|:--|
| Damage and gold are both bare `int` | Week 2 |
| `Hero`, `Monster`, `Merchant` duplicate each other | Week 3 |
| `resolve_ability()` is a giant `switch` on a type tag | Week 4 |
| The bestiary is searched with a loop every time | Week 5 |
| `char name[32]` instead of `std::string` | Week 2 |
| There is no inventory, undo, or turn queue | Week 6 |
| The header declares everything but groups nothing | unassigned — fair game for TRIAGE.md |
| `printf` is interleaved with the combat logic | Week 7 |
| The loot table is a raw C array | Week 5 — and it stays |

---

## Running it

```bash
./build/course-debug/dungeon_delve                 # full play-by-play
./build/course-debug/dungeon_delve --quiet         # just the result
./build/course-debug/dungeon_delve --seed 4242     # a different run
```

The RNG is deterministic for a given seed, so a bug you can reproduce once you can reproduce every time. Use that.

> There is a TODO in the source that says *"the loot thing is weird sometimes, look at it."* The original author was right. Try a few seeds.

---

## Layout

```
dungeon-delve-core/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── include/
│   └── dungeon_delve.h            structs, the ownership aliases, declarations
├── src/
│   └── dungeon_delve.cpp          the whole engine, in one file
├── tests/
│   └── regression.cpp             the three Build 1 tests
└── labs/
    └── week01_ownership_audit/
        ├── ownership_audit.cpp    Concept Lab 1
        └── expected_output.txt    what it should print once fixed
```

Concept Lab 1 builds from the same presets:

```bash
./build/course-debug/ownership_audit
```

---

## If you are stuck

Email your instructor **the day it goes wrong**, not the Sunday after. In an eight-week course a lost week is a tenth of the term.

Environment problems in Week 1 are normal, they are fixable, and they are only a crisis if they are discovered late.
