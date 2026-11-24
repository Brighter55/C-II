# CONTAINERS.md


## The access patterns

| Collection | How it is used | Container | Hot-operation cost |
|---|---|---|---|
| party roster | Appended when an entity joins; iterated to print the party; sorted into views for display; never looked up by key | `std::vector<std::unique_ptr<Entity>>` | append amortized O(1), iterate O(n), sort O(n log n) |
| bestiary | Looked up by species on every spawn, many times per wave; built once at startup | `std::map<std::string, Monster*>`, wrapped by `Registry<Monster*>` | find O(log n), insert O(log n), iterate O(n) |
| species already encountered | Membership test and insert on every spawn; duplicates collapse on insert | `std::set<std::string>` | insert and find O(log n), iterate O(n) |
| loot table | Indexed by position for display and random rolls; edited by hand; fixed order paired with `LOOT_VALUE` | raw `const char*[12]` | index O(1), find by name O(log n) through the index |

`vector` wins the roster because the pattern is append and iterate. Two sorted
views are built over it: one by hit points and one by name. The hit-point view
is supplied three ways. `ByHP` is a function object, `by_hp_desc` is a free
function used as a function pointer, and the name view is a lambda at the call
site. `Roster::sorted_by` takes any of them.

`std::map<std::string, Monster*>` wins the bestiary because the pattern is
lookup by species name. The old code scanned the vector and called `strcmp` on
every entry, which was O(n) per spawn. The map lookup is O(log n), and the
linear scan is gone.

`std::set<std::string>` wins the encounter log because the only question is
membership, and insertion deduplicates for free. A test records eleven
encounters with four repeats and asserts the set holds seven species.

`loot_value_of` used to scan the table with `strcmp`. It now builds a
`Registry<int>` from the table on first use and looks the value up in O(log n).
The raw table itself is untouched.

## Measurements

| Container | Insert (ms) | Find (ms) | Iterate (ms) |
|---|---:|---:|---:|
| `vector` | 0.56 | 1,358 | 0.04 |
| `list` | 4.27 | 22,673 | 0.41 |
| `set` | 20.39 | 19.72 | 2.49 |
| `map` | 41.51 | 26.45 | 3.21 |

The surprise was the `vector` and `list` find gap. Both are O(n), but the list
took about 17 times longer because each step follows a pointer to a different
cache line. Contiguity beat the same asymptotic cost.

The numbers confirmed the choices and did not change one. `vector` won insert
and iterate, which is why the roster stays a vector. `set` and `map` won find,
which is why the bestiary and the encounter log are ordered indexes. `list` lost
every column, so it appears nowhere.

## At a thousand times the size

The first choice I would revisit is the `std::map` index. Node-based storage
allocates per entry and has poor cache locality. For read-mostly lookups, a
sorted `std::vector` with binary search or an `unordered_map` may be faster at
100 million entries. I would measure all three on the target machine before
switching.
