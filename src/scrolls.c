/*
 * scrolls.c
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

#include <assert.h>
#include <stdlib.h>

#include "display.h"
#include "game.h"
#include "nlarn.h"
#include "scrolls.h"

const magic_scroll_data scrolls[ST_MAX] =
{
    /* ID                   name                  effect               price obtainable */
    { ST_NONE,              "",                   ET_NONE,                 0, FALSE },
    { ST_ENCH_ARMOUR,       "enchant armour",     ET_NONE,               100,  TRUE },
    { ST_ENCH_WEAPON,       "enchant weapon",     ET_NONE,               125,  TRUE },
    { ST_ENLIGHTENMENT,     "enlightenment",      ET_ENLIGHTENMENT,       60,  TRUE },
    { ST_BLANK,             "blank paper",        ET_NONE,                10, FALSE },
    { ST_CREATE_MONSTER,    "create monster",     ET_NONE,               100, FALSE },
    { ST_CREATE_ARTIFACT,   "create artifact",    ET_NONE,               200, FALSE },
    { ST_AGGRAVATE_MONSTER, "aggravate monsters", ET_AGGRAVATE_MONSTER,  110, FALSE },
    { ST_TIMEWARP,          "time warp",          ET_NONE,               500, FALSE },
    { ST_TELEPORT,          "teleportation",      ET_NONE,               200,  TRUE },
    { ST_AWARENESS,         "expanded awareness", ET_AWARENESS,          250,  TRUE },
    { ST_SPEED,             "speed",              ET_SPEED,              200, FALSE },
    { ST_HEAL_MONSTER,      "monster healing",    ET_NONE,                30, FALSE },
    { ST_SPIRIT_PROTECTION, "spirit protection",  ET_SPIRIT_PROTECTION,  340,  TRUE },
    { ST_UNDEAD_PROTECTION, "undead protection",  ET_UNDEAD_PROTECTION,  340,  TRUE },
    { ST_STEALTH,           "stealth",            ET_STEALTH,            300,  TRUE },
    { ST_MAPPING,           "magic mapping",      ET_NONE,               400,  TRUE },
    { ST_HOLD_MONSTER,      "hold monsters",      ET_HOLD_MONSTER,       500, FALSE },
    { ST_GEM_PERFECTION,    "gem perfection",     ET_NONE,              1000, FALSE },
    { ST_SPELL_EXTENSION,   "spell extension",    ET_NONE,               500, FALSE },
    { ST_IDENTIFY,          "identify",           ET_NONE,               340,  TRUE },
    { ST_REMOVE_CURSE,      "remove curse",       ET_NONE,               220,  TRUE },
    { ST_ANNIHILATION,      "annihilation",       ET_NONE,              3900, FALSE },
    { ST_PULVERIZATION,     "pulverization",      ET_NONE,               610,  TRUE },
    { ST_LIFE_PROTECTION,   "life protection",    ET_LIFE_PROTECTION,   3000, FALSE },
    { ST_GENOCIDE_MONSTER,  "genocide monster",   ET_NONE,              3800, FALSE },
};

static int scroll_with_effect(player *p, item *scroll);
static int scroll_annihilate(player *p, item *scroll);
static int scroll_create_artefact(player *p, item *scroll);
static int scroll_enchant_armour(player *p, item *scroll);
static int scroll_enchant_weapon(player *p, item *scroll);
static int scroll_gem_perfection(player *p, item *scroll);
static int scroll_genocide_monster(player *p, item *scroll);
static int scroll_heal_monster(player *p, item *scroll);
static int scroll_identify(player *p, item *scroll);
static int scroll_remove_curse(player *p, item *scroll);
static int scroll_spell_extension(player *p, item *scroll);
static int scroll_teleport(player *p, item *scroll);
static int scroll_timewarp(player *p, item *scroll);

