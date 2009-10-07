/*
 * player.h
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

#ifndef __PLAYER_H_
#define __PLAYER_H_

#include "amulets.h"
#include "armour.h"
#include "buildings.h"
#include "map.h"
#include "monsters.h"
#include "position.h"
#include "potions.h"
#include "rings.h"
#include "scrolls.h"
#include "spells.h"
#include "utils.h"
#include "weapons.h"

/* forward declaration */
struct game;

typedef struct player_stats
{
    guint32 moves_made;
    guint32 deepest_level;
    guint32 monsters_killed[MT_MAX];
    guint32 spells_cast;
    guint32 potions_quaffed;
    guint32 scrolls_read;
    guint32 books_read;
    guint32 gold_collected;
    guint32 gold_spent;
    guint32 times_prayed;
    guint32 max_level;
    guint32 max_xp;
} player_stats;

typedef struct _player_settings
{
    gboolean auto_pickup[IT_MAX]; /* automatically pick up item of enabled types */
} player_settings;


struct _player_tile_memory
{
    map_tile_t type;
    map_stationary_t stationary;
    item_t item; /* type of item located here */
    trap_t trap;
};

typedef struct _player_tile_memory player_tile_memory;

typedef struct player
{
    char *name;
    guint8 sex; /* 0 female, 1 male */

    guint32 strength;
    guint32 intelligence;
    guint32 wisdom;
    guint32 constitution;
    guint32 dexterity;
    guint32 charisma;

    gint32 hp; /* current hp */
    guint32 hp_max; /* max hp */
    gint32 mp;
    guint32 mp_max;
    guint32 regen_counter; /* regeneration counter */

    guint32 bank_account; /* There is nothing quite as wonderful as money */
    guint32 outstanding_taxes;
    guint32 interest_lasttime; /* last time interest has been calculated */

    guint32 experience; /* experience points */
    guint32 lvl; /* current experience level */

    /* other stuff */
    GPtrArray *known_spells;
    inventory *inventory;
    GPtrArray *effects; /* temporary effects from potions, spells, ... */

    /* pointers to elements of items which are currently equipped */
    item *eq_amulet;
    item *eq_weapon;

    /* armour types */
    item *eq_boots;
    item *eq_cloak;
    item *eq_gloves;
    item *eq_helmet;
    item *eq_shield;
    item *eq_suit;

    /* rings */
    item *eq_ring_l;
    item *eq_ring_r;
    /* enough items for now */

    /* items identified */
    guint8 identified_amulets[AM_MAX];
    guint8 identified_books[SP_MAX];
    guint8 identified_potions[PO_MAX];
    guint8 identified_rings[RT_MAX];
    guint8 identified_scrolls[ST_MAX];

    position pos; /* player's position */
    map *map; /* current dungeon map */
    player_stats stats; /* statistics */
    message_log *log; /* game message log */

    /* player's field of vision */
    int fov[MAP_MAX_Y][MAP_MAX_X];

    /* player's memory of the map */
    player_tile_memory memory[MAP_MAX][MAP_MAX_Y][MAP_MAX_X];

    /* courses available in school */
    guint8 school_courses_taken[SCHOOL_COURSE_COUNT];

    player_settings settings; /* game configuration */
} player;

typedef enum player_equipment_t
{
    PE_NONE,
    PE_AMULET,
    PE_BOOTS,
    PE_CLOAK,
    PE_GLOVES,
    PE_HELMET,
    PE_RING_L,
    PE_RING_R,
    PE_SHIELD,
    PE_SUIT,
    PE_WEAPON,
    PE_MAX
} player_equipment_t;

/* various causes of death */
typedef enum player_cod
{
    PD_NONE,
    PD_EFFECT,
    PD_LASTLEVEL, /* lost a level at level 1 */
    PD_MONSTER,
    PD_SPHERE,
    PD_TRAP,
    PD_LEVEL, /* damaged by level effects */
    PD_SPELL, /* damaged by own spell */
    PD_CURSE, /* damaged by a cursed item */
    PD_STATIONARY, /* killed by stationary item */
    /* *** causes above this line can be stopped by live protection *** */
    PD_STUCK, /* stuck in a wall */
    PD_TOO_LATE, /* returned with potion too late */
    PD_WON, /* won the game */
    PD_LOST, /* daughter has died, potion not found */
    PD_QUIT, /* quit the game */
    PD_MAX
} player_cod;

