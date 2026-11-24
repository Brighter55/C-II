// ===========================================================================
//  data_layer_tests.cpp -- the Build 5 acceptance checks.
//
//  Covers the three comparator forms, the species set's deduplication, the
//  indexed bestiary, the two-way loot bridge, and Pool acquisition.
// ===========================================================================
#include "dungeon_delve.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_failures = 0;

static void check(bool condition, const char* what)
{
    std::printf("  %s  %s\n", condition ? "ok  " : "FAIL", what);
    if (!condition) g_failures++;
}

// ---------------------------------------------------------------------------
//  Bestiary: std::map index keyed on species
// ---------------------------------------------------------------------------
static void test_bestiary_index()
{
    std::printf("\n[bestiary index]\n");

    Bestiary bestiary = build_bestiary();
    Monster* kobold = bestiary.find("kobold");

    check(kobold != nullptr, "kobold is found");
    check(kobold != nullptr &&
          std::strcmp(kobold->name().c_str(), "Kobold") == 0,
          "find returns the Kobold entry");
    check(bestiary.find("nope") == nullptr, "a missing species returns null");
    check(bestiary.count_species("kobold") == 1, "one kobold in the bestiary");
    check(bestiary.count_species("nope") == 0, "a missing species counts zero");
    check(bestiary.size() == 9, "the bestiary holds nine species");

    std::printf("> bestiary.find(\"kobold\")\n");
    if (kobold != nullptr)
        std::printf("  %-8s %2d HP   2d6   %-10s [O(log n)]\n",
                    kobold->name().c_str(), kobold->max_hp, "lootable");
}

// ---------------------------------------------------------------------------
//  Roster: two sorted views, three comparator forms
// ---------------------------------------------------------------------------
static void test_roster_views()
{
    std::printf("\n[roster sorted views]\n");

    Roster party;
    party.add(make_hero("Ash", 100, 10, 3));
    party.add(make_hero("Bree", 84, 9, 2));
    party.add(make_hero("Cato", 61, 8, 1));

    // Function object.
    auto by_hp_object = party.sorted_by(ByHP{});
    check(by_hp_object.size() == 3, "the ByHP view has three entries");
    check(by_hp_object[0]->name() == "Ash" &&
          by_hp_object[1]->name() == "Bree" &&
          by_hp_object[2]->name() == "Cato",
          "ByHP orders by hit points, highest first");

    // Function pointer.
    auto by_hp_function = party.sorted_by(&by_hp_desc);
    check(by_hp_function.size() == 3 &&
          by_hp_function[0] == by_hp_object[0] &&
          by_hp_function[2] == by_hp_object[2],
          "the function pointer agrees with ByHP");

    // Lambda.
    auto by_name = party.sorted_by([](const Entity* a, const Entity* b) {
        return a->name() < b->name();
    });
    check(by_name.size() == 3 &&
          by_name[0]->name() == "Ash" &&
          by_name[1]->name() == "Bree" &&
          by_name[2]->name() == "Cato",
          "the lambda orders by name");

    std::printf("> roster.sorted_by(ByHP{})\n  ");
    for (std::size_t i = 0; i < by_hp_object.size(); ++i) {
        if (i != 0) std::printf(" | ");
        std::printf("%s %d", by_hp_object[i]->name().c_str(),
                    entity_hp(*by_hp_object[i]));
    }
    std::printf("\n");
}

// ---------------------------------------------------------------------------
//  Species set: deduplication
// ---------------------------------------------------------------------------
static void test_species_set()
{
    std::printf("\n[species set]\n");

    reset_species_seen();
    const char* encounters[] = {
        "kobold", "kobold", "goblin", "goblin", "bat", "bat",
        "warg", "warg", "adept", "priest", "ogre"
    };
    const int encounter_count = 11;
    int duplicates = 0;

    for (const char* species : encounters)
        if (!record_species_seen(species)) duplicates++;

    check(species_seen_count() == 7,
          "seven unique species after eleven encounters");
    check(duplicates == 4, "four duplicate encounters were absorbed");
    check(species_was_seen("kobold"), "kobold is in the set");
    check(!species_was_seen("troll"), "troll is not in the set");

    std::printf("> species_seen.size()\n  %d   (%d encounters, %d duplicates absorbed)\n",
                species_seen_count(), encounter_count, duplicates);
}

// ---------------------------------------------------------------------------
//  Legacy bridge: both directions, pointer preservation
// ---------------------------------------------------------------------------
static void test_bridge()
{
    std::printf("\n[legacy bridge]\n");

    std::vector<const char*> table;
    for (int i = 0; i < loot_count(); ++i)
        table.push_back(loot_at(i));

    LegacyLootBridge::View view;
    const int loaded = LegacyLootBridge::load(
        table.data(), static_cast<int>(table.size()), view);

    check(loaded == loot_count(), "load appends every raw entry");
    check(view.in_use() == static_cast<std::size_t>(loot_count()),
          "the view holds every raw entry");
    check(view.at(0) != nullptr && *view.at(0) == "Rusted Dagger",
          "the first entry survived the load");

    std::vector<const char*> back(view.in_use());
    const int stored = LegacyLootBridge::store(
        view, back.data(), static_cast<int>(back.size()));

    check(stored == loot_count(), "store writes every view entry");
    bool same_buffers = true;
    for (std::size_t i = 0; i < view.in_use(); ++i)
        same_buffers = same_buffers && back[i] == view.at(i)->c_str();
    check(same_buffers, "store hands out the view's own buffers");

    LegacyLootBridge::View full;
    check(LegacyLootBridge::load(table.data(), loot_count(), full) == loot_count(),
          "first load fits the view");
    check(LegacyLootBridge::load(table.data(), loot_count(), full) == -1,
          "a second load is refused when the view cannot hold it");
}

// ---------------------------------------------------------------------------
//  Pool: acquisition and release
// ---------------------------------------------------------------------------
static void test_pool()
{
    std::printf("\n[pool]\n");

    Pool<int, 2> pool;
    int* first = pool.acquire();
    int* second = pool.acquire();

    check(first != nullptr && second != nullptr && first != second,
          "the pool hands out distinct slots");
    check(pool.acquire() == nullptr, "a full pool returns null");
    check(pool.at(0) == first && pool.at(1) == second,
          "the pool keeps acquisition order");
    check(pool.in_use() == 2, "the live count matches the acquisitions");

    pool.release(first);
    check(pool.in_use() == 1, "release lowers the live count");
    check(pool.acquire() == first, "release makes the same slot available");
}

int main()
{
    std::printf("IS 267 Build 5 data layer tests\n");

    test_bestiary_index();
    test_roster_views();
    test_species_set();
    test_bridge();
    test_pool();

    std::printf("\n%s\n", g_failures == 0 ? "All checks passed." : "FAILED");
    return g_failures == 0 ? 0 : 1;
}
