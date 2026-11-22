// ===========================================================================
//  IS 267 — Concept Lab 5:  Generic and Non-Generic, Side by Side
//
//  satchel.h -- THE TEMPLATE SIDE.  You implement the four marked bodies.
//
//  A Satchel<T, Capacity> is a fixed-capacity, order-preserving container.
//  It is deliberately the same shape as the raw C array in legacy_loot.h --
//  a block of slots plus a count -- so that the only difference between the
//  two halves of this lab is that one of them is a class template and the
//  other is not. Any difference you report in ANALYSIS.md has to come from
//  that, because nothing else differs.
//
//  THE OPERATION is remove_at(): take out the entry at an index, close the
//  gap, keep the survivors in their original order, and keep the count
//  honest. You implement that same operation twice -- here for any T, and
//  in legacy_loot.cpp for a raw const char* array.
// ===========================================================================
#ifndef DD_LAB05_SATCHEL_H
#define DD_LAB05_SATCHEL_H

#include <array>

// ---------------------------------------------------------------------------
//  Satchel<T, Capacity>
//
//  Instantiate this for AT LEAST TWO different types. The lab supplies work
//  for two already: Satchel<std::string> holds loot names, Satchel<int, 12>
//  holds loot values. If you only ever instantiate it for one type you have
//  written a class, not a class template, and the acceptance criteria say so.
// ---------------------------------------------------------------------------
template <typename T, int Capacity = 16>
class Satchel {
    static_assert(Capacity > 0, "a satchel with no slots is not a satchel");

public:
    static constexpr int capacity = Capacity;

    Satchel() = default;

    // -----------------------------------------------------------------------
    // TODO: grow the satchel by one slot and return a pointer to that new
    // slot. Return nullptr, and change nothing, if the satchel is full.
    //
    // This is the primitive the other three are built on, and it is the one
    // the bridge in legacy_loot.cpp needs: it hands back the satchel's own
    // storage so a caller can build the new entry IN PLACE, instead of
    // building it somewhere else and then copying it in. That is what "moves
    // data between them without copying twice" means in practice, and it is
    // a thing you can only offer because the container is yours.
    //
    // The slot you hand back already holds a default-constructed T. The
    // caller assigns over it.
    // -----------------------------------------------------------------------
    T* add_slot() {
        if (full()) return nullptr;
        T* slot = &slots_[static_cast<std::size_t>(count_)];
        ++count_;
        return slot;
    }

    // Supplied, in terms of add_slot(). Once add_slot() is right, this is
    // right. Look at it before you write the bridge: it is the version that
    // DOES copy twice -- once into `value` at the call site, once into the
    // slot here -- and it is the convenient one, which is why it is the one
    // everybody reaches for.
    bool add(const T& value) {
        T* slot = add_slot();
        if (slot == nullptr) return false;
        *slot = value;
        return true;
    }

    // -----------------------------------------------------------------------
    // TODO: return how many entries the satchel currently holds -- not its
    // capacity. An empty satchel reports 0; a satchel with three entries
    // reports 3, however large Capacity is.
    // -----------------------------------------------------------------------
    int size() const {
        return count_;
    }

    // -----------------------------------------------------------------------
    // TODO: return a reference to the entry at `index`.
    //
    // Return by const reference, as declared. Do not change this to return
    // by value: the bridge hands out pointers into these strings, and a copy
    // would hand out a pointer into a dead temporary. The sanitizer would
    // catch that, eventually, somewhere else, in a week.
    //
    // `index` is caller-checked against size(). Assume it is in range.
    // -----------------------------------------------------------------------
    const T& at(int index) const {
        return slots_[static_cast<std::size_t>(index)];
    }

    // -----------------------------------------------------------------------
    // TODO: remove the entry at `index`, closing the gap.
    //
    //   - The entries after `index` shift down one slot, IN ORDER. Swapping
    //     the hole with the last entry is faster and is wrong here: a
    //     satchel is ordered and the tests check the order.
    //   - The count drops by one.
    //   - Return false and change nothing if `index` is out of range.
    //
    // This is THE operation. You implement it again, for a raw array, in
    // legacy_loot.cpp. Write this one first, then write that one, then
    // notice what you had to think about the second time that you did not
    // have to think about here. That noticing is the graded artifact.
    // -----------------------------------------------------------------------
    bool remove_at(int index) {
        if (index < 0 || index >= count_) return false;
        for (int i = index; i < count_ - 1; ++i)
            slots_[static_cast<std::size_t>(i)] = slots_[static_cast<std::size_t>(i + 1)];
        --count_;
        return true;
    }

    bool empty() const { return size() == 0; }
    bool full()  const { return size() == Capacity; }

private:
    std::array<T, Capacity> slots_{};
    int                     count_ = 0;
};

#endif // DD_LAB05_SATCHEL_H
