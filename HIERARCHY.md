# The Entity Hierarchy

## Diagram

```
Describable
|-- virtual -- Entity
|               |-- Hero
|               |-- Monster
|               `-- Merchant
|-- virtual -- Combatant
|               |-- Hero
|               `-- Monster
`-- virtual -- Lootable
                |-- Monster
                `-- Merchant
```

Concrete types:

- Hero = Entity + Combatant
- Monster = Entity + Combatant + Lootable
- Merchant = Entity + Lootable

Entity, Combatant, and Lootable each inherit Describable virtually. Every
entity therefore has one Describable subobject, not one per path.

## The most debatable decision

Hit points, strength, and armor live in Combatant, not Entity, even though
Entity is the base all three entities share. Merchant is an Entity and has no
hit points. Putting combat state in Entity would give Merchant fields its own
code can never use, and it would let `apply_damage(Entity&, int)` compile for
a type that cannot take damage. The cost is that a reader must look in two
places to see a Monster's full state, and Hero and Monster have to inherit a
mixin to get the fields they share. I would pay that cost because it keeps the
Entity interface honest and keeps a Merchant out of combat code.

## One sentence per inheritance edge

- Hero inherits Entity: `describe_entity(const Entity&)` returns the `[Hero]` line for a Hero.
- Monster inherits Entity: `describe_entity(const Entity&)` returns the `[Monster]` line for a Monster.
- Merchant inherits Entity: `describe_entity(const Entity&)` returns the `[Merchant]` line for a Merchant.
- Hero inherits Combatant: `apply_damage(Combatant&, int)` subtracts armor and HP from a Hero.
- Monster inherits Combatant: `apply_damage(Combatant&, int)` subtracts armor and HP from a Monster.
- Monster inherits Lootable: `loot_gold_of(const Lootable&)` returns a Monster's `gold_drop`.
- Merchant inherits Lootable: `loot_gold_of(const Lootable&)` returns a Merchant's `gold`.
- Entity inherits Describable virtually: `describe_object(const Describable&)` returns an Entity's description.
- Combatant inherits Describable virtually: `describe_object(const Describable&)` returns the final overrider's text for any Combatant subobject of a concrete entity.
- Lootable inherits Describable virtually: `describe_object(const Describable&)` returns the final overrider's text for any Lootable subobject of a concrete entity.
