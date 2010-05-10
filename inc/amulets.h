/*
 * amulets.h
 * Copyright (C) 2009, 2010 Joachim de Groot <jdegroot@web.de>
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

#ifndef __AMULETS_H_
#define __AMULETS_H_

#include "items.h"

enum amulet_types
{
    AM_NONE,
    AM_AWARENESS,
    AM_INVISIBILITY,
    AM_UNDEAD_CONTROL,
    AM_NEGATE_SPIRIT,
    AM_NOTHEFT,
    AM_DRAGON_SLAYING,
    AM_POWER,
    AM_REFLECTION,
    AM_LARN,
    AM_MAX
};

typedef struct amulet_data
{
    int id;
    char *name;
    effect_type effect; /* effect causes by this amulet */
    int price;          /* base price in the shops */
} amulet_data;

/* external vars */

extern const amulet_data amulets[AM_MAX];

/* function declarations */

item_material_t amulet_material(int amulet_id);

/* macros */

#define amulet_name(item)        (amulets[(item)->id].name)
#define amulet_effect_type(item) (amulets[(item)->id].effect)
#define amulet_price(item)       (amulets[(item)->id].price)

#endif