static const char *_scroll_desc[ST_MAX - 1] =
{
    "Ssyliir Wyleeum",
    "Etzak Biqolix",
    "Tzaqa Chanim",
    "Lanaj Lanyesaj",
    "Azayal Ixasich",
    "Assossasda",
    "Sondassasta",
    "Mindim Lanak",
    "Sudecke Chadosia",
    "L'sal Chaj Izjen",
    "Assosiala",
    "Lassostisda",
    "Bloerdadarsya",
    "Chadosia",
    "Iskim Namaj",
    "Chamote Ajaqa",
    "Lirtilsa",
    "Undim Jiskistast",
    "Lirtosiala",
    "Frichassaya",
    "Undast Kabich",
    "Fril Ajich Lsosa",
    "Chados Azil Tzos",
    "Ixos Tzek Ajak",
    "Xodil Keterulo",
};

char *scroll_desc(int scroll_id)
{
    assert(scroll_id > ST_NONE && scroll_id < ST_MAX);
    return (char *)_scroll_desc[nlarn->scroll_desc_mapping[scroll_id - 1]];
}

item_usage_result scroll_read(struct player *p, item *scroll)
{
    char description[61];
    item_usage_result result;

    result.time = 2;
    result.used_up = TRUE;

    item_describe(scroll, player_item_known(p, scroll),
                  TRUE, TRUE, description, 60);

    if (player_effect(p, ET_BLINDNESS))
    {
        log_add_entry(p->log, "As you are blind you can't read %s.",
                      description);

        result.identified = FALSE;
        result.used_up = FALSE;

        return result;
    }

    log_add_entry(p->log, "You read %s.", description);

    /* increase number of scrolls read */
    p->stats.scrolls_read++;

    if (scroll->cursed)
    {
        damage *dam = damage_new(DAM_FIRE, ATT_NONE, rand_1n(p->hp), NULL);
        log_add_entry(p->log, "The scroll explodes!");
        player_damage_take(p, dam, PD_CURSE, scroll->type);
        result.identified = FALSE;
    }
    else
    {
        switch (scroll->id)
        {
        case ST_ENCH_ARMOUR:
            result.identified = scroll_enchant_armour(p, scroll);
            break;

        case ST_ENCH_WEAPON:
            result.identified = scroll_enchant_weapon(p, scroll);
            break;

        case ST_BLANK:
            result.used_up = FALSE;
            result.identified = TRUE;
            log_add_entry(p->log, "This scroll is blank.");
            break;

        case ST_CREATE_MONSTER:
            result.identified = spell_create_monster(p);
            break;

        case ST_CREATE_ARTIFACT:
            result.identified = scroll_create_artefact(p, scroll);
            break;

        case ST_TIMEWARP:
            result.identified = scroll_timewarp(p, scroll);
            break;

        case ST_TELEPORT:
            result.identified = scroll_teleport(p, scroll);
            break;

        case ST_HEAL_MONSTER:
            result.identified = scroll_heal_monster(p, scroll);
            break;

        case ST_MAPPING:
            log_add_entry(p->log, "There is a map on the scroll!");
            result.identified = scroll_mapping(p, scroll);
            break;

        case ST_GEM_PERFECTION:
            result.identified = scroll_gem_perfection(p, scroll);
            break;

        case ST_SPELL_EXTENSION:
            result.identified = scroll_spell_extension(p, scroll);
            break;

        case ST_IDENTIFY:
            result.identified = scroll_identify(p, scroll);
            break;

        case ST_REMOVE_CURSE:
            result.identified = scroll_remove_curse(p, scroll);
            break;

        case ST_ANNIHILATION:
            result.identified = scroll_annihilate(p, scroll);
            break;

        case ST_PULVERIZATION:
            if (!p->identified_scrolls[ST_PULVERIZATION])
            {
                log_add_entry(p->log, "This is a scroll of %s. ",
                              scroll_name(scroll));
            }

            if (spell_vaporize_rock(p))
            {
                /* recalc fov if something has been vaporised */
                player_update_fov(p);
            }

            result.identified = TRUE;
            break;

        case ST_GENOCIDE_MONSTER:
            scroll_genocide_monster(p, scroll);
            result.identified = TRUE;
            break;

        default:
            result.identified = scroll_with_effect(p, scroll);
            break;
        }

        if (!result.identified)
        {
            log_add_entry(p->log, "Nothing happens.");
        }
    }

    return result;
}

