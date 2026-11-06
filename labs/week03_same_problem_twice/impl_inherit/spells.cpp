// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//  VERSION A — INHERITANCE
//
//  One abstract Spell base holds the shared mana rule. Each spell derives and
//  overrides name(), mana_cost() and effect(). Dispatch is virtual; there is
//  no tag and no switch choosing the effect.
// ===========================================================================
#include "spellbook.h"

namespace {

// The shared rule lives here, once. effect() only runs after the mana is paid.
class Spell {
public:
    virtual ~Spell() = default;

    virtual const char* name() const = 0;
    virtual int mana_cost() const = 0;

    CastResult cast(Caster& caster, Target& target) const
    {
        CastResult out{false, 0, 0, 0, 0, 0};
        if (caster.mana < mana_cost())
            return out;

        caster.mana -= mana_cost();
        out.cast = true;
        out.mana_spent = mana_cost();
        effect(caster, target, out);
        return out;
    }

protected:
    // What makes one spell different from another. Nothing else.
    virtual void effect(Caster& caster, Target& target,
                        CastResult& out) const = 0;
};

class Fireball : public Spell {
public:
    const char* name() const override { return "Fireball"; }
    int mana_cost() const override { return 6; }

protected:
    void effect(Caster&, Target& target, CastResult& out) const override
    {
        int dealt = 9 - target.ward;
        if (dealt < 1) dealt = 1;
        target.hp -= dealt;
        if (target.hp < 0) target.hp = 0;
        out.damage_dealt = dealt;
    }
};

class Mend : public Spell {
public:
    const char* name() const override { return "Mend"; }
    int mana_cost() const override { return 4; }

protected:
    void effect(Caster& caster, Target&, CastResult& out) const override
    {
        int room = caster.max_hp - caster.hp;
        if (room < 0) room = 0;
        int healed = room < 7 ? room : 7;
        caster.hp += healed;
        out.healing_done = healed;
    }
};

class Summon : public Spell {
public:
    const char* name() const override { return "Summon"; }
    int mana_cost() const override { return 8; }

protected:
    void effect(Caster& caster, Target& target, CastResult& out) const override
    {
        if (caster.summon_budget > 0) {
            --caster.summon_budget;
            ++caster.minions;
            out.minions_added = 1;
            return;
        }

        int dealt = 3 - target.ward;
        if (dealt < 1) dealt = 1;
        target.hp -= dealt;
        if (target.hp < 0) target.hp = 0;
        out.damage_dealt = dealt;
    }
};

class Hex : public Spell {
public:
    const char* name() const override { return "Hex"; }
    int mana_cost() const override { return 3; }

protected:
    void effect(Caster&, Target& target, CastResult& out) const override
    {
        int room = target.strength - 1;
        int drained = room <= 0 ? 0 : (room < 2 ? room : 2);
        target.strength -= drained;
        out.strength_drained = drained;
    }
};

// Static storage, so no new and no delete. Order matches impl_compose.
const Fireball g_fireball;
const Mend     g_mend;
const Summon   g_summon;
const Hex      g_hex;

const Spell* const SPELLS[] = {
    &g_fireball,
    &g_mend,
    &g_summon,
    &g_hex,
};

constexpr int SPELL_COUNT = sizeof(SPELLS) / sizeof(SPELLS[0]);

}  // namespace

// ---------------------------------------------------------------- the API

int spell_count()
{
    return SPELL_COUNT;
}

const char* spell_name(int index)
{
    if (index < 0 || index >= SPELL_COUNT)
        return nullptr;
    return SPELLS[index]->name();
}

int spell_mana_cost(int index)
{
    if (index < 0 || index >= SPELL_COUNT)
        return 0;
    return SPELLS[index]->mana_cost();
}

CastResult cast_spell(int index, Caster& caster, Target& target)
{
    if (index < 0 || index >= SPELL_COUNT)
        return CastResult{false, 0, 0, 0, 0, 0};
    return SPELLS[index]->cast(caster, target);
}
