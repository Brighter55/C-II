// ===========================================================================
//  data_layer.h -- the Week 5 data layer.
//
//  Three pieces, each chosen from an access pattern:
//    Registry<T>  a keyed index, ordered, O(log n) find
//    Pool<T>      fixed-capacity storage with stable addresses, O(1) acquire
//    LegacyLootBridge  a two-way adapter around the raw C array
//
//  Comparators come three ways. BySpecies and ByHP are function objects.
//  by_hp_desc is a free function used as a function pointer. The sorted-by-
//  name view uses a lambda at the call site.
// ===========================================================================
#ifndef DD_DATA_LAYER_H
#define DD_DATA_LAYER_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

class Entity;
class Monster;

// Function object: the bestiary index ordering.
struct BySpecies {
    bool operator()(const std::string& a, const std::string& b) const
    {
        return a < b;
    }
};

// Hit points through the Entity interface. Non-combatants report 0.
int entity_hp(const Entity& entity);

// Free function: the roster comparator, usable as a function pointer.
bool by_hp_desc(const Entity* a, const Entity* b);

// Function object: the same ordering, supplied as an object.
struct ByHP {
    bool operator()(const Entity* a, const Entity* b) const
    {
        return by_hp_desc(a, b);
    }
};

// ---------------------------------------------------------------------------
//  Registry<T, Key, Compare>
//
//  A keyed index. Ordered, so find, insert, and count are O(log n). One value
//  per key; adding a key that is already present keeps the first value.
// ---------------------------------------------------------------------------
template <typename T, typename Key = std::string,
          typename Compare = std::less<Key>>
class Registry {
public:
    using Map = std::map<Key, T, Compare>;

    void add(const Key& key, T value)
    {
        items_.emplace(key, value);
    }

    // Replace the entry for key. Used where a tag must map to one object.
    void set(const Key& key, T value)
    {
        items_.erase(key);
        items_.emplace(key, value);
    }

    const T* find(const Key& key) const
    {
        const auto it = items_.find(key);
        return it == items_.end() ? nullptr : &it->second;
    }

    T* find(const Key& key)
    {
        auto it = items_.find(key);
        return it == items_.end() ? nullptr : &it->second;
    }

    bool contains(const Key& key) const { return find(key) != nullptr; }
    std::size_t count(const Key& key) const { return items_.count(key); }
    std::size_t size() const { return items_.size(); }
    bool empty() const { return items_.empty(); }

    typename Map::const_iterator begin() const { return items_.begin(); }
    typename Map::const_iterator end() const { return items_.end(); }

private:
    Map items_;
};

// ---------------------------------------------------------------------------
//  Pool<T, Capacity>
//
//  Fixed-capacity storage. All slots are constructed once and never move, so
//  a pointer handed out by acquire() stays valid until release(). acquire and
//  release are O(1) for the free list; at() walks the acquisition order.
// ---------------------------------------------------------------------------
template <typename T, std::size_t Capacity = 128>
class Pool {
    static_assert(Capacity > 0, "a pool with no slots is not a pool");

public:
    Pool() : slots_(Capacity), free_(Capacity)
    {
        for (std::size_t i = 0; i < Capacity; ++i)
            free_[i] = Capacity - 1 - i;
    }

    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool(Pool&&) = default;
    Pool& operator=(Pool&&) = default;

    T* acquire()
    {
        if (free_.empty()) return nullptr;
        const std::size_t index = free_.back();
        free_.pop_back();
        T* slot = &slots_[index];
        order_.push_back(slot);
        return slot;
    }

    void release(T* slot)
    {
        if (slot == nullptr) return;
        const T* base = slots_.data();
        if (slot < base || slot >= base + slots_.size()) return;

        const auto it = std::find(order_.begin(), order_.end(), slot);
        if (it != order_.end()) order_.erase(it);
        free_.push_back(static_cast<std::size_t>(slot - base));
    }

    T* at(std::size_t index) { return order_.at(index); }
    const T* at(std::size_t index) const { return order_.at(index); }

    std::size_t in_use() const { return order_.size(); }
    std::size_t capacity() const { return Capacity; }

private:
    std::vector<T> slots_;
    std::vector<std::size_t> free_;
    std::vector<T*> order_;
};

// ---------------------------------------------------------------------------
//  LegacyLootBridge
//
//  The raw table is the content team's file and it stays where it is. The
//  templated side is a Pool of strings, so the store direction can hand back
//  the strings' own buffers with no allocation and no copy.
// ---------------------------------------------------------------------------
struct LegacyLootBridge {
    using View = Pool<std::string, 16>;

    static int load(const char* const* slots, int count, View& out);
    static int store(const View& source, const char** dest, int dest_capacity);
};

// The set of species the campaign has already encountered.
void reset_species_seen();
bool record_species_seen(const char* species);
bool species_was_seen(const char* species);
int  species_seen_count();

#endif // DD_DATA_LAYER_H
