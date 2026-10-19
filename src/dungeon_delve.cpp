/* ===========================================================================
   dungeon_delve.cpp

   Dungeon Delve -- encounter engine.

   Started this as a weekend thing to test the combat math and it sort of
   became the whole game. It works. Ship it.

   TODO (someday):
     - split this file up
     - the loot thing is weird sometimes, look at it
     - save files break if you rename a hero, don't rename heroes
     - do NOT touch resolve_ability(), it took forever to get right

   -- B.R.
   =========================================================================== */

#include "dungeon_delve.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/* ---------------------------------------------------------------------------
   RNG. Rolled our own so the numbers are the same on every machine, which
   matters because the designers balance encounters against specific seeds.
   --------------------------------------------------------------------------- */

static unsigned g_seed = 1337u;

void dd_seed(unsigned s)
{
    g_seed = s;
}

int dd_rand()
{
    g_seed = g_seed * 1103515245u + 12345u;
    return (int)((g_seed >> 16) & 0x7FFF);
}

/* NOTE: this is inclusive on both ends. */
int dd_rand_range(int lo, int hi)
{
    if (hi <= lo) return lo;
    return lo + dd_rand() % (hi - lo + 1);
}

int roll_dice(int count, int sides)
{
    int total = 0;
    for (int i = 0; i < count; i++)
        total += dd_rand_range(1, sides);
    return total;
}

/* ---------------------------------------------------------------------------
   The loot table.

   This is a plain C array on purpose -- the content team edits it by hand in
   a text editor and they do not want it moved into a database or a map or
   anything clever. Leave it alone.
   --------------------------------------------------------------------------- */

#define LOOT_COUNT 12

static const char* LOOT_TABLE[LOOT_COUNT] = {
    "Rusted Dagger",
    "Cracked Buckler",
    "Mouldy Rations",
    "Copper Ring",
    "Tin Whistle",
    "Bent Lockpick",
    "Waterlogged Scroll",
    "Chipped Whetstone",
    "Iron Spur",
    "Glass Eye",
    "Knotted Rope",
    "Tarnished Locket"
};

static int LOOT_VALUE[LOOT_COUNT] = {
    4, 6, 1, 12, 3, 8, 15, 5, 7, 2, 3, 20
};

/* ---------------------------------------------------------------------------
   Ability tags.

   Every ability in the game is an int. resolve_ability() switches on it.
   Adding an ability means adding a tag here and a case in every switch that
   looks at tags, and there are a few of those now.
   --------------------------------------------------------------------------- */

const char* ability_name(int tag)
{
    switch (tag) {
        case ABIL_NONE:   return "Flail";
        case ABIL_CLEAVE: return "Cleave";
        case ABIL_BITE:   return "Bite";
        case ABIL_MEND:   return "Mend";
        case ABIL_HEX:    return "Hex";
        case ABIL_SUMMON: return "Summon";
        case ABIL_GUARD:  return "Guard";
        case ABIL_RALLY:  return "Rally";
    }
    return "???";
}

/* ---------------------------------------------------------------------------
   Allocation helpers.
   --------------------------------------------------------------------------- */

HeroPtr make_hero(const char* name, int hp, int strength, int armor)
{
    Hero* h = new Hero;
    std::strncpy(h->name, name, 31);
    h->name[31] = '\0';
    h->hp       = hp;
    h->max_hp   = hp;
    h->strength = strength;
    h->armor    = armor;
    h->gold     = 0;
    h->xp       = 0;
    h->level    = 1;
    h->potions  = 2;
    return HeroPtr{h};
}

MonsterPtr make_monster(const char* name, const char* species,
                        int hp, int strength, int armor,
                        int gold_drop, int ability_tag)
{
    Monster* m = new Monster;
    std::strncpy(m->name, name, 31);
    m->name[31] = '\0';
    std::strncpy(m->species, species, 23);
    m->species[23] = '\0';
    m->hp            = hp;
    m->max_hp        = hp;
    m->strength      = strength;
    m->armor         = armor;
    m->gold_drop     = gold_drop;
    m->ability_tag   = ability_tag;
    m->is_dead       = 0;
    m->summon_budget = (ability_tag == ABIL_SUMMON) ? 1 : 0;
    return MonsterPtr{m};
}