static int scroll_with_effect(struct player *p, item *scroll)
{
    effect *eff;

    assert(p != NULL && scroll != NULL);

    eff = effect_new(scroll_effect(scroll));
    eff = player_effect_add(p, eff);

    if (eff && !effect_get_msg_start(eff))
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * Scroll "annihilation".
 *
 * @param the player
 * @param the scroll just read
 *
 */
static int scroll_annihilate(struct player *p, item *scroll)
{
    int count = 0;
    area *blast, *obsmap;
    position cursor = p->pos;
    monster *m;
    map *cmap = game_map(nlarn, p->pos.z);

    assert(p != NULL && scroll != NULL);

    obsmap = map_get_obstacles(cmap, p->pos, 2);
    blast = area_new_circle_flooded(p->pos, 2, obsmap);

    for (cursor.y = blast->start_y; cursor.y < blast->start_y + blast->size_y; cursor.y++)
    {
        for (cursor.x = blast->start_x; cursor.x < blast->start_x + blast->size_x; cursor.x++)
        {
            if (area_pos_get(blast, cursor) && (m = map_get_monster_at(cmap, cursor)))
            {
                if (monster_is_demon(m))
                {
                    m = monster_damage_take(m, damage_new(DAM_MAGICAL, ATT_NONE, 2000, p));

                    /* check if the monster has been killed */
                    if (!m) count++;
                }
                else
                {
                    log_add_entry(p->log, "The %s barely escapes being annihilated.",
                                  monster_get_name(m));

                    /* lose half hit points*/
                    damage *dam = damage_new(DAM_MAGICAL, ATT_NONE, monster_hp(m) / 2, p);
                    monster_damage_take(m, dam);
                }
            }
        }
    }

    area_destroy(blast);

    if (count)
    {
        log_add_entry(p->log, "You hear loud screams of agony!");
    }

    return count;
}

static int scroll_create_artefact(player *p, item *scroll)
{
    item *it;
    char buf[61];

    assert(p != NULL && scroll != NULL);

    it = item_new_by_level(rand_1n(IT_MAX), p->pos.z);
    inv_add(map_ilist_at(game_map(nlarn, p->pos.z), p->pos), it);

    item_describe(it, player_item_known(p, it), (it->count == 1),
                  FALSE, buf, 60);

    log_add_entry(p->log, "You find %s below your feet.", buf);

    return TRUE;
}

/**
 * Scroll "enchant armour".
 *
 * @param the player
 * @param the scroll just read
 *
 */
static int scroll_enchant_armour(player *p, item *scroll)
{
    item **armour;

    assert(p != NULL && scroll != NULL);

    /* get a random piece of armour to enchant */
    if ((armour = player_get_random_armour(p)))
    {
        log_add_entry(p->log, "Your %s glows for a moment.",
                      armour_name(*armour));

        item_enchant(*armour);

        return TRUE;
    }

    return FALSE;
}

static int scroll_enchant_weapon(player *p, item *scroll)
{
    assert(p != NULL && scroll != NULL);

    if (p->eq_weapon)
    {
        log_add_entry(p->log,
                      "Your %s glisters for a moment.",
                      weapon_name(p->eq_weapon));

        item_enchant(p->eq_weapon);

        return TRUE;
    }

    return FALSE;
}

static int scroll_gem_perfection(player *p, item *scroll)
{
    guint idx;
    item *it;

    assert(p != NULL && scroll != NULL);

    if (inv_length_filtered(p->inventory, item_filter_gems) == 0)
    {
        return FALSE;
    }

    log_add_entry(p->log, "This is a scroll of gem perfection.");

    if (scroll->blessed)
    {
        for (idx = 0; idx < inv_length_filtered(p->inventory, item_filter_gems); idx++)
        {
            it = inv_get_filtered(p->inventory, idx, item_filter_gems);
            /* double gem value */
            it->bonus <<= 1;
        }
        log_add_entry(p->log, "You bring all your gems to perfection.");
    }
    else
    {
        it = display_inventory("Choose a gem to make perfect", p, &p->inventory, NULL,
                               FALSE, FALSE, FALSE, item_filter_gems);

        if (it)
        {
            char desc[81];
            item_describe(it, TRUE, it->count == 1, TRUE, desc, 80);
            log_add_entry(p->log, "You make %s perfect.", desc);

            /* double gem value */
            it->bonus <<= 1;
        }
    }

    return TRUE;
}

static int scroll_genocide_monster(player *p, item *scroll)
{
    char *in;
    int id;
    GString *msg;

    assert(p != NULL);

    msg = g_string_new(NULL);

    if (!p->identified_scrolls[ST_GENOCIDE_MONSTER])
    {
        g_string_append_printf(msg, "This is a scroll of %s. ",
                               scroll_name(scroll));
    }

    g_string_append(msg, "Which monster do you want to genocide (type letter)?");

    in = display_get_string(msg->str, NULL, 1);

    g_string_free(msg, TRUE);

    if (!in)
    {
        log_add_entry(p->log, "You chose not to genocide any monster.");
        return FALSE;
    }

    for (id = 1; id < MT_MAX; id++)
    {
        if (monster_type_image(id) == in[0])
        {
            if (!monster_is_genocided(id))
            {
                p->stats.monsters_killed[id] += monster_genocide(id);
                log_add_entry(p->log, "Wiped out all %s.",
                              monster_type_plural_name(id, 2));

                g_free(in);

                return TRUE;
            }
        }
    }

    g_free(in);

    log_add_entry(p->log, "No such monster.");
    return FALSE;
}

static int scroll_heal_monster(player *p, item *scroll)
{
    GList *mlist;
    int count = 0;
    monster *m;

    assert(p != NULL && scroll != NULL);

    mlist = g_hash_table_get_values(nlarn->monsters);

    /* purge genocided monsters */
    do
    {
        m = (monster *)mlist->data;
        position mpos = monster_pos(m);

        /* find monsters on the same level */
        if (mpos.z == p->pos.z)
        {
            if (monster_hp(m) < monster_hp_max(m))
            {
                monster_hp_inc(m, monster_hp_max(m));
                count++;
            }
        }
    }
    while ((mlist = mlist->next));

    if (count > 0)
    {
        log_add_entry(p->log, "You feel uneasy.");
    }

    g_list_free(mlist);

    return count;
}

static int scroll_identify(player *p, item *scroll)
{
    item *it;

    assert(p != NULL && scroll != NULL);

    if (inv_length_filtered(p->inventory, item_filter_unid) == 0)
    {
        /* player has no unidentfied items */
        log_add_entry(p->log, "Nothing happens.");
        return FALSE;
    }

    log_add_entry(p->log, "This is a scroll of identify.");

    if (scroll->blessed)
    {
        /* identify all items */
        log_add_entry(p->log, "You identify your possessions.");

        while (inv_length_filtered(p->inventory, item_filter_unid))
        {
            it = inv_get_filtered(p->inventory, 0, item_filter_unid);
            player_item_identify(p, NULL, it);
        }
    }
    else
    {
        /* identify the scroll being read, otherwise it would show up here */
        player_item_identify(p, NULL, scroll);

        /* choose a single item to identify */
        it = display_inventory("Choose an item to identify", p, &p->inventory,
                               NULL, FALSE, FALSE, FALSE, item_filter_unid);

        if (it != NULL)
        {
            char desc[81];

            item_describe(it, FALSE, (it->count == 1), TRUE, desc, 80);
            log_add_entry(p->log, "You identify %s.", desc);
            player_item_identify(p, NULL, it);

            item_describe(it, TRUE, (it->count == 1), FALSE, desc, 80);
            log_add_entry(p->log, "%s %s.", (it->count > 1) ? "These are" :
                          "This is", desc);
        }
    }

    return TRUE;
}

int scroll_mapping(player *p, item *scroll)
{
    position pos;
    map *m;

    /* scroll can be null as I use this to fake a known level */
    assert(p != NULL);

    m = game_map(nlarn, p->pos.z);
    pos.z = p->pos.z;

    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < MAP_MAX_X; pos.x++)
        {
            map_tile_t tile = map_tiletype_at(m, pos);
            if (scroll == NULL || tile != LT_FLOOR)
                player_memory_of(p, pos).type = tile;
            player_memory_of(p, pos).sobject = map_sobject_at(m, pos);
        }
    }

    return TRUE;
}

