/*
 * items.h
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

#ifndef __ITEM_H_
#define __ITEM_H_

#include <glib.h>

#include "effects.h"

typedef enum item_types {
    IT_NONE,
    IT_AMULET,          /* amulet, defined in amulets.h */
    IT_ARMOUR,          /* armour, defined in armour.h */
    IT_BOOK,            /* book, defined in spells.h */
    IT_CONTAINER,       /* container, defined in container.h */
    IT_FOOD,            /* food, defined in food.h */
    IT_GEM,             /* gem, defined in gems.h */
    IT_GOLD,            /* just gold. defined nowhere as type and count are sufficient. */
    IT_POTION,          /* potion, defined in potions.h */
    IT_RING,            /* ring, defined in rings.h */
    IT_SCROLL,          /* magic_scroll, defined in scrolls.h */
    IT_WEAPON,          /* weapon, defined in weapons.h */
    IT_MAX,             /* ~ item type count */
    IT_ALL             /* for special uses: all item types */
} item_t;

/* inspired by Nethack's objclass.h */
typedef enum item_material_t {
	IM_NONE,
	IM_PAPER,
	IM_CLOTH,
	IM_LEATHER,
	IM_WOOD,
	IM_BONE,
	IM_DRAGON_HIDE,		/* not leather! */
	IM_IRON,			/* Fe  */
	IM_STEEL,           /* stainless steel */
	IM_COPPER,			/* Cu - includes brass */
	IM_SILVER,
	IM_GOLD,			/* Au */
	IM_PLATINUM,		/* Pt */
	IM_MITHRIL,
	IM_GLASS,
	IM_GEMSTONE,
	IM_MAX				/* ~ item material count */
} item_material_t;

typedef struct item_material_data {
    item_material_t type;
    char *name;
    char *adjective;
} item_material_data;

/* FIXME: better name, better everything */
enum perish_types {
	PT_NONE,
	PT_FIRE,
	PT_WATER,
	PT_ACID,
	PT_MAX
};

enum perish_impact {
	PI_NONE,
	PI_ENFORCED,
	PI_DESTROYED,
	PI_MAX
};

struct _inventory;
struct _item;

typedef gint (*inv_callback_bool) (struct _inventory *inv, struct _item  *item);
typedef void (*inv_callback_void) (struct _inventory *inv, struct _item  *item);

typedef struct _inventory
{
    inv_callback_bool pre_add;
    inv_callback_void post_add;
    inv_callback_bool pre_del;
    inv_callback_void post_del;
    gconstpointer owner;
    GPtrArray *content;
} inventory;

typedef struct _item {
    item_t type;            /* element type */
    guint32 id;             /* item id, type specific */
    gint32 bonus;
    guint32 count;          /* for stackable items */
    GPtrArray *effects;     /* storage for effects */
    inventory *content;     /* for containers */
    guint32
        blessed: 1,
        cursed: 1,
        corroded: 2,        /* 0: no; 1: yes; 2: very */
        burnt: 2,           /* 0: no; 1: yes; 2: very */
        rusty: 2,           /* 0: no; 1: yes; 2: very */
        blessed_known: 1,   /* player known that item is blessed */
        bonus_known: 1,     /* player knows the bonus */
        curse_known: 1;     /* player knows that item is cursed */
} item;

typedef struct item_type_data {
	item_t id;
	char *name_sg;
	char *name_pl;
	char image;
	char *desc_known;
	char *desc_unknown;
	int max_id;
	unsigned
		equippable: 1,
		usable: 1,
		stackable: 1;
} item_type_data;

/* function definitions */

item *item_new(item_t item_type, int item_id, int item_bonus);
item *item_new_random(item_t item_type);
item *item_new_by_level(item_t item_type, int num_level);
item *item_copy(item *original);
item *item_split(item *original, guint32 count);
void item_destroy(item *it);

int item_compare(item *a, item *b);
int item_sort(gconstpointer a, gconstpointer b, gpointer data);
char *item_describe(item *it, int known, int singular, int definite, char *str, int str_len);
item_material_t item_material(item *it);
guint item_price(item *it);
int item_weight(item *it);

void item_effect_add(item *i, effect *e);
void item_effect_del(item *i, effect *e);

int item_bless(item *it);
int item_curse(item *it);
int item_remove_blessing(item *it);
int item_remove_curse(item *it);

int item_enchant(item *it);
int item_disenchant(item *it);
int item_rust(item *it);
int item_corrode(item *it);
int item_burn(item *it);

/* external vars */
extern const item_type_data item_data[IT_MAX];
extern const item_material_data item_materials[IM_MAX];

/* item macros */
#define item_image(type)          item_data[(type)].image
#define item_name_sg(type)        item_data[(type)].name_sg
#define item_name_pl(type)        item_data[(type)].name_pl
#define item_max_id(type)         item_data[(type)].max_id
#define item_is_equippable(type)      item_data[(type)].equippable
#define item_is_usable(type)          item_data[(type)].usable
#define item_is_stackable(type)       item_data[(type)].stackable
#define item_material_name(type)      item_materials[(type)].name
#define item_material_adjective(type) item_materials[(type)].adjective

/* inventory functions */

inventory *inv_new(gconstpointer owner);
void inv_destroy(inventory *inv);

void inv_callbacks_set(inventory *inv, inv_callback_bool pre_add,
                       inv_callback_void post_add, inv_callback_bool pre_del,
                       inv_callback_void post_del);

int inv_add(inventory **inv, item *item_new);
item *inv_get(inventory *inv, int idx);
item *inv_del(inventory **inv, guint idx);
int inv_del_element(inventory **inv, item *item);
int inv_length(inventory *inv);
void inv_sort(inventory *inv, GCompareDataFunc compare_func, gpointer user_data);
int inv_weight(inventory *inv);
int inv_item_count(inventory *inv, item_t type, guint32 id);

int inv_length_filtered(inventory *inv, int (*filter)(item *));
item *inv_get_filtered(inventory *inv, guint idx, int (*filter)(item *));

int inv_filter_container(item *it);
int inv_filter_not_container(item *it);
int inv_filter_food(item *it);
int inv_filter_gems(item *it);
int inv_filter_not_gold(item *it);
int inv_filter_potions(item *it);
int inv_filter_readable_items(item *it);

#endif