/* function declarations */

player *player_new();
void player_destroy(player *p);

int player_regenerate(player *p);
void player_die(player *p, player_cod cause_type, int cause);
gint64 player_calc_score(player *p, int won);
int player_move(player *p, direction dir);
int player_attack(player *p, monster *m);
void player_update_fov(player *p, int radius);
int player_level_enter(player *p, map *l, gboolean teleported);
item *player_random_armour(player *p);

int player_examine(player *p, position pos);
int player_pickup(player *p);

void player_autopickup(player *p);
void player_autopickup_show(player *p);

void player_lvl_gain(player *p, int count);
void player_lvl_lose(player *p, int count);

void player_exp_gain(player *p, int count);
void player_exp_lose(player *p, int count);

int player_hp_gain(player *p, int count);
void player_damage_take(player *p, damage *dam, player_cod cause_type, int cause);
int player_hp_max_gain(player *p, int count);
int player_hp_max_lose(player *p, int count);

int player_mp_gain(player *p, int count);
int player_mp_lose(player *p, int count);
int player_mp_max_gain(player *p, int count);
int player_mp_max_lose(player *p, int count);

/* dealing with temporary effects */
void player_effect_add(player *p, effect *e);
void player_effects_add(player *p, GPtrArray *effects);
int player_effect_del(player *p, effect *e);
void player_effects_del(player *p, GPtrArray *effects);
effect *player_effect_get(player *p, int effect_id);
int player_effect(player *p, int effect_type); /* check if a effect is set */
void player_effects_expire(player *p, int turns);
GPtrArray *player_effect_text(player *p);

/* dealing with the inventory */
int player_inv_display(player *p);
char *player_inv_weight(player *p);
int player_inv_pre_add(inventory *inv, item *item);
void player_inv_weight_recalc(inventory *inv, item *item);

/* dealing with items */
int player_item_equip(player *p, inventory **inv, item *it);
int player_item_unequip(player *p, inventory **inv, item *it);
int player_item_is_container(player *p, item *it);
int player_item_can_be_added_to_container(player *p, item *it);
int player_item_is_equipped(player *p, item *it);
int player_item_is_equippable(player *p, item *it);
int player_item_is_usable(player *p, item *it);
int player_item_is_dropable(player *p, item *it);
int player_item_is_damaged(player *p, item *it);
int player_item_is_affordable(player *p, item *it);
int player_item_is_sellable(player *p, item *it);
int player_item_is_identifiable(player *p, item *it);
int player_item_known(player *p, item *it);
int player_item_identified(player *p, item *it);
char *player_item_identified_list(player *p);
void player_item_identify(player *p, inventory **inv, item *it);
int player_item_use(player *p, inventory **inv, item *it);
int player_item_drop(player *p, inventory **inv, item *it);
int player_item_pickup(player *p, inventory **inv, item *it);

/* deal with stationaries */
int player_altar_desecrate(player *p);
int player_altar_pray(player *p);
int player_building_enter(player *p);
int player_door_close(player *p);
int player_door_open(player *p);
int player_fountain_drink(player *p);
int player_fountain_wash(player *p);
int player_stairs_down(player *p);
int player_stairs_up(player *p);
int player_throne_pillage(player *p);
int player_throne_sit(player *p);

/* query values */
int player_get_ac(player *p);
int player_get_wc(player *p);
int player_get_hp_max(player *p);
int player_get_mp_max(player *p);
int player_get_str(player *p);
int player_get_int(player *p);
int player_get_wis(player *p);
int player_get_con(player *p);
int player_get_dex(player *p);
int player_get_cha(player *p);

/* deal with money */
guint player_get_gold(player *p);
guint player_set_gold(player *p, guint amount);

char *player_get_lvl_desc(player *p);

/* macros */

#define player_memory_of(p,pos) ((p)->memory[(p)->map->nlevel][(pos).y][(pos).x])
#define player_pos_visible(p,pos) ((p)->fov[(pos).y][(pos).x])

#endif