static int scroll_remove_curse(player *p, item *scroll)
{
    char buf[61];
    item *it;

    assert(p != NULL && scroll != NULL);

    if (inv_length_filtered(p->inventory, item_filter_cursed) == 0)
    {
        /* player has no cursed items */
        log_add_entry(p->log, "Nothing happens.");
        return FALSE;
    }

    log_add_entry(p->log, "This is a scroll of remove curse.");

    if (scroll->blessed)
    {
        /* remove curses on all items */
        log_add_entry(p->log, "You remove curses on your possessions.");

        while (inv_length_filtered(p->inventory, item_filter_cursed) > 0)
        {
            it = inv_get_filtered(p->inventory, 0, item_filter_cursed);

            // Get the description before uncursing the item.
            item_describe(it, player_item_known(p, it),
                          FALSE, TRUE, buf, 60);

            buf[0] = g_ascii_toupper(buf[0]);
            log_add_entry(p->log, "%s glow%s in a white light.",
                          buf, it->count == 1 ? "s" : "");

            item_remove_curse(it);
        }
    }
    else
    {
        /* choose a single item to uncurse */
        it = display_inventory("Choose an item to uncurse", p, &p->inventory,
                               NULL, FALSE, FALSE, FALSE, item_filter_cursed);

        if (it != NULL)
        {
            // Get the description before uncursing the item.
            item_describe(it, player_item_known(p, it),
                          FALSE, TRUE, buf, 60);

            buf[0] = g_ascii_toupper(buf[0]);

            log_add_entry(p->log, "%s glow%s in a white light.",
                          buf, it->count == 1 ? "s" : "");

            item_remove_curse(it);
        }
    }

    return TRUE;
}