MerchantPtr make_merchant(const char* name, int gold, int markup)
{
    Merchant* mer = new Merchant;
    std::strncpy(mer->name, name, 31);
    mer->name[31]       = '\0';
    mer->gold           = gold;
    mer->markup_percent = markup;
    mer->stock_count    = 0;
    for (int i = 0; i < 8; i++)
        mer->stock_tags[i] = -1;
    return MerchantPtr{mer};
}

/* ---------------------------------------------------------------------------
   Loot.
   --------------------------------------------------------------------------- */

/* Used by the display code and by the content team's export script. */
const char* loot_at(int i)
{
    if (i < 0 || i >= LOOT_COUNT) return nullptr;
    return LOOT_TABLE[i];
}

int loot_count()
{
    return LOOT_COUNT;
}

const char* roll_loot()
{
    int idx = dd_rand_range(0, LOOT_COUNT);
    return LOOT_TABLE[idx];
}

int loot_value_of(const char* loot_name)
{
    for (int i = 0; i < LOOT_COUNT; i++) {
        if (std::strcmp(LOOT_TABLE[i], loot_name) == 0)
            return LOOT_VALUE[i];
    }
    return 0;
}

/* ---------------------------------------------------------------------------
   Damage.
   --------------------------------------------------------------------------- */

int compute_raw_damage(int strength, int dice_count, int dice_sides)
{
    return strength + roll_dice(dice_count, dice_sides);
}

int apply_armor(int raw_damage, int armor)
{
    int out = raw_damage - armor;
    if (out < 1) out = 1;
    return out;
}

int apply_damage_to_monster(Monster& m, int incoming)
{
    int final_damage = apply_armor(incoming, m.armor);

    m.hp -= final_damage;
    if (m.hp <= 0) {
        m.hp = 0;
        m.is_dead = 1;
    }
    return final_damage;
}

int apply_damage_to_hero(Hero& h, int incoming)
{
    int final_damage = apply_armor(incoming, h.armor);

    h.hp -= final_damage;
    if (h.hp < 0) h.hp = 0;
    return final_damage;
}

/* Hero swings at a monster. Returns the damage that landed. */
int resolve_attack_on_monster(Hero& attacker, Monster& target)
{
    int raw_damage = compute_raw_damage(attacker.strength, 3, 6);
    int after_armor = apply_armor(raw_damage, target.armor);

    apply_damage_to_monster(target, after_armor);
    return after_armor;
}

/* Monster swings at the hero. Returns the damage that landed. */
int resolve_attack_on_hero(Monster& attacker, Hero& target)
{
    int raw_damage = compute_raw_damage(attacker.strength, 2, 6);
    int after_armor = apply_armor(raw_damage, target.armor);

    apply_damage_to_hero(target, after_armor);
    return after_armor;
}

/* ---------------------------------------------------------------------------
   resolve_ability()

   Do not touch this. It took forever to get right.
   --------------------------------------------------------------------------- */

