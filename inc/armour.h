/*
 * armour.h
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

#ifndef __ARMOUR_H_
#define __ARMOUR_H_

#include "items.h"

typedef enum armour_categories {
	AC_NONE,
	AC_BOOTS,
	AC_CLOAK,
	AC_GLOVES,
	AC_HELMET,
	AC_SHIELD,
	AC_SUIT,
	AC_MAX
} armour_cat;

enum armour_types {
	AT_NONE,
	AT_SHIELD,
	AT_LEATHER,
	AT_SLEATHER,
	AT_RIGNMAIL,
	AT_CHAINMAIL,
	AT_SPLINTMAIL,
	AT_PLATEMAIL,
	AT_PLATE,
	AT_SPLATE,
	AT_ELVENCHAIN,
	AT_MAX
};

typedef struct armour_data {
	int id;
	char *name;
	int ac;
    armour_cat category;
	int	material;       /* material type from item_materials */
	int weight;         /* used to determine inventory weight and if item can be thrown */
	int price;          /* base price in the shops */
	unsigned
		unique: 		1;	/* unique */
} armour_data;

/* external vars */

extern const armour_data armours[AT_MAX];

/* macros */

#define armour_name(armour)     (armours[(armour)->id].name)
#define armour_ac(armour)       (armours[(armour)->id].ac + (armour)->bonus)
#define armour_category(armour) (armours[(armour)->id].category)
#define armour_material(armour) (armours[(armour)->id].material)
#define armour_weight(armour)   (armours[(armour)->id].weight)
#define armour_price(armour)    (armours[(armour)->id].price)
#define armour_uniqe(armour)    (armours[(armour)->id].unique)

#endif
