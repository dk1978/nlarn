/*
 * spells.c
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

const spell_data spells[SP_MAX] =
{
    {
        SP_NONE,
        NULL,
        NULL,
        SC_NONE,
        ET_NONE,
        NULL,
        NULL,
        NULL,
        0,
        0
    },
    {
        SP_PRO,
        "pro",
        "protection",
        SC_PLAYER,
        ET_PROTECTION,
        "Generates a protection field",
        NULL,
        NULL,
        1,
        260
    },
    {
        SP_MLE,
        "mle",
        "magic missile",
        SC_RAY,
        ET_NONE,
        "Creates and hurls a magic missile equivalent to a + 1 magic arrow",
        "Your missiles hit the %s.",
        "Your missile bounces off the %s.",
        1,
        320
    },
    {
        SP_DEX,
        "dex",
        "dexterity",
        SC_PLAYER,
        ET_INC_DEX,
        "Improves the casters dexterity",
        NULL,
        NULL,
        1,
        260
    },
    {
        SP_SLE,
        "sle",
        "sleep",
        SC_POINT,
        ET_SLEEP,
        "causes some monsters to go to sleep",
        NULL,
        "The %s doesn't sleep.",
        1,
        260
    },
    {
        SP_CHM,
        "chm",
        "charm monster",
        SC_PLAYER,
        ET_CHARM_MONSTER,
        "some monsters may be awed at your magnificence",
        NULL,
        NULL,
        1,
        260
    },
    {
        SP_SSP,
        "ssp",
        "sonic spear",
        SC_RAY,
        ET_NONE,
        "causes your hands to emit a screeching sound toward what they point",
        "The sound damages the %s.",
        "The %s can't hear the noise.",
        1,
        300
    },
    {
        SP_STR,
        "str",
        "strength",
        SC_PLAYER,
        ET_INC_STR,
        "Increase the casters strength for a short term",
        NULL,
        NULL,
        2,
        460
    },
    {
        SP_ENL,
        "enl",
        "enlightenment",
        SC_PLAYER,
        ET_ENLIGHTENMENT,
        "the caster becomes aware of things around him",
        NULL,
        NULL,
        2,
        460
    },
    {
        SP_HEL,
        "hel",
        "healing",
        SC_PLAYER,
        ET_INC_HP,
        "restores some hp to the caster",
        NULL,
        NULL,
        2,
        400
    },
    {
        SP_CBL,
        "cbl",
        "cure blindness",
        SC_OTHER,
        ET_NONE,
        "restores sight to one so unfortunate as to be blinded",
        NULL,
        NULL,
        2,
        400
    },
    {
        SP_CRE,
        "cre",
        "create monster",
        SC_OTHER,
        ET_NONE,
        "creates a monster near the caster appropriate for the location",
        NULL,
        NULL,
        2,
        400
    },
    {
        SP_PHA,
        "pha",
        "phantasmal forces",
        SC_POINT,
        ET_SCARE_MONSTER,
        "creates illusions, and if believed, monsters flee",
        "The %s believed!",
        "The %s didn't believe the illusions!",
        2,
        600
    },
    {
        SP_INV,
        "inv",
        "invisibility",
        SC_PLAYER,
        ET_INVISIBILITY,
        "the caster becomes invisible",
        NULL,
        NULL,
        2,
        600
    },
    {
        SP_BAL,
        "bal",
        "fireball",
        SC_BLAST,
        ET_NONE,
        "makes a ball of fire that burns on what it hits",
        "The fireball hits the %s.",
        NULL,
        3,
        1200
    },
    {
        SP_CLD,
        "cld",
        "cone of cold",
        SC_RAY,
        ET_NONE,
        "sends forth a cone of cold which freezes what it touches",
        "The cone of cold strikes the %s.",
        "The %s loves the cold!",
        3,
        1200
    },
    {
        SP_PLY,
        "ply",
        "polymorph",
        SC_POINT,
        ET_NONE,
        "you can find out what this does for yourself",
        NULL,
        "The %s resists.",
        3,
        950
    },
    {
        SP_CAN,
        "can",
        "cancellation",
        SC_PLAYER,
        ET_CANCELLATION,
        "negates the ability of a monster to use his special abilities",
        NULL,
        NULL,
        3,
        950
    },
    {
        SP_HAS,
        "has",
        "haste self",
        SC_PLAYER,
        ET_SPEED,
        "speeds up the casters movements",
        NULL,
        NULL,
        3,
        950
    },
    {
        SP_CKL,
        "ckl",
        "killing cloud",
        SC_FLOOD,
        ET_NONE,
        "creates a fog of poisonous gas which kills all that is within it",
        "The %s gasps for air.",
        "The %s loves the water!",
        3,
        1200
    },
    {
        SP_VPR,
        "vpr",
        "vaporize rock",
        SC_OTHER,
        ET_NONE,
        "this changes rock to air",
        NULL,
        NULL,
        3,
        950
    },
    {
        SP_DRY,
        "dry",
        "dehydration",
        SC_POINT,
        ET_NONE,
        "dries up water in the immediate vicinity",
        "The %s shrivels up.",
        "The %s isn't affected.",
        4,
        1600
    },
    {
        SP_LIT,
        "lit",
        "lightning",
        SC_RAY,
        ET_NONE,
        "you finger will emit a lightning bolt when this spell is cast",
        "A lightning bolt hits the %s.",
        "The %s loves fire and lightning!",
        4,
        1600
    },
    {
        SP_DRL,
        "drl",
        "drain life",
        SC_POINT,
        ET_NONE,
        "subtracts hit points from both you and a monster",
        NULL,
        NULL,
        4,
        1400
    },
    {
        SP_GLO,
        "glo",
        "invulnerability",
        SC_PLAYER,
        ET_INVULNERABILITY,
        "this globe helps to protect the player from physical attack",
        NULL,
        NULL,
        4,
        1400
    },
    {
        SP_FLO,
        "flo",
        "flood",
        SC_FLOOD,
        ET_NONE,
        "this creates an avalanche of H2O to flood the immediate chamber",
        "The %s struggles for air in your flood!",
        NULL,
        4,
        1600
    },
    {
        SP_FGR,
        "fgr",
        "finger of death",
        SC_POINT,
        ET_NONE,
        "this is a holy spell and calls upon your god to back you up",
        "The %s's heart stopped.",
        "The %s isn't affected.",
        4,
        1600
    },
    {
        SP_SCA,
        "sca",
        "scare monster",
        SC_POINT,
        ET_SCARE_MONSTER,
        "terrifies the monster so that hopefully he wont hit the magic user",
        NULL,
        NULL,
        5,
        2000
    },
    {
        SP_HLD,
        "hld",
        "hold monster",
        SC_POINT,
        ET_HOLD_MONSTER,
        "the monster is frozen in his tracks if this is successful",
        NULL,
        NULL,
        5,
        2000
    },
    {
        SP_STP,
        "stp",
        "time stop",
        SC_OTHER,
        ET_TIMESTOP,
        "all movement in the caverns ceases for a limited duration",
        NULL,
        NULL,
        5,
        2500
    },
    {
        SP_TEL,
        "tel",
        "teleport away",
        SC_POINT,
        ET_NONE,
        "moves a particular monster around in the dungeon",
        NULL,
        NULL,
        5,
        2000
    },
    {
        SP_MFI,
        "mfi",
        "magic fire",
        SC_FLOOD,
        ET_NONE,
        "this causes a curtain of fire to appear all around you",
        "The %s cringes from the flame.",
        NULL,
        5,
        2500
    },
    {
        SP_MKW,
        "mkw",
        "make wall",
        SC_OTHER,
        ET_NONE,
        "Makes a wall in the specified place",
        NULL,
        NULL,
        6,
        3000
    },
    {
        SP_SPH,
        "sph",
        "sphere of annihilation",
        SC_OTHER,
        ET_NONE,
        "anything caught in this sphere is instantly killed.",
        NULL,
        NULL,
        6,
        3500
    },
    {
        SP_GEN,
        "gen",
        "genocide",
        SC_OTHER,
        ET_NONE,
        "eliminates a species of monster from the game -- use sparingly",
        NULL,
        NULL,
        6,
        3800
    },
    {
        SP_SUM,
        "sum",
        "summon demon",
        SC_OTHER,
        ET_NONE,
        "summons a demon who hopefully helps you out",
        NULL,
        NULL,
        6,
        3500
    },
    {
        SP_WTW,
        "wtw",
        "walk through walls",
        SC_PLAYER,
        ET_WALL_WALK,
        "allows the caster to walk through walls for a short period of time",
        NULL,
        NULL,
        6,
        3800
    },
    {
        SP_ALT,
        "alt",
        "alter reality",
        SC_OTHER,
        ET_NONE,
        "god only knows what this will do",
        NULL,
        "Polinneaus won't let you mess with his dungeon!",
        6,
        3800
    },
    {
        SP_PER,
        "per",
        "permanence",
        SC_OTHER,
        ET_NONE,
        "makes a character spell permanent, i. e. protection, strength, etc.",
        NULL,
        NULL,
        6,
        3800
    },
};

static int book_desc_mapping[SP_MAX - 1] = { 0 };

static const char *book_descriptions[SP_MAX - 1] =
{
    "parchment-bound",
    "thick",
    "dusty",
    "leather-bound",
    "heavy",
    "ancient",
    "buckram",
    "gilded",
    "embossed",
    "old",
    "thin",
    "light",
    "large",
    "vellum",
    "tan",
    "papyrus",
    "linen",
    "paper",
    "musty",
    "faded",
    "antique",
    "worn out",
    "tattered",
    "aged",
    "ornate",
    "inconspicuous",
    "awe-inspiring",
    "stained",
    "mottled",
    "plaid",
    "wax-lined",
    "bamboo",
    "clasped",
    "ragged",
    "dull",
    "canvas",
    "well-thumbed",
    "chambray",
};


spell *spell_new(int id)
{
    spell *nspell;

    assert(id > SP_NONE && id < SP_MAX);

    nspell = g_malloc0(sizeof(spell));
    nspell->id = id;
    nspell->knowledge = 1;

    return nspell;
}

void spell_destroy(spell *s)
{
    assert(s != NULL);
    g_free(s);
}

int spell_sort(gconstpointer a, gconstpointer b)
{
    gint order;
    spell *spell_a = *((spell**)a);
    spell *spell_b = *((spell**)b);

    if (spell_a->id == spell_b->id)
        order = 0;
    else
        order = strcmp(spell_code(spell_a), spell_code(spell_b));

    return order;
}

void spell_alter_reality(player *p)
{
    level *nlevel, *olevel;

    olevel = p->level;

    /* create new level */
    nlevel = g_malloc0(sizeof (level));
    nlevel->nlevel = olevel->nlevel;

    level_new(nlevel,
              game_difficulty(p->game),
              game_mazefile(p->game));

    /* make new level active */
    p->game->levels[p->level->nlevel] = nlevel;
    p->level = nlevel;

    /* reposition player (if needed) */
    if (!level_pos_passable(nlevel, p->pos))
    {
        p->pos = level_find_space(nlevel, LE_MONSTER);
    }

    /* destroy old level */
    level_destroy(olevel);
}

