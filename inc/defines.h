/*
 * defines.h
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

#ifndef __DEFINES_H_
#define __DEFINES_H_

typedef enum _speed
{
    SPEED_NONE,
    SPEED_SLOW,
    SPEED_NORMAL,
    SPEED_FAST,
    SPEED_MAX
} speed;

typedef enum _esize
{
    ESIZE_NONE,
    ESIZE_TINY,
    ESIZE_SMALL,
    ESIZE_MEDIUM,
    ESIZE_LARGE,
    ESIZE_HUGE,
    ESIZE_GARGANTUAN,
    ESIZE_MAX
} esize;

typedef enum _attack_types
{
    ATT_NONE,
    ATT_WEAPON,
    ATT_MAGIC,  /* e.g. psionics */
    ATT_CLAW,   /* some dragons */
    ATT_BITE,   /* bugbear, osequip, snake */
    ATT_STING,  /* ant, centipede */
    ATT_SLAM,   /* shambling mound */
    ATT_KICK,   /* centaur? */
    ATT_TOUCH,  /* vampire, wraith */
    ATT_BREATH, /* dragons, hellhound */
    ATT_GAZE,   /* floating eye */
    ATT_MAX
} attack_t;

typedef enum _damage_types
{
    DAM_NONE,
    DAM_PHYSICAL,
    DAM_MAGICAL,        /* e.g. magic missile */

    /* elements */
    DAM_FIRE,
    DAM_COLD,
    DAM_ACID,
    DAM_WATER,
    DAM_ELECTRICITY,

    /* effects */
    DAM_POISON,         /* traps, snake */
    DAM_BLINDNESS,
    DAM_CONFUSION,      /* umber hulk */
    DAM_PARALYSIS,      /* floating eye */
    DAM_DEC_STR,        /* ant, centipede */
    DAM_DEC_DEX,        /* quasit */
    DAM_DRAIN_LIFE,     /* vampire, wraith */

    /* inventory manipulation */
    /* these damage types are handled by the monster, not the player */
    DAM_STEAL_GOLD,     /* leprechaun */
    DAM_STEAL_ITEM,     /* nymph */
    DAM_RUST,           /* rust monster, gelatious cube */
    DAM_REM_ENCH,       /* disenchantress */
    DAM_MAX,
    DAM_RANDOM          /* random damage: spirit naga */
} damage_t;

typedef struct _attack
{
    attack_t type;
    damage_t damage;
    int base;
    int rand;
} attack;

typedef struct _damage
{
    damage_t type;
    int amount;
    gpointer originator; /* pointer to player / monster that caused the damage */
} damage;

typedef struct _damage_msg
{
    char *msg_affected;
    char *msg_unaffected;
} damage_msg;

#endif