static int scroll_spell_extension(player *p, item *scroll)
{
    guint idx;
    spell *sp;

    assert(p != NULL && scroll != NULL);

    for (idx = 0; idx < p->known_spells->len; idx++)
    {
        sp = g_ptr_array_index(p->known_spells, idx);

        if (scroll->blessed)
        {
            /* double spell knowledge */
            sp->knowledge <<=1;
        }
        else
        {
            /* increase spell knowledge */
            sp->knowledge++;
        }

    }

    /* give a message if any spell has been extended */
    if (p->known_spells->len > 0)
    {
        log_add_entry(p->log, "You feel your magic skills improve.");
        return TRUE;
    }

    return FALSE;
}

static int scroll_teleport(player *p, item *scroll)
{
    guint nlevel;

    assert(p != NULL);

    if (p->pos.z == 0)
        nlevel = 0;
    else if (p->pos.z < MAP_DMAX)
        nlevel = rand_0n(MAP_DMAX);
    else
        nlevel = rand_m_n(MAP_DMAX, MAP_MAX);

    if (nlevel != p->pos.z)
    {
        player_map_enter(p, game_map(nlarn, nlevel), TRUE);
        return TRUE;
    }

    return FALSE;
}

static int scroll_timewarp(player *p, item *scroll)
{
    /* number of mobuls */
    gint32 mobuls = 0;
    gint32 turns;

    assert(p != NULL && scroll != NULL);

    turns = (rand_1n(1000) - 850);

    if (turns == 0)
        turns = 1;

    if ((gint32)(game_turn(nlarn) + turns) < 0)
    {
        turns = 1 - game_turn(nlarn);
    }

    mobuls = gtime2mobuls(turns);

    /* rare case that time has not been modified */
    if (!mobuls)
    {
        return FALSE;
    }

    game_turn(nlarn) += turns;
    log_add_entry(p->log,
                  "You go %sward in time by %d mobul%s.",
                  (mobuls < 0) ? "back" : "for",
                  abs(mobuls),
                  (abs(mobuls) == 1) ? "" : "s");

    /* FIXME: adjust effects for time warping */

    return TRUE;
}
