/*
 * monsters.c
 * Copyright (C) Joachim de Groot 2009 <jdegroot@web.de>
 *
 * NLarn is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NLarn is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nlarn.h"

const monster_data monsters[MT_MAX] =
{
    /* ID                 NAME                  LV   AC DAM INT    GO  HP      EXP IMG he no ha sl fa  fl sp un in iv */
    { MT_NONE,            "",                    0,  0,  0,  0,    0,   0,      0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_BAT,             "bat",                 1,  0,  1,  3,    0,   1,      1, 'B', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { MT_GNOME,           "gnome",               1, 10,  1,  8,   30,   2,      2, 'G', 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { MT_HOBGOBLIN,       "hobgoblin",           1, 14,  2,  5,   25,   3,      2, 'H', 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
    { MT_JACKAL,          "jackal",              1, 17,  1,  4,    0,   1,      1, 'J', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_KOBOLD,          "kobold",              1, 20,  1,  7,   10,   1,      1, 'K', 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { MT_ORC,             "orc",                 2, 12,  1,  9,   40,   4,      2, 'O', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_SNAKE,           "snake",               2, 15,  1,  3,    0,   3,      1, 'S', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_CENTIPEDE,       "giant centipede",     2, 14,  0,  2,    0,   1,      2, 'c', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_JACULI,          "jaculi",              2, 20,  1,  3,    0,   2,      1, 'j', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_TROGLODYTE,      "troglodyte",          2, 10,  2,  5,   80,   4,      3, 't', 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
    { MT_GIANT_ANT,       "giant ant",           2,  8,  1,  3,    0,   5,      5, 'A', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_FLOATING_EYE,    "floating eye",        3,  8,  1,  3,    0,   5,      2, 'E', 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { MT_LEPRECHAUN,      "leprechaun",          3,  3,  0,  6, 1500,  13,     45, 'L', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_NYMPH,           "nymph",               3,  3,  0,  9,    0,  18,     45, 'N', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_QUASIT,          "quasit",              3,  5,  3,  3,    0,  10,     15, 'Q', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_RUST_MONSTER,    "rust monster",        3,  4,  0,  3,    0,  18,     25, 'R', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_ZOMBIE,          "zombie",              3, 12,  2,  3,    0,   6,      7, 'Z', 1, 0, 1, 0, 0, 0, 0, 1, 0, 0 },
    { MT_ASSASSIN_BUG,    "assassin bug",        4,  9,  3,  3,    0,  20,     15, 'a', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_BUGBEAR,         "bugbear",             4,  5,  4,  5,   40,  20,     35, 'b', 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { MT_HELLHOUND,       "hell hound",          4,  5,  2,  6,    0,  16,     35, 'h', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_ICE_LIZARD,      "ice lizard",          4, 11,  2,  6,   50,  16,     25, 'i', 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
    { MT_CENTAUR,         "centaur",             4,  6,  4, 10,   40,  24,     45, 'C', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    /* ID                 NAME                  LV   AC DAM  INT   GO   HP     EXP IMG he no ha sl fa  fl sp un in iv */
    { MT_TROLL,           "troll",               5,  4,  5,  9,   80,  50,    300, 'T', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_YETI,            "yeti",                5,  6,  4,  5,   50,  35,    100, 'Y', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_ELF,             "elf",                 5,  8,  1, 15,   50,  22,     35, 'e', 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { MT_GELATINOUSCUBE,  "gelatinous cube",     5,  9,  1,  3,    0,  22,     45, 'g', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_METAMORPH,       "metamorph",           6,  7,  3,  3,    0,  30,     40, 'm', 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
    { MT_VORTEX,          "vortex",              6,  4,  3,  3,    0,  30,     55, 'v', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_ZILLER,          "ziller",              6, 15,  3,  3,    0,  30,     35, 'z', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_VIOLET_FUNGUS,   "violet fungi",        6, 12,  3,  3,    0,  38,    100, 'F', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_WRAITH,          "wraith",              6,  3,  1,  3,    0,  30,    325, 'w', 1, 0, 1, 0, 0, 1, 0, 1, 0, 0 },
    { MT_FORVALAKA,       "forvalaka",           6,  2,  5,  7,    0,  50,    280, 'f', 1, 0, 0, 0, 1, 0, 0, 1, 0, 1 },
    { MT_LAMA_NOBE,       "lama nobe",           7,  7,  3,  6,    0,  35,     80, 'l', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_OSEQUIP,         "osequip",             7,  4,  3,  4,    0,  35,    100, 'o', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_ROTHE,           "rothe",               7, 15,  5,  3,  100,  50,    250, 'r', 1, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
    { MT_XORN,            "xorn",                7,  0,  6, 13,    0,  60,    300, 'X', 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    { MT_VAMPIRE,         "vampire",             7,  3,  4, 17,    0,  50,   1000, 'v', 1, 0, 1, 0, 0, 1, 0, 1, 0, 0 },
    { MT_STALKER,         "invisible stalker",   7,  3,  6,  5,    0,  50,    350, 'I', 1, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
    { MT_POLTERGEIST,     "poltergeist",         8,  1,  4,  5,    0,  50,    450, 'p', 0, 0, 0, 0, 0, 0, 1, 0, 1, 0 },
    { MT_DISENCHANTRESS,  "disenchantress",      8,  3,  0,  5,    0,  50,    500, 'q', 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_SHAMBLINGMOUND,  "shambling mound",     8,  2,  5,  6,    0,  45,    400, 's', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_YELLOW_MOLD,     "yellow mold",         8, 12,  4,  3,    0,  35,    250, 'y', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_UMBER_HULK,      "umber hulk",          8,  3,  7, 14,    0,  65,    600, 'U', 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    /*  ID                NAME                  LV  AC  DAM INT   GO   HP     EXP IMG  he no ha sl fa  fl sp un in iv */
    { MT_GNOME_KING,      "gnome king",          9, -1, 10, 18, 2000, 100,   3000, 'k', 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { MT_MIMIC,           "mimic",               9,  5,  6,  8,    0,  55,     99, 'M', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_WATER_LORD,      "water lord",          9,-10, 15, 20,    0, 150,  15000, 'w', 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
    { MT_PURPLE_WORM,     "purple worm",         9, -1, 11,  3,  100, 120,  15000, 'P', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_XVART,           "xvart",               9, -2, 12, 13,    0,  90,   1000, 'x', 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
    { MT_WHITE_DRAGON,    "white dragon",        5,  2,  4, 16,  500,  55,   1000, 'd', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_BRONCE_DRAGON,   "bronze dragon",       9,  2,  9, 16,  300,  80,   4000, 'D', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { MT_GREEN_DRAGON,    "green dragon",        9,  3,  8, 15,  200,  70,   2500, 'D', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { MT_SILVER_DRAGON,   "silver dragon",      10, -1, 12, 20,  700, 100,  10000, 'D', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { MT_PLATINUM_DRAGON, "platinum dragon",    10, -5, 15, 22, 1000, 130,  24000, 'D', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { MT_RED_DRAGON,      "red dragon",         10, -2, 13, 19,  800, 110,  14000, 'D', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
    { MT_SPIRIT_NAGA,     "spirit naga",        10,-20, 12, 23,    0,  95,  20000, 'n', 1, 1, 0, 0, 0, 1, 1, 0, 0, 1 },
    { MT_GREEN_URCHIN,    "green urchin",       10, -3, 12,  3,    0,  85,   5000, 'u', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { MT_DEMONLORD_I,     "type I demon lord",  12,-30, 18, 20,    0, 140,  50000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 },
    { MT_DEMONLORD_II,    "type II demon lord", 13,-30, 18, 21,    0, 160,  75000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 },
    { MT_DEMONLORD_III,   "type III demon lord",14,-30, 18, 22,    0, 180, 100000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 },
    { MT_DEMONLORD_IV,    "type IV demon lord", 15,-35, 20, 23,    0, 200, 125000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 },
    { MT_DEMONLORD_V,     "type V demon lord",  16,-40, 22, 24,    0, 220, 150000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 },
    { MT_DEMONLORD_VI,    "type VI demon lord", 17,-45, 24, 25,    0, 240, 175000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 },
    { MT_DEMONLORD_VII,   "type VII demon lord",18,-70, 27, 26,    0, 260, 200000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 },
    { MT_DAEMON_PRINCE,   "demon prince",       25,-127, 30, 28,   0, 345, 300000, '&', 1, 1, 1, 0, 1, 1, 0, 0, 1, 1 }
};

static int monster_genocided[MT_MAX] = { 1, 0 };

static int monster_player_special_attack(monster *m, struct player *p);
static char *monster_player_rob(monster *m, struct player *p, item_t item_type);

monster *monster_new(int monster_type, struct level *l)
{
    monster *nmonster;
    int iid, icount;    /* item id, item count */

    assert(monster_type > MT_NONE && monster_type < MT_MAX && l != NULL);

    /* make room for monster */
    nmonster = g_malloc0(sizeof(monster));

    nmonster->type = monster_type;
    nmonster->hp = divert(monsters[monster_type].hp_max, 10);

    /* put monster into level */
    nmonster->level = l;

    if (!monster_move(nmonster, level_find_space(l, LE_MONSTER)))
    {
        /* no free space could be found for the monstert -> abort */
        g_free(nmonster);
        return NULL;
    }

    g_ptr_array_add(l->mlist, nmonster);

    nmonster->effects = g_ptr_array_new();
    nmonster->inventory = inv_new();

    /* fill monsters inventory */
    if (monster_get_gold(nmonster) > 0)
    {
        /* add gold to monster's inventory, ramdomize the amount */
        icount = max(divert(monster_get_gold(nmonster), 10), 1);
        inv_add(nmonster->inventory, item_new(IT_GOLD, icount, 0));
    }

    switch (monster_type)
    {
    case MT_LEPRECHAUN:
        if (chance(25))
            inv_add(nmonster->inventory,
                    item_new(IT_GEM, rand_m_n(GT_NONE + 1, GT_MAX -1), 0));
        break;

    case MT_ORC:
    case MT_ELF:
    case MT_TROLL:
        iid = rand_1n(WT_SWORDSLASHING);
        inv_add(nmonster->inventory,
                item_new(IT_WEAPON, iid, 0));
        break;

    case MT_NYMPH:
    case MT_TROGLODYTE:
    case MT_ROTHE:
    case MT_VIOLET_FUNGUS:
    case MT_PLATINUM_DRAGON:
    case MT_RED_DRAGON:
    case MT_GNOME_KING:
        inv_add(nmonster->inventory,
                item_create_random(rand_m_n(IT_NONE + 1, IT_MAX -1)));
        break;
    }

    /* initialize AI */
    nmonster->action = MA_WANDER;
    nmonster->lastseen = -1;
    nmonster->player_pos = pos_new(G_MAXINT16, G_MAXINT16);

    return nmonster;
}

monster *monster_new_by_level(struct level *l)
{
    const int monster_level[] = { 5, 11, 17, 22, 27, 33, 39, 42, 46, 50, 53, 56, MT_MAX - 1 };
    int monster_id = 0;
    int monster_id_min;
    int monster_id_max;

    assert(l != NULL);

    if (l->nlevel < 5)
    {
        monster_id_min = 1;
        monster_id_max = monster_level[l->nlevel - 1];
    }
    else
    {
        monster_id_min = monster_level[l->nlevel - 4] + 1;
        monster_id_max = monster_level[l->nlevel - 1];
    }

    while (monster_genocided[monster_id]
            || (monster_id <= MT_NONE)
            || (monster_id >= MT_MAX))
    {
        monster_id = rand_m_n(monster_id_min, monster_id_max);
    }

    return monster_new(monster_id, l);
}

void monster_destroy(monster *m)
{
    assert(m != NULL);

    /* free effects */
    while (m->effects->len > 0)
        effect_destroy(g_ptr_array_remove_index_fast(m->effects, m->effects->len - 1));

    g_ptr_array_free(m->effects, TRUE);

    /* remove monster from level */
    g_ptr_array_remove_fast(m->level->mlist, m);

    /* free inventory */
    if (m->inventory)
        inv_destroy(m->inventory);

    g_free(m);
}

int monster_move(monster *m, position target)
{
    assert(m != NULL);

    if (level_validate_position(m->level, target, LE_MONSTER))
    {
        m->pos = target;
        return TRUE;
    }

    return FALSE;
}

/**
 * Substract monsters hitpoints
 * @param monster
 * @param amount of hitpoints to withdraw
 * @return TRUE if monster has been killed, FALSE if it survived
 */
gboolean monster_hp_lose(monster *m, int amount)
{
    assert(m != NULL);

    m->hp -= amount;

    if (m->hp < 1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void monster_drop_items(monster *m, inventory *floor)
{

    assert(m != NULL && floor != NULL);

    while (inv_length(m->inventory) > 0)
    {
        inv_add(floor, inv_get(m->inventory, 0));
        inv_del(m->inventory, 0);
    }
}

void monster_pickup_items(monster *m, message_log *log)
{
    int i;
    item *it;
    inventory *floor;

    assert(m != NULL && log != NULL);

    if (!(floor = level_ilist_at(m->level, m->pos)))
        return;

    for (i = 1; i <= inv_length(floor); i++)
    {
        it = inv_get(floor, i - 1);

        if (m->type == MT_LEPRECHAUN
                && ((it->type == IT_GEM) || (it->type == IT_GOLD)))
        {
            if (m->m_visible)
            {
                log_add_entry(log,
                              "The %s picks up %s %s.",
                              monster_get_name(m),
                              (it->count == 1) ? "a" : "some",
                              (it->count == 1) ? item_get_name_sg(it->type) : item_get_name_pl(it->type));
            }

            inv_del_element(floor, it);
            inv_add(m->inventory, it);
        }

    }
}

void monster_player_attack(monster *m, player *p)
{
    int dam;

    if (player_effect(p, ET_SPIRIT_PROTECTION) && monster_is_spirit(m))
        return;

    if (player_effect(p, ET_UNDEAD_PROTECTION) && monster_is_undead(m))
        return;

    /* if monster has accomplished a special attack do nothing */
    if (monster_player_special_attack(m, p))
        return;

    /* the player is invisible and the monster bashes into thin air */
    if (!pos_identical(m->player_pos, p->pos))
    {
        if (!level_is_monster_at(p->level, p->pos) && m->m_visible)
        {
            log_add_entry(p->log, "The %s bashes into thin air.",
                          monster_get_name(m));
        }

        m->lastseen++;

        return;
    }

    if (player_effect(p, ET_INVISIBILITY)
            && !monster_has_infravision(m)
            && chance(65))
    {
        log_add_entry(p->log,
                      "The %s misses wildly.",
                      monster_get_name(m));
        return;
    }

    if (player_effect(p, ET_CHARM_MONSTER)
            && (rand_m_n(5, 30) * monster_get_level(m)
                - player_get_cha(p) < 30))
    {
        log_add_entry(p->log,
                      "The %s is awestruck at your magnificence!",
                      monster_get_name(m));
        return;
    }

    dam = monster_get_dam(m);
    if (dam > 1)
    {
        dam += rand_1n(dam);
        dam += monster_get_level(m);
        dam += game_difficulty(p->game);
    }

    if ((dam > player_get_ac(p)) || (rand_1n(20) == 1))
    {
        dam -= player_get_ac(p);

        if (dam > 0)
        {
            player_hp_lose(p, dam, PD_MONSTER, m->type);
            log_add_entry(p->log,
                          "The %s hits you.",
                          monster_get_name(m));
        }
        else
        {
            log_add_entry(p->log,
                          "The %s hit you but your armour protects you.",
                          monster_get_name(m));
        }
    }
    else
    {
        log_add_entry(p->log, "The %s missed.", monster_get_name(m));
    }
}

monster *monster_trigger_trap(monster *m, struct player *p)
{
    /* original position of the monster */
    position pos_orig;

    /* the trap */
    trap_t trap;

    /* effect monster might have gained during the move */
    effect *eff = NULL;

    assert (m != NULL && p != NULL);

    trap = level_trap_at(m->level, m->pos);

    /* return if the monster has not triggered the trap */
    if (!chance(trap_chance(trap)))
    {
        return m;
    }

    /* flying monsters are only affected by sleeping gas traps */
    if (monster_can_fly(m) && (trap != TT_SLEEPGAS))
    {
        return m;
    }

    pos_orig = m->pos;

    /* monster triggered the trap */
    switch (trap)
    {
    case TT_TRAPDOOR:
        /* just remove the monster */
        /* done below */
        break;

    case TT_TELEPORT:
        monster_move(m, level_find_space(m->level, LE_MONSTER));

        break;

    default:
        /* if there is an effect on the trap add it to the
         * monster's list of effects. */
        if (trap_effect(trap))
        {
            eff = effect_new(trap_effect(trap), game_turn(p->game));
            monster_effect_add(m, eff);
        }

    } /* switch (trap) */

    if (m->m_visible)
    {
        log_add_entry(p->log,
                      trap_m_message(trap),
                      monster_get_name(m));

        if (eff)
            log_add_entry(p->log,
                          effect_get_msg_m_start(eff),
                          monster_get_name(m));

        /* set player's knowlege of trap */
        player_memory_of(p, pos_orig).trap = trap;
    }

    /* has the monster survived the trap? */
    if (trap_damage(trap) && monster_hp_lose(m, rand_1n(trap_damage(trap))))
    {
        monster_die(m, p->log);
        m = NULL;
    }

    /* destroy the monster if it has left the level */
    if (trap == TT_TRAPDOOR)
        monster_destroy(m);

    return m;
}


void monster_die(monster *m, message_log *log)
{
    assert(m != NULL);

    /*
     * if the player can see the monster and we have been supplied with a log
     * describe the event
     */
    if (m->m_visible && log)
    {
        log_add_entry(log,
                      "The %s died!",
                      monster_get_name(m));
    }

    /* drop stuff the monster carries */
    if (m->inventory->len)
    {
        if (level_ilist_at(m->level, m->pos) == NULL)
            level_ilist_at(m->level, m->pos) = inv_new();

        monster_drop_items(m, level_ilist_at(m->level, m->pos));
    }

    monster_destroy(m);
}

void monsters_genocide(level *l)
{
    int count;
    monster *monst;

    /* purge genocided monsters */
    for (count = 1; count <= l->mlist->len; count++)
    {
        monst = g_ptr_array_index(l->mlist, count - 1);
        if (monster_is_genocided(monst->type))
        {
            g_ptr_array_remove_index_fast(l->mlist, count - 1);
            monster_destroy(monst);
        }
    }
}

/**
 * Determine a monster's action.
 *
 * @param the monster
 * @return TRUE if the action has changed
 */
gboolean monster_update_action(monster *m)
{
    monster_action_t naction;   /* new action */
    int time;
    gboolean low_hp;
    gboolean smart;

    /* FIXME: should include difficulty here */
    time   = monster_get_int(m) + 25;
    low_hp = (m->hp < (monster_get_hp_max(m) / 4 ));
    smart  = (monster_get_int(m) > 4);

    if (m->type == MT_MIMIC)
    {
        /* stationary monsters */
        naction = MA_REMAIN;
    }
    else if (monster_effect(m, ET_HOLD_MONSTER)
             || monster_effect(m, ET_SLEEP))
    {
        /* no action if monster is held or sleeping */
        naction = MA_REMAIN;
    }
    else if ((low_hp && smart)
             || (monster_effect(m, ET_SCARE_MONSTER) > monster_get_int(m)))
    {
        /* low HP or very scared => FLEE player */
        naction = MA_FLEE;
    }
    else if (m->lastseen && (m->lastseen < time))
    {
        /* after having spotted the player, agressive monster will follow
           the player for a certain amount of time turns, afterwards loose
           interest. More peaceful monsters will do something else. */
        /* TODO: need to test for agressiveness */
        naction = MA_ATTACK;
    }
    else
    {
        /* if no action could be found, return to original behaviour */
        naction = MA_WANDER;
    }

    if (naction != m->action)
    {
        m->action = naction;
        return TRUE;
    }

    return FALSE;
}

void monster_update_player_pos(monster *m, position ppos)
{
    assert (m != NULL);

    m->player_pos = ppos;
    m->lastseen = 1;
}

gboolean monster_regenerate(monster *m, time_t gtime, int difficulty, message_log *log)
{
    /* number of turns between occasions */
    int frequency;

    /* temporary var for effect */
    effect *e;

    assert(m != NULL && log != NULL);

    /* modify frequency by difficulty: more regeneration, less poison */
    frequency = difficulty << 3;

    /* handle regeneration */
    if (m->type == MT_TROLL)
    {
        /* regenerate every (22- frequency) turns */
        if (gtime % (22 - frequency) == 0)
            m->hp = min(monster_get_hp_max(m), m->hp++);

    }

    /* handle poison */
    if ((e = monster_effect_get(m, ET_POISON)))
    {
        if ((gtime - e->start) % (22 + frequency) == 0)
        {
            m->hp -= e->amount;
        }

        if (m->hp < 1)
            return FALSE;
    }

    return TRUE;
}

int monster_genocide(int monster_id)
{
    assert(monster_id > MT_NONE && monster_id < MT_MAX);

    monster_genocided[monster_id] = TRUE;
    return monster_genocided[monster_id];
}

int monster_is_genocided(int monster_id)
{
    assert(monster_id > MT_NONE && monster_id < MT_MAX);
    return monster_genocided[monster_id];
}

void monster_effect_expire(monster *m, message_log *log)
{
    int i = 1;
    effect *e;

    assert(m != NULL && log != NULL);

    while (i <= m->effects->len)
    {
        e = g_ptr_array_index(m->effects, i - 1);

        if (effect_expire(e, 1) == -1)
        {
            /* effect has expired */
            monster_effect_del(m, e);

            /* log info */
            if (m->m_visible && effect_get_msg_m_stop(e))
                log_add_entry(log,
                              effect_get_msg_m_stop(e),
                              monster_get_name(m));

            g_free(e);
        }
        else
        {
            i++;
        }
    }
}

int monster_player_special_attack(monster *m, struct player *p)
{
    int spc_dam = 0; /* damage from special attack */
    char *message = NULL; /* message for special attack */
    item *it; /* destroyed / robbed item */
    int pi; /* impact of perishing */
    effect *ef;

    /* flag to indicate if a special attack has been accomplished */
    gboolean sp_att = TRUE;

    /* special attacks */
    switch (m->type)
    {
    case MT_RUST_MONSTER:
        if (p->eq_suit != NULL)
        {
            pi = item_rust(p->eq_suit);
            if (pi == PI_ENFORCED)
            {
                log_add_entry(p->log, "Your %s is dulled by the %s.",
                              armour_name(p->eq_suit),
                              monster_get_name(m));
            }
            else if (pi == PI_DESTROYED)
            {
                /* armour has been destroyed */
                log_add_entry(p->log, "Your %s disintegrates!",
                              armour_name(p->eq_suit));

                it = p->eq_suit;

                log_disable(p->log);
                player_item_unequip(p, it);
                log_enable(p->log);

                inv_del_element(p->inventory, it);
            }
        }
        break;

    case MT_HELLHOUND:
    case MT_RED_DRAGON:
    case MT_BRONCE_DRAGON:
    case MT_SILVER_DRAGON:
        spc_dam = ((m->type == MT_HELLHOUND) ? rand_m_n(8, 23) : rand_m_n(20, 45))
                  - player_get_ac(p)
                  - player_effect(p, ET_FIRE_RESISTANCE);

        if (spc_dam > 0)
            message = "The %s breathes fire at you!";
        else
            message = "The %s's flame doesn't phase you.";

        break;

    case MT_CENTIPEDE:
    case MT_GIANT_ANT:
        if (player_get_str(p) > 3)
        {
            ef = effect_new(ET_DEC_STR, game_turn(p->game));

            /* this nasty effect goes away after some turns */
            ef->turns = 50;

            player_effect_add(p, ef);
        }

        message = "The %s stung you!";

        break;

    case MT_WHITE_DRAGON:
        spc_dam = rand_1n(15)
                  + 18
                  - player_get_ac(p)
                  - player_effect(p, ET_COLD_RESISTANCE);

        if (spc_dam > 0)
            message = "The %s blasts you with his cold breath.";
        else
            message = "The %s's breath doesn't seem so cold.";

        break;

    case MT_VAMPIRE:
    case MT_WRAITH:
        message = "The %s drains you of your life energy!";
        player_lvl_lose(p, 1);
        break;

    case MT_WATER_LORD:
        message = "The %s got you with a gusher!";
        spc_dam = rand_1n(15)
                  + 25
                  - player_get_ac(p);
        break;

    case MT_LEPRECHAUN:
        /* if player has a device of no theft abort this */
        if (!player_effect(p, ET_NOTHEFT))
            message = monster_player_rob(m, p, IT_GOLD);

        /* teleport away */
        monster_move(m, level_find_space(p->level, LE_MONSTER));

        break;

    case MT_DISENCHANTRESS:
        /* destroy random item */
        it = inv_get(p->inventory, rand_1n(inv_length(p->inventory)));
        if (it->type != IT_SCROLL && it->type != IT_POTION)
        {
            message = "The %s hits you - you feel a sense of loss.";
            if (player_item_is_equipped(p, it))
                player_item_unequip(p, it);

            inv_del_element(p->inventory, it);
            item_destroy(it);
        }
        break;

    case MT_ICE_LIZARD:
    case MT_GREEN_DRAGON:
        message = "The %s hit you with his barbed tail.";
        spc_dam = rand_1n(25)
                  - player_get_ac(p);

        break;

    case MT_UMBER_HULK:
        ef = effect_new(ET_CONFUSION, game_turn(p->game));
        player_effect_add(p, ef);
        break;

    case MT_SPIRIT_NAGA:
        /* FIXME: random effect */
        break;

    case MT_PLATINUM_DRAGON:
        message = "The %s flattens you with his psionics!";
        spc_dam = rand_1n(15)
                  + 30
                  - player_get_ac(p);
        break;

    case MT_NYMPH:
        if (!player_effect(p, ET_NOTHEFT))
            message = monster_player_rob(m, p, IT_ALL);

        /* teleport away */
        monster_move(m, level_find_space(p->level, LE_MONSTER));


        break;

    case MT_BUGBEAR:
    case MT_OSEQUIP:
        spc_dam = ((m->type == MT_BUGBEAR)
                   ? rand_m_n(5, 15)
                   : rand_m_n(10, 25))
                  - player_get_ac(p);

        message = "The %s bit you!";
        spc_dam = rand_1n(15) + 10 - player_get_ac(p);
        break;

    default:
        sp_att = FALSE;
        break;
    } /* special attacks */

    if (spc_dam > 0)
    {
        player_hp_lose(p, spc_dam, PD_MONSTER, m->type);
    }

    if (message != NULL)
    {
        log_add_entry(p->log, message, monster_get_name(m));
    }

    return sp_att;
}

char *monster_player_rob(monster *m, struct player *p, item_t item_type)
{
    char *message = NULL;
    int player_gold = 0;
    item *it = NULL;

    assert (m != NULL && p != NULL);

    /* Leprechaun robs only gold */
    if (item_type == IT_GOLD)
    {
        if ((player_gold = player_get_gold(p)))
        {
            if (player_gold > 32767)
            {
                it = item_new(IT_GOLD, player_gold >> 1, 0);
                player_set_gold(p, player_gold >> 1);
            }
            else
            {
                it = item_new(IT_GOLD, rand_1n(1 + (player_gold >> 1)), 0);
                player_set_gold(p, player_gold - it->count);
            }

            if (player_get_gold(p) < 0)
                player_set_gold(p, 0);

            message = "The %s picks your pocket. Your purse feels lighter";
        }
    }
    else if (item_type == IT_ALL) /* must be the nymph */
    {
        if (inv_length(p->inventory))
        {
            it = inv_get(p->inventory, rand_0n(inv_length(p->inventory) - 1));

            if (player_item_is_equipped(p, it))
            {
                log_disable(p->log);
                player_item_unequip(p, it);
                log_enable(p->log);
            }

            inv_del_element(p->inventory, it);
            message = "The %s picks your pocket.";
        }
    }

    /* if item / gold has been stolen, add it to the monster's inv */
    if (it)
    {
        inv_add(m->inventory, it);
    }
    else
    {
        message = "The %s couldn't find anything to steal";
    }

    return message;
}
