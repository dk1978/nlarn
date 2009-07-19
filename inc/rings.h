/*
 * rings.h
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

#ifndef __RINGS_H_
#define __RINGS_H_

#include "effects.h"

enum ring_types
{
    RT_NONE,
    RT_EXTRA_REGEN,
    RT_REGENERATION,
    RT_PROTECTION,
    RT_ENERGY,
    RT_DEXTERITY,
    RT_STRENGTH,
    RT_CLEVERNESS,
    RT_INC_DAMAGE,
    RT_MAX
};

typedef struct ring_data
{
    int id;
    char *name;
    int	effect_type; 	/* effect causes by this ring */
    int price;
    unsigned
        observable: 1;  /* can be identified by using */
} ring_data;

/* function definitions */

void ring_material_shuffle();
item_material_t ring_material(int ring_id);

/* external vars */

extern const ring_data rings[RT_MAX];

/* macros */

#define ring_name(item)          (rings[(item)->id].name)
#define ring_effect_type(item)   (rings[(item)->id].effect_type)
#define ring_price(item)         (rings[(item)->id].price)
#define ring_is_observable(item) (rings[(item)->id].observable)

#endif
