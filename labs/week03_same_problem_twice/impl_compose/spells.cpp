// ===========================================================================
//  IS 267 — Concept Lab 3:  Same Problem, Twice
//  VERSION B — COMPOSITION
//
//  A spell is a value that HAS a name, a cost, and an effect function. There
//  are no base classes and no virtual functions. The shared mana rule lives
//  in cast_spell(), the one place every spell passes through.
// ===========================================================================
#include "spellbook.h"

namespace {

using EffectFn = void (*)(Caster&, Target&, CastResult&);

struct Spell {
    const char* name;
    int         mana_cost;
    EffectFn    effect;
};

void fireball_effect(Caster&, Target& target, CastResult& out)
{
    int dealt = 9 - target.ward;
    if (dealt < 1) dealt = 1;
    target.hp -= dealt;
    if (target.hp < 0) target.hp = 0;
    out.damage_dealt = dealt;
}

void mend_effect(Caster& caster, Target&, CastResult& out)
{
    int room = caster.max_hp - caster.hp;
    if (room < 0) room = 0;
    int healed = room < 7 ? room : 7;
    caster.hp += healed;
    out.healing_done = healed;
}

void summon_effect(Caster& caster, Target& target, CastResult& out)
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

void hex_effect(Caster&, Target& target, CastResult& out)
{
    int room = target.strength - 1;
    int drained = room <= 0 ? 0 : (room < 2 ? room : 2);
    target.strength -= drained;
    out.strength_drained = drained;
}

// Order matches impl_inherit, so the two drivers print the same transcript.
const Spell SPELLS[] = {
    {"Fireball", 6, &fireball_effect},
    {"Mend",     4, &mend_effect},
    {"Summon",   8, &summon_effect},
    {"Hex",      3, &hex_effect},
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
    return SPELLS[index].name;
}

int spell_mana_cost(int index)
{
    if (index < 0 || index >= SPELL_COUNT)
        return 0;
    return SPELLS[index].mana_cost;
}

CastResult cast_spell(int index, Caster& caster, Target& target)
{
    if (index < 0 || index >= SPELL_COUNT)
        return CastResult{false, 0, 0, 0, 0, 0};

    const Spell& spell = SPELLS[index];
    CastResult out{false, 0, 0, 0, 0, 0};
    if (caster.mana < spell.mana_cost)
        return out;

    caster.mana -= spell.mana_cost;
    out.cast = true;
    out.mana_spent = spell.mana_cost;
    spell.effect(caster, target, out);
    return out;
}
