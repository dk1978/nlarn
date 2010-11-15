/*
 * spells.h
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

#ifndef __SPELLS_H_
#define __SPELLS_H_

#include "defines.h"
#include "effects.h"
#include "items.h"
#include "player.h"

typedef enum spell_type
{
    SC_NONE,
    SC_PLAYER, /* affects the player */
    SC_POINT,  /* affects a single monster */
    SC_RAY,    /* creates a ray */
    SC_FLOOD,  /* effect pours like water */
    SC_BLAST,  /* effect occurs like an explosion */
    SC_OTHER,  /* unclassified */
    SC_MAX
} spell_t;

typedef struct spell_data {
    guint id;
    char *code;
    char *name;
    spell_t type;
    damage_t damage_type;
    effect_t effect;
    char *description;
    char *msg_success;
    char *msg_fail;
    int level;
    int price;
    unsigned
        obtainable: 1;  /* available in the shop */
} spell_data;

typedef struct spell {
    guint id;         /* reference to spell_data */
    guint knowledge;  /* quality of knowledge */
    guint used;       /* usage counter */
} spell;

typedef enum spell_ids {
    SP_NONE,
    SP_PRO,         /* protection */
    SP_MLE,         /* magic missile */
    SP_DEX,         /* dexterity */
    SP_SLE,         /* sleep */
    SP_CHM,         /* charm monster */
    SP_SSP,         /* sonic spear */
    SP_STR,         /* strength */
    SP_CPO,         /* cure poison */
    SP_HEL,         /* healing */
    SP_CBL,         /* cure blindness */
    SP_CRE,         /* create monster */
    SP_PHA,         /* phantasmal forces */
    SP_INV,         /* invisibility */
    SP_BAL,         /* fireball */
    SP_CLD,         /* cold */
    SP_PLY,         /* polymorph */
    SP_CAN,         /* cancellation */
    SP_HAS,         /* haste self */
    SP_CKL,         /* cloud kill */
    SP_VPR,         /* vaporize rock */
    SP_DRY,         /* dehydration */
    SP_LIT,         /* lightning */
    SP_DRL,         /* drain life */
    SP_GLO,         /* globe of invulnerability */
    SP_FLO,         /* flood */
    SP_FGR,         /* finger of death */
    SP_SCA,         /* scare monster */
    SP_HLD,         /* hold monster */
    SP_STP,         /* time stop */
    SP_TEL,         /* teleport */
    SP_MFI,         /* magic fire */
    SP_MKW,         /* make wall */
    SP_SPH,         /* sphere of annihilation */
    SP_SUM,         /* summon demon */
    SP_WTW,         /* walk through walls */
    SP_ALT,         /* alter reality */
    SP_MAX_BOOK,    /* last known spell */
    /* monster spells */
    SP_MON_FIRE = SP_MAX_BOOK,    /* burst of fire */
    SP_MON_PSY,     /* psionic blast */
    SP_MON_POISON,  /* poisonous fumes */
    SP_MAX
} spell_id;

/* external vars */

extern const spell_data spells[SP_MAX];

/* function definitions */

spell *spell_new(int id);
void spell_destroy(spell *s);

cJSON *spell_serialize(spell *s);
spell *spell_deserialize(cJSON *sser);
cJSON *spells_serialize(GPtrArray *sparr);
GPtrArray *spells_deserialize(cJSON *sser);

int spell_sort(gconstpointer a, gconstpointer b);

/**
 * Select a spell to cast and cast it
 *
 * @param the player
 * @return number of turns elapsed
 */
int spell_cast(struct player *p);

/**
 * Try to add a spell to the list of known spells
 *
 * @param the player
 * @param id of spell to learn
 * @return FALSE if learning the spell failed, otherwise level of knowledge
 */
int spell_learn(struct player *p, guint spell_type);

/**
 * Remove a spell from the list of known spells
 *
 * @param the player
 * @param the id of the spell to forget
 * @return TRUE if the spell could be found and removed, othrwise FALSE
 */
int spell_forget(struct player *p, guint spell_type);

/**
 * Check if a spell is known to the player
 *
 * @param the player
 * @param id of the spell in question
 * @return FALSE if unknown, otherwise level of knowledge of that spell
 */
int spell_known(struct player *p, guint spell_type);

position throw_ray(spell *sp, struct player *p, position start, position target,
                   int damage, gboolean player_cast);

int spell_type_player(spell *s, struct player *p);
int spell_type_point(spell *s, struct player *p);
int spell_type_ray(spell *s, struct player *p);
int spell_type_flood(spell *s, struct player *p);
int spell_type_blast(spell *s, struct player *p);

gboolean spell_alter_reality(struct player *p);
gboolean spell_create_monster(struct player *p);
gboolean spell_create_sphere(spell *s, struct player *p);
gboolean spell_cure_poison(struct player *p);
gboolean spell_cure_blindness(struct player *p);
gboolean spell_phantasmal_forces(spell *s, struct player *p);
gboolean spell_scare_monsters(spell *s, struct player *p);
gboolean spell_summon_demon(spell *s, struct player *p);
gboolean spell_make_wall(struct player *p);
gboolean spell_vaporize_rock(struct player *p);

#define spell_code(spell)     (spells[(spell)->id].code)
#define spell_name(spell)     (spells[(spell)->id].name)
#define spell_type(spell)     (spells[(spell)->id].type)
#define spell_damage(spell)   (spells[(spell)->id].damage_type)
#define spell_effect(spell)   (spells[(spell)->id].effect)
#define spell_desc(spell)     (spells[(spell)->id].description)
#define spell_msg_succ(spell) (spells[(spell)->id].msg_success)
#define spell_msg_fail(spell) (spells[(spell)->id].msg_fail)
#define spell_level(spell)    (spells[(spell)->id].level)

#define spell_code_by_id(id)     (spells[(id)].code)
#define spell_name_by_id(id)     (spells[(id)].name)
#define spell_type_by_id(id)     (spells[(id)].type)
#define spell_damage_by_id(id)   (spells[(id)].damage_type)
#define spell_effect_by_id(id)   (spells[(id)].effect)
#define spell_desc_by_id(id)     (spells[(id)].description)
#define spell_msg_succ_by_id(id) (spells[(id)].msg_success)
#define spell_msg_fail_by_id(id) (spells[(id)].msg_fail)
#define spell_level_by_id(id)    (spells[(id)].level)

/* *** BOOKS *** */

char *book_desc(int book_id);
int book_weight(item *book);
int book_colour(item *book);
item_usage_result book_read(struct player *p, item *book);

#define book_type_obtainable(id) (spells[id].obtainable)

#define book_name(book)   (spells[(book)->id].name)
#define book_price(book)  (spells[(book)->id].price)

#endif