int resolve_ability(int tag, Monster& self, Hero& hero,
                    Wave& wave, int verbose)
{
    int amount = 0;

    switch (tag) {

        case ABIL_NONE:
            amount = resolve_attack_on_hero(self, hero);
            if (verbose)
                std::printf("  %s flails .................. %d damage\n",
                            self.name, amount);
            break;

        case ABIL_CLEAVE:
            amount = resolve_attack_on_hero(self, hero);
            amount = amount + (amount / 2);
            apply_damage_to_hero(hero, amount / 2);
            if (verbose)
                std::printf("  %s cleaves ................. %d damage\n",
                            self.name, amount);
            break;

        case ABIL_BITE:
            amount = resolve_attack_on_hero(self, hero);
            if (dd_rand_range(1, 100) > 70) {
                apply_damage_to_hero(hero, 2);
                amount += 2;
                if (verbose)
                    std::printf("  %s bites deep ............. %d damage\n",
                                self.name, amount);
            } else {
                if (verbose)
                    std::printf("  %s bites ................... %d damage\n",
                                self.name, amount);
            }
            break;

        case ABIL_MEND:
            amount = roll_dice(2, 4);
            self.hp += amount;
            if (self.hp > self.max_hp) self.hp = self.max_hp;
            if (verbose)
                std::printf("  %s mends ................... +%d HP\n",
                            self.name, amount);
            break;

        case ABIL_HEX:
            amount = roll_dice(1, 4);
            hero.strength -= amount;
            if (hero.strength < 1) hero.strength = 1;
            if (verbose)
                std::printf("  %s hexes ................... -%d STR\n",
                            self.name, amount);
            break;

        case ABIL_SUMMON:
            if (self.summon_budget > 0) {
                self.summon_budget--;
                MonsterPtr pup = make_monster("Whelp", "whelp", 6, 2, 0, 3, ABIL_NONE);
                wave.push_back(std::move(pup));
                amount = 1;
                if (verbose)
                    std::printf("  %s summons a Whelp!\n", self.name);
            } else {
                amount = resolve_attack_on_hero(self, hero);
                if (verbose)
                    std::printf("  %s is out of whelps ....... %d damage\n",
                                self.name, amount);
            }
            break;

        case ABIL_GUARD:
            self.armor += 2;
            amount = 2;
            if (verbose)
                std::printf("  %s guards .................. +2 armor\n",
                            self.name);
            break;

        case ABIL_RALLY:
            for (size_t i = 0; i < wave.size(); i++) {
                if (!wave[i]->is_dead && raw(wave[i]) != &self)
                    wave[i]->strength += 1;
            }
            amount = 1;
            if (verbose)
                std::printf("  %s rallies the pack ........ +1 STR each\n",
                            self.name);
            break;

        default:
            amount = resolve_attack_on_hero(self, hero);
            if (verbose)
                std::printf("  %s does something ......... %d damage\n",
                            self.name, amount);
            break;
    }

    return amount;
}

/* ---------------------------------------------------------------------------
   Bestiary lookup.
   --------------------------------------------------------------------------- */

Monster* find_in_bestiary(Bestiary& bestiary, const char* species)
{
    for (size_t i = 0; i < bestiary.size(); i++) {
        if (std::strcmp(bestiary[i]->species, species) == 0)
            return raw(bestiary[i]);
    }
    return nullptr;
}

int count_species_in_bestiary(Bestiary& bestiary, const char* species)
{
    int n = 0;
    for (size_t i = 0; i < bestiary.size(); i++) {
        if (std::strcmp(bestiary[i]->species, species) == 0)
            n++;
    }
    return n;
}

Bestiary build_bestiary()
{
    Bestiary b;
    b.push_back(make_monster("Kobold",      "kobold",    12,  4, 1,  6, ABIL_BITE));
    b.push_back(make_monster("Goblin",      "goblin",    10,  3, 0,  5, ABIL_NONE));
    b.push_back(make_monster("Cave Bat",    "bat",        5,  2, 0,  1, ABIL_BITE));
    b.push_back(make_monster("Hex Adept",   "adept",     14,  3, 1, 11, ABIL_HEX));
    b.push_back(make_monster("Bone Priest", "priest",    16,  4, 2, 14, ABIL_MEND));
    b.push_back(make_monster("Warg",        "warg",      18,  6, 1, 10, ABIL_CLEAVE));
    b.push_back(make_monster("Brood Mother","brood",     22,  5, 2, 18, ABIL_SUMMON));
    b.push_back(make_monster("Shield Ogre", "ogre",      26,  7, 3, 21, ABIL_GUARD));
    b.push_back(make_monster("Pack Leader", "packleader",20,  5, 1, 16, ABIL_RALLY));
    return b;
}

MonsterPtr spawn_from_bestiary(Bestiary& bestiary, const char* species)
{
    Monster* proto = find_in_bestiary(bestiary, species);
    if (proto == nullptr) return MonsterPtr{};

    return make_monster(proto->name, proto->species, proto->max_hp,
                        proto->strength, proto->armor, proto->gold_drop,
                        proto->ability_tag);
}

