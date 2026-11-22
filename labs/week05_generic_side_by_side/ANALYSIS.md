# Concept Lab 5: Generic and Non-Generic, Side by Side

**Name:** Piyawit Sriphrakhun    **Date:** 2026-11-21

---

## Step 1: Get both sides working

**Test result:** 0 check(s) failed.

**The two types I instantiated `Satchel<T>` for:**

| | Type | Where it is used |
|:--|:--|:--|
| 1 | `std::string` | Loot names: the bridge destination in the driver (`generic_side_by_side.cpp:56-64`) and the template tests (`tests.cpp:60-72`). |
| 2 | `int` | Loot values: `Satchel<int, 12>` in the driver (`generic_side_by_side.cpp:70-79`) and the template tests (`tests.cpp:74-80`). |

---

## Step 2: What the template version made easier

| | What it made easier | The concrete evidence |
|:--|:--|:--|
| 1 | Two arguments instead of three, so the count cannot get separated from the slots. | `Satchel<T>::remove_at(int index)` at `satchel.h:110` against `raw_remove_at(const char** slots, int* count, int index)` at `legacy_loot.cpp:76`. The satchel's slots and count are members of one object, so I cannot pass a count that belongs to a different array. The raw signature accepts exactly that mistake and the compiler cannot tell the two variables apart. |
| 2 | The capacity check lives in the type, so I could not forget it. | `add_slot()` tests `full()` at `satchel.h:55` and `Capacity` is a template parameter. The raw signature has no capacity argument at all, and `legacy_loot.h:48-52` says the capacity is not knowable from the arguments, so `raw_remove_at` can only bound-check against the count it was handed. I wrote two null checks and an index check in the raw body; the template body has one index check. |
| 3 | The second instantiation cost zero implementation lines. | `Satchel<int, 12>` reuses the same four bodies in the driver and the tests. A raw version for ints would be a second function with an `int**` signature, or a `void*` cast that gives up the type. The same body would also hold `Monster*` without an edit; `raw_remove_at` accepts only `const char**`. |

---

## Step 3: What the raw version made easier

| | What it made easier | The concrete evidence |
|:--|:--|:--|
| 1 | Direct indexing of two parallel arrays is one expression. | `print_loot_table()` at `legacy_loot.cpp:57` reads `LOOT_TABLE[i]` and `LOOT_VALUE[i]` on one line. Through a satchel it becomes `names.at(i).c_str()` plus a second satchel for the values, and nothing in `Satchel` ties `names.at(i)` to `values.at(i)`. The pairing is the content format, and the raw version keeps it visible. |
| 2 | The content team can edit it as text, with no capacity decision. | Adding an entry means editing `#define LOOT_COUNT 12` and the two initializers at `legacy_loot.cpp:23-40`. The satchel's capacity is a template argument fixed at compile time, and the data has to be loaded at runtime through `add()` or the bridge. The content team cannot change game content without a code change and a rebuild. |
| 3 | It is smaller and allocates nothing. | `LOOT_TABLE` is 12 pointers, 96 bytes on this 64-bit build, plus 170 bytes of literal text (158 characters and 12 terminators), so 266 bytes. `Satchel<std::string>` measured 520 bytes when empty (16 slots of 32-byte `std::string` plus the count), and three names longer than the 15-character small-string buffer allocate on the heap. The raw table needs no heap and no constructor order. |

---

## Step 4: The bridge

**Where in your `load()` is the text copied, and how many times?**

The copy is at `legacy_loot.cpp:110`, `*slot = slots[i];`, and it happens once. `add_slot()` at line 109 returns a pointer to the default-constructed string already in the slot, and assigning the `const char*` constructs the text in place. I did not use `add()`, and I did not stage the entries in a vector or a second satchel.

**`store()` hands out pointers into the satchel's own strings. Name one way a caller could get a dangling pointer out of that, and the rule that prevents it.**

If the caller keeps `dest` after the satchel is destroyed, or calls `remove_at` and then reads `dest`, the pointers refer to freed storage or to strings that have been shifted and possibly reallocated. The rule is that the satchel must outlive every use of the pointers, and no add or remove may run while they are in use. The raw side needs the same rule, which is why `raw_remove_at` clears the tail slot.

**The bridge is not allowed to touch `LOOT_TABLE`. If that constraint were lifted, name one thing you would still not change, and why.**

I would keep the index pairing between `LOOT_TABLE` and `LOOT_VALUE`. The engine reads both with the same `i`, in `print_loot_table()` at `legacy_loot.cpp:57` and in `loot_at()`. If I rewrote the storage into a map, entry i's name and value would still have to travel together. Sorting by name or dropping `LOOT_VALUE` would silently pair a name with the wrong value.

---

## Step 5: The closing question

Build 5 gets the template version. New engine code calls `Satchel<T>` because the object carries its own count and capacity and works for any element type. `raw_remove_at` keeps its job on the legacy path: any code that holds a `const char*` working copy of the table and needs to drop an entry uses it, because `LOOT_TABLE` itself stays untouched. The table also stays the source that the bridge reads and that `print_loot_table()` and `loot_at()` index directly. The split is right because the constraint is institutional, not technical. The content team edits text, the engine indexes the array, and neither can move this week. New code should not inherit the raw shape's hand-carried count.
