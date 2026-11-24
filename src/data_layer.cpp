// ===========================================================================
//  data_layer.cpp -- the non-template half of the Week 5 data layer:
//  Bestiary storage and index, the legacy loot bridge, and the species set.
// ===========================================================================
#include "dungeon_delve.h"
#include "data_layer.h"

#include <set>
#include <string>

// ---------------------------------------------------------------------------
//  Bestiary
// ---------------------------------------------------------------------------

void Bestiary::push_back(MonsterPtr monster)
{
    Monster* source = raw(monster);
    if (source == nullptr) return;

    Monster* slot = monsters_.acquire();
    if (slot == nullptr) return;

    *slot = *source;
    by_species_.add(std::string(slot->species), slot);
}

std::size_t Bestiary::size() const noexcept
{
    return monsters_.in_use();
}

const Monster* Bestiary::operator[](std::size_t index) const
{
    return monsters_.at(index);
}

Monster* Bestiary::find(const char* species) const
{
    if (species == nullptr) return nullptr;
    Monster* const* found = by_species_.find(std::string(species));
    return found == nullptr ? nullptr : *found;
}

int Bestiary::count_species(const char* species) const
{
    if (species == nullptr) return 0;
    return static_cast<int>(by_species_.count(std::string(species)));
}

// ---------------------------------------------------------------------------
//  Roster comparators
// ---------------------------------------------------------------------------

int entity_hp(const Entity& entity)
{
    const auto* combatant = dynamic_cast<const Combatant*>(&entity);
    return combatant != nullptr ? combatant->hp : 0;
}

bool by_hp_desc(const Entity* a, const Entity* b)
{
    return entity_hp(*a) > entity_hp(*b);
}

// ---------------------------------------------------------------------------
//  LegacyLootBridge
// ---------------------------------------------------------------------------

int LegacyLootBridge::load(const char* const* slots, int count, View& out)
{
    if (slots == nullptr || count < 0) return -1;
    if (static_cast<std::size_t>(count) > out.capacity() - out.in_use())
        return -1;

    for (int i = 0; i < count; ++i) {
        std::string* slot = out.acquire();
        *slot = slots[i];
    }
    return count;
}

int LegacyLootBridge::store(const View& source, const char** dest,
                            int dest_capacity)
{
    if (dest == nullptr ||
        dest_capacity < static_cast<int>(source.in_use()))
        return -1;

    for (std::size_t i = 0; i < source.in_use(); ++i)
        dest[i] = source.at(i)->c_str();
    return static_cast<int>(source.in_use());
}

// ---------------------------------------------------------------------------
//  Encountered species
// ---------------------------------------------------------------------------

namespace {
std::set<std::string>& seen()
{
    static std::set<std::string> instance;
    return instance;
}
} // namespace

void reset_species_seen()
{
    seen().clear();
}

bool record_species_seen(const char* species)
{
    if (species == nullptr) return false;
    return seen().insert(std::string(species)).second;
}

bool species_was_seen(const char* species)
{
    if (species == nullptr) return false;
    return seen().count(std::string(species)) != 0;
}

int species_seen_count()
{
    return static_cast<int>(seen().size());
}