/* ---------------------------------------------------------------------------
   Encounter waves.
   --------------------------------------------------------------------------- */

int wave_is_cleared(Wave& wave)
{
    for (size_t i = 0; i < wave.size(); i++) {
        if (!wave[i]->is_dead) return 0;
    }
    return 1;
}

int living_in_wave(Wave& wave)
{
    int n = 0;
    for (size_t i = 0; i < wave.size(); i++) {
        if (!wave[i]->is_dead) n++;
    }
    return n;
}

/* Every living monster in the wave takes its turn against the hero.
   Returns the total damage the wave dealt this round. */
int run_monster_round(Wave& wave, Hero& hero, int verbose,
                      std::vector<std::string>* acted_names)
{
    int total = 0;

    for (Wave::iterator it = wave.begin(); it != wave.end(); ++it) {
        Monster* m = raw(*it);

        if (m->is_dead) continue;
        if (hero.hp <= 0) break;

        if (acted_names != nullptr)
            acted_names->push_back(std::string(m->name));

        total += resolve_ability(m->ability_tag, *m, hero, wave, verbose);
    }

    return total;
}

/* Hero attacks the first living monster. */
int run_hero_round(Wave& wave, Hero& hero, int verbose)
{
    for (size_t i = 0; i < wave.size(); i++) {
        if (wave[i]->is_dead) continue;

        int dealt = resolve_attack_on_monster(hero, *raw(wave[i]));
        if (verbose) {
            std::printf("  %s strikes %s ........... %d damage\n",
                        hero.name, wave[i]->name, dealt);
            if (wave[i]->is_dead)
                std::printf("  %s falls.\n", wave[i]->name);
        }
        return dealt;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
   The encounter loop.
   --------------------------------------------------------------------------- */

int run_encounter(Hero& hero, Wave& wave, int verbose)
{
    int round = 1;

    if (verbose) {
        std::printf("\n");
        std::printf("--- ENCOUNTER: %d monster(s) ---\n", living_in_wave(wave));
    }

    while (hero.hp > 0 && !wave_is_cleared(wave) && round <= 40) {

        if (verbose)
            std::printf("\n[round %d]\n", round);

        run_hero_round(wave, hero, verbose);

        if (wave_is_cleared(wave)) break;

        run_monster_round(wave, hero, verbose, nullptr);

        round++;
    }

    if (hero.hp <= 0) {
        if (verbose) std::printf("\n%s has fallen.\n", hero.name);
        return 0;
    }

    int gold = 0;
    for (size_t i = 0; i < wave.size(); i++) {
        if (wave[i]->is_dead)
            gold += wave[i]->gold_drop;
    }

    hero.gold += gold;
    hero.xp   += gold * 2;

    const char* drop = roll_loot();

    if (verbose) {
        std::printf("\nWave cleared.\n");
        std::printf("  gold ...... %d\n", gold);
        std::printf("  loot ...... %s (worth %d)\n", drop, loot_value_of(drop));
    }

    return 1;
}

/* ---------------------------------------------------------------------------
   Levelling.
   --------------------------------------------------------------------------- */

int xp_for_next_level(int level)
{
    return level * 100;
}

void check_level_up(Hero& h, int verbose)
{
    while (h.xp >= xp_for_next_level(h.level)) {
        h.xp -= xp_for_next_level(h.level);
        h.level++;
        h.max_hp   += 10;
        h.hp        = h.max_hp;
        h.strength += 2;

        if (verbose)
            std::printf("\n*** %s reaches level %d ***\n", h.name, h.level);
    }
}

/* ---------------------------------------------------------------------------
   The merchant.
   --------------------------------------------------------------------------- */

int merchant_price(Merchant& mer, int base_value)
{
    return base_value + (base_value * mer.markup_percent) / 100;
}

void visit_merchant(Merchant& mer, Hero& hero, int verbose)
{
    int potion_price = merchant_price(mer, 15);

    if (verbose) {
        std::printf("\n--- %s's stall ---\n", mer.name);
        std::printf("  potion ..... %d gold (you have %d)\n",
                    potion_price, hero.gold);
    }

    while (hero.gold >= potion_price && hero.potions < 5) {
        hero.gold -= potion_price;
        hero.potions++;
        mer.gold  += potion_price;
        if (verbose)
            std::printf("  bought a potion. potions=%d gold=%d\n",
                        hero.potions, hero.gold);
    }
}

void quaff_potion(Hero& h, int verbose)
{
    if (h.potions <= 0) return;
    h.potions--;
    int healed = roll_dice(3, 6);
    h.hp += healed;
    if (h.hp > h.max_hp) h.hp = h.max_hp;
    if (verbose)
        std::printf("  %s drinks a potion ....... +%d HP\n", h.name, healed);
}

/* ---------------------------------------------------------------------------
   Save and load. Plain text, one field per line, order matters.
   Renaming a hero breaks old saves. Don't rename heroes.
   --------------------------------------------------------------------------- */

int save_hero(Hero& h, const char* path)
{
    FILE* f = std::fopen(path, "w");
    if (f == nullptr) return 0;

    std::fprintf(f, "%s\n", h.name);
    std::fprintf(f, "%d\n", h.hp);
    std::fprintf(f, "%d\n", h.max_hp);
    std::fprintf(f, "%d\n", h.strength);
    std::fprintf(f, "%d\n", h.armor);
    std::fprintf(f, "%d\n", h.gold);
    std::fprintf(f, "%d\n", h.xp);
    std::fprintf(f, "%d\n", h.level);
    std::fprintf(f, "%d\n", h.potions);

    std::fclose(f);
    return 1;
}

HeroPtr load_hero(const char* path)
{
    FILE* f = std::fopen(path, "r");
    if (f == nullptr) return HeroPtr{};

    Hero* h = new Hero;
    char line[64];

    if (std::fgets(line, 64, f) == nullptr) { std::fclose(f); delete h; return HeroPtr{}; }
    size_t n = std::strlen(line);
    if (n > 0 && line[n - 1] == '\n') line[n - 1] = '\0';
    std::strncpy(h->name, line, 31);
    h->name[31] = '\0';

    std::fscanf(f, "%d", &h->hp);
    std::fscanf(f, "%d", &h->max_hp);
    std::fscanf(f, "%d", &h->strength);
    std::fscanf(f, "%d", &h->armor);
    std::fscanf(f, "%d", &h->gold);
    std::fscanf(f, "%d", &h->xp);
    std::fscanf(f, "%d", &h->level);
    std::fscanf(f, "%d", &h->potions);

    std::fclose(f);
    return HeroPtr{h};
}

/* ---------------------------------------------------------------------------
   Display. Printing is done wherever it is convenient.
   --------------------------------------------------------------------------- */

void print_banner()
{
    for (int i = 0; i < 64; i++) std::printf("=");
    std::printf("\n");
    std::printf("  D U N G E O N   D E L V E\n");
    for (int i = 0; i < 64; i++) std::printf("=");
    std::printf("\n");
}

void print_hero(Hero& h)
{
    std::printf("\n%s  --  level %d\n", h.name, h.level);
    std::printf("  HP ......... %d / %d\n", h.hp, h.max_hp);
    std::printf("  STR ........ %d\n", h.strength);
    std::printf("  ARMOR ...... %d\n", h.armor);
    std::printf("  GOLD ....... %d\n", h.gold);
    std::printf("  XP ......... %d / %d\n", h.xp, xp_for_next_level(h.level));
    std::printf("  POTIONS .... %d\n", h.potions);
}

void print_wave(Wave& wave)
{
    std::printf("\nWave:\n");
    for (size_t i = 0; i < wave.size(); i++) {
        std::printf("  %-14s %-11s %3d/%-3d HP  str %-2d  armor %-2d  %s\n",
                    wave[i]->name, wave[i]->species,
                    wave[i]->hp, wave[i]->max_hp,
                    wave[i]->strength, wave[i]->armor,
                    ability_name(wave[i]->ability_tag));
    }
}

void print_bestiary(Bestiary& bestiary)
{
    std::printf("\nBestiary (%d entries):\n", (int)bestiary.size());
    for (size_t i = 0; i < bestiary.size(); i++) {
        std::printf("  %-14s %-11s %3d HP  str %-2d  armor %-2d  gold %-3d  %s\n",
                    bestiary[i]->name, bestiary[i]->species,
                    bestiary[i]->max_hp, bestiary[i]->strength,
                    bestiary[i]->armor, bestiary[i]->gold_drop,
                    ability_name(bestiary[i]->ability_tag));
    }
}

void print_loot_table()
{
    std::printf("\nLoot table (%d entries):\n", LOOT_COUNT);
    for (int i = 0; i < LOOT_COUNT; i++)
        std::printf("  %-22s %3d gold\n", LOOT_TABLE[i], LOOT_VALUE[i]);
}

/* ---------------------------------------------------------------------------
   The campaign.
   --------------------------------------------------------------------------- */

Wave build_wave(Bestiary& bestiary, const char* a, const char* b, const char* c)
{
    Wave wave;

    if (a != nullptr) {
        MonsterPtr m = spawn_from_bestiary(bestiary, a);
        if (raw(m) != nullptr) wave.push_back(std::move(m));
    }
    if (b != nullptr) {
        MonsterPtr m = spawn_from_bestiary(bestiary, b);
        if (raw(m) != nullptr) wave.push_back(std::move(m));
    }
    if (c != nullptr) {
        MonsterPtr m = spawn_from_bestiary(bestiary, c);
        if (raw(m) != nullptr) wave.push_back(std::move(m));
    }

    return wave;
}

int run_campaign(int verbose)
{
    print_banner();

    Bestiary    bestiary = build_bestiary();
    HeroPtr     hero_owner = make_hero("Ash", 100, 10, 3);
    MerchantPtr yarrow_owner = make_merchant("Old Yarrow", 400, 35);

    Hero&     hero = *hero_owner;          /* non-owning alias for readability */
    Merchant& yarrow = *yarrow_owner;      /* non-owning alias for readability */

    if (verbose) {
        print_hero(hero);
        print_bestiary(bestiary);
        print_loot_table();
    }

    const char* script[6][3] = {
        { "bat",        "bat",     nullptr      },
        { "goblin",     "kobold",  nullptr      },
        { "adept",      "goblin",  "bat"        },
        { "warg",       "kobold",  nullptr      },
        { "brood",      "goblin",  "bat"        },
        { "ogre",       "priest",  "packleader" }
    };

    int cleared = 0;

    for (int stage = 0; stage < 6; stage++) {

        Wave wave = build_wave(bestiary,
                               script[stage][0],
                               script[stage][1],
                               script[stage][2]);

        if (verbose) {
            std::printf("\n");
            for (int i = 0; i < 64; i++) std::printf("-");
            std::printf("\nSTAGE %d\n", stage + 1);
            print_wave(wave);
        }

        if (hero.hp < hero.max_hp / 3)
            quaff_potion(hero, verbose);

        int survived = run_encounter(hero, wave, verbose);

        if (!survived) {
            if (verbose) std::printf("\nRun over at stage %d.\n", stage + 1);
            return cleared;
        }

        cleared++;
        check_level_up(hero, verbose);

        if (stage == 2)
            visit_merchant(yarrow, hero, verbose);

        if (verbose) print_hero(hero);
    }

    if (verbose) {
        std::printf("\n");
        for (int i = 0; i < 64; i++) std::printf("=");
        std::printf("\n  %s clears the delve with %d gold.\n",
                    hero.name, hero.gold);
        for (int i = 0; i < 64; i++) std::printf("=");
        std::printf("\n");
    }

    save_hero(hero, "ash.save");

    return cleared;
}

/* ---------------------------------------------------------------------------
   main
   --------------------------------------------------------------------------- */

#ifndef DD_NO_MAIN
int main(int argc, char** argv)
{
    unsigned seed = 1337u;
    int verbose = 1;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--quiet") == 0) {
            verbose = 0;
        } else if (std::strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = (unsigned)std::atoi(argv[i + 1]);
            i++;
        }
    }

    dd_seed(seed);

    int cleared = run_campaign(verbose);

    std::printf("\nstages cleared: %d / 6\n", cleared);
    return 0;
}
#endif
