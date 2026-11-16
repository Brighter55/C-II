# Polymorphism — Build 4

## The resolver, before and after

| | Old | New |
|:--|:--|:--|
| Where | `src/dungeon_delve.cpp` | `src/abilities/registry.cpp` |
| Lines | 101 | 9 code lines (12 with comments) |
| Decides by | `switch` on the integer `tag` | virtual call through `Ability&` |
| Type inspection | 8 `case` labels | none |
| Adding an ability | 3 edits in 2 files | 1 new file, 0 edits |

The old `ability_name()` switch was 14 lines and had the same 8 cases. It is
gone too. `ability_name()` now looks up the ability and returns
`ability->name()`.

`describe()` is the virtual function overridden at two levels. It is pure in
`Ability`, overridden in `HostileAbility`, and overridden again in `Cleave`.
Neither derived class writes `virtual`, which is the point: virtuality is
inherited once the base declares it.

### Old resolver

```cpp
int resolve_ability(int tag, Monster& self, Hero& hero,
                    Wave& wave, int verbose)
{
    int amount = 0;

    switch (tag) {

        case ABIL_NONE:
            amount = resolve_attack_on_hero(self, hero);
            if (verbose)
                std::printf("  %s flails .................. %d damage\n",
                            self.name().c_str(), amount);
            break;

        case ABIL_CLEAVE:
            amount = resolve_attack_on_hero(self, hero);
            amount = amount + (amount / 2);
            apply_damage_to_hero(hero, amount / 2);
            if (verbose)
                std::printf("  %s cleaves ................. %d damage\n",
                            self.name().c_str(), amount);
            break;

        case ABIL_BITE:
            amount = resolve_attack_on_hero(self, hero);
            if (dd_rand_range(1, 100) > 70) {
                apply_damage_to_hero(hero, 2);
                amount += 2;
                if (verbose)
                    std::printf("  %s bites deep ............. %d damage\n",
                                self.name().c_str(), amount);
            } else {
                if (verbose)
                    std::printf("  %s bites ................... %d damage\n",
                                self.name().c_str(), amount);
            }
            break;

        case ABIL_MEND:
            amount = roll_dice(2, 4);
            self.hp += amount;
            if (self.hp > self.max_hp) self.hp = self.max_hp;
            if (verbose)
                std::printf("  %s mends ................... +%d HP\n",
                            self.name().c_str(), amount);
            break;

        case ABIL_HEX:
            amount = roll_dice(1, 4);
            hero.strength -= amount;
            if (hero.strength < 1) hero.strength = 1;
            if (verbose)
                std::printf("  %s hexes ................... -%d STR\n",
                            self.name().c_str(), amount);
            break;

        case ABIL_SUMMON:
            if (self.summon_budget > 0) {
                self.summon_budget--;
                MonsterPtr pup = make_monster("Whelp", "whelp", 6, 2, 0, 3, ABIL_NONE);
                wave.push_back(std::move(pup));
                amount = 1;
                if (verbose)
                    std::printf("  %s summons a Whelp!\n", self.name().c_str());
            } else {
                amount = resolve_attack_on_hero(self, hero);
                if (verbose)
                    std::printf("  %s is out of whelps ....... %d damage\n",
                                self.name().c_str(), amount);
            }
            break;

        case ABIL_GUARD:
            self.armor += 2;
            amount = 2;
            if (verbose)
                std::printf("  %s guards .................. +2 armor\n",
                            self.name().c_str());
            break;

        case ABIL_RALLY:
            for (size_t i = 0; i < wave.size(); i++) {
                if (!wave[i]->is_dead && raw(wave[i]) != &self)
                    wave[i]->strength += 1;
            }
            amount = 1;
            if (verbose)
                std::printf("  %s rallies the pack ........ +1 STR each\n",
                            self.name().c_str());
            break;

        default:
            amount = resolve_attack_on_hero(self, hero);
            if (verbose)
                std::printf("  %s does something ......... %d damage\n",
                            self.name().c_str(), amount);
            break;
    }

    return amount;
}
```

### New resolver

```cpp
int resolve_ability(const Ability& ability, Monster& self, Hero& hero,
                    Wave& wave, int verbose)
{
    try {
        return ability.apply(self, hero, wave, verbose);
    } catch (const AbilityError& e) {
        throw AbilityFailure(ability.name(), self.name(), e.what());
    }
}
```

The tag overload does the lookup and the fallback, then calls the function
above:

```cpp
int resolve_ability(int tag, Monster& self, Hero& hero, Wave& wave,
                    int verbose)
{
    const Ability* ability = ability_for_tag(tag);
    if (ability == nullptr) ability = default_ability();
    return resolve_ability(*ability, self, hero, wave, verbose);
}
```

## What got worse

The ability code went from 115 lines in two switches in one file to 388 lines
across ten files, and no single place shows the whole system anymore. The old switch let
a reader see every ability's name, effect, and print line in one pass; now the
metadata and the behavior are on the class in its own file, and finding one
ability means knowing which file holds it. The resolver also stopped being a
description of the game: it shows a lookup and a virtual call, not what Cleave
or Mend does. There is a new startup-order coupling as well, because each
ability file registers itself through a static object, and the registry has to
be ready before the first `Monster` is constructed. The exception path is a new
flow that the old code did not have, and it is only worth its cost because the
failure is now named instead of being a case the old switch had no answer for.

## The open/closed test

I added `src/abilities/shatter.cpp` after the refactor was green. It is 38 raw
lines and 28 code lines. The measurement:

```
$ diff -rq before_shatter/src src
Only in src/abilities: shatter.cpp

$ diff -rq before_shatter/include include
(no output)

$ diff -q before_shatter/CMakeLists.txt CMakeLists.txt
(no output)
```

Files added: one. Existing files edited: zero. The new file defines a class
derived from `HostileAbility` and one `AbilityRegistration` object. The tag is
already reserved in `dungeon_delve.h`, and `CMakeLists.txt` globs
`src/abilities/*.cpp`, so the resolver learns about the ability through the
registry without an edit.

The old design would have needed three edits for the same ability: one
`#define` in the header, one `case` in `ability_name()`, and one `case` in
`resolve_ability()`. That is the comparison the test is for. The honest limit
is that a future ability with a brand-new tag still needs the header edit
unless the tag is reserved in advance. The registry handles any integer, but
the tag has to exist somewhere before the new file can name it.