int spell_create_monster(player *p)
{
    monster *m;

    position pos;

    /* this spell doesn't work in town */
    if (p->level->nlevel == 0)
    {
        log_add_entry(p->log, "Nothing happens.");
        return FALSE;
    }

    /* try to find a space for the monster near the player */
    pos = level_find_space_in(p->level,
                              rect_new_sized(p->pos, 2),
                              LE_MONSTER);

    if (pos_valid(pos))
    {
        m = monster_new_by_level(p->level);
        monster_position(m, pos);

        return TRUE;
    }
    else
    {
        log_add_entry(p->log, "You feel failure.");
        return FALSE;
    }
}

void spell_create_sphere(player *p)
{
    position pos;

    assert(p != NULL);

    pos = display_get_position(p, "Where do you want to place the sphere?",
                               FALSE, TRUE);

    if (pos_valid(pos))
    {
        g_ptr_array_add(p->level->slist, sphere_new(pos, p, p->lvl * 10));
    }
    else
    {
        log_add_entry(p->log, "Huh?");
    }
}

void spell_genocide_monster(player *p)
{
    char in;
    int id;

    assert(p != NULL);

    log_add_entry(p->log, "Whih monster do you want to genocide (type letter)?");
    display_paint_screen(p);

    in = getch();

    for (id = 1; id < MT_MAX; id++)
    {
        if (monster_get_image_by_type(id) == in)
        {
            if (!monster_is_genocided(id))
            {
                monster_genocide(id);
                log_add_entry(p->log,
                              "Wiped out all %ss",
                              monster_get_name_by_type(id));

                monsters_genocide(p->level);
            }

            return;
        }
    }

    log_add_entry(p->log, "No such monster.");
}

void spell_make_wall(player *p)
{
    position pos;

    pos = display_get_position(p, "Select a position where you want to place a wall.", FALSE, TRUE);

    if (pos_identical(pos, p->pos))
    {
        log_add_entry(p->log, "You are actually standing there.");
        return;
    }
    else if (!pos_valid(pos))
    {
        log_add_entry(p->log, "No wall today.");
        return;
    }

    if (level_tiletype_at(p->level, pos) != LT_WALL)
    {
        level_tiletype_at(p->level, pos) = LT_WALL;

        /* destroy all items at that position */
        if (level_ilist_at(p->level, pos))
        {
            inv_destroy(level_ilist_at(p->level, pos));
            level_ilist_at(p->level, pos) = NULL;
        }

        log_add_entry(p->log, "You have created a wall.");
    }
    else
    {
        log_add_entry(p->log, "There was a wall already..");
    }
}

void spell_vaporize_rock(player *p)
{
    position pos;
    monster *m = NULL;
    char *desc = NULL;

    pos = display_get_position(p, "What do you want to vaporize?", FALSE, FALSE);

    if (!pos_valid(pos))
    {
        log_add_entry(p->log, "So you chose not to vaprize anything.");
        return;
    }

    if (level_tiletype_at(p->level, pos) == LT_WALL)
    {
        level_tiletype_at(p->level, pos) = LT_FLOOR;
        return;
    }

    if ((m = level_get_monster_at(p->level, pos)) && (m->type == MT_XORN))
    {
        /* xorns take damage from vpr */
        if (monster_hp_lose(m, divert(200, 10)))
        {
            player_monster_kill(p, m);
        }
    }

    switch (level_stationary_at(p->level, pos))
    {
    case LS_ALTAR:
        m = monster_new(MT_DAEMON_PRINCE, p->level);
        desc = "altar";
        break;

    case LS_FOUNTAIN:
        m = monster_new(MT_WATER_LORD, p->level);
        desc = "fountain";
        break;

    case LS_STATUE:
        if (game_difficulty(p->game) < 3)
        {
            if (!level_ilist_at(p->level, pos))
                level_ilist_at(p->level, pos) = inv_new();

            inv_add(level_ilist_at(p->level, pos),
                    item_new(IT_BOOK, rand_1n(SP_MAX - 1), 0));
        }

        desc = "statue";
        break;

    case LS_THRONE:
    case LS_THRONE2:
        m = monster_new(MT_GNOME_KING, p->level);
        desc = "throne";
        break;

    case LS_DEADFOUNTAIN:
    case LS_DEADTHRONE:
        level_stationary_at(p->level, pos) = LS_NONE;
        break;

    default:
        log_add_entry(p->log, "Somehow that did not work.");
        /* NOP */
    }

    if (desc)
    {
        log_add_entry(p->log, "You destroy the %s.", desc);
        level_stationary_at(p->level, pos) = LS_NONE;
    }

    /* created a monster - position it correctly */
    if (m)
    {
        monster_position(m, pos);
    }
}

void book_desc_shuffle()
{
    shuffle(book_desc_mapping, SP_MAX - 1, 0);
}

char *book_desc(int book_id)
{
    assert(book_id > SP_NONE && book_id < SP_MAX);
    return (char *)book_descriptions[book_desc_mapping[book_id - 1]];
}
