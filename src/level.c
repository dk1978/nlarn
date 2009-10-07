/*
 * level.c
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

#include <assert.h>
#include <stdlib.h>

#include "container.h"
#include "display.h"
#include "items.h"
#include "level.h"
#include "nlarn.h"
#include "spheres.h"

static void level_fill_with_stationary(level *l);
static void level_fill_with_objects(level *l);
static void level_fill_with_traps(level *l);

static int level_load_from_file(level *l, char *mazefile, int which);
static void level_make_maze(level *l);
static void level_make_maze_eat(level *l, int x, int y);
static void level_add_treasure_room(level *l);
static void level_add_item(level *l, item *what);

static level_path *level_path_new(position start, position goal);
static level_path_element *level_path_element_new(position pos);
static int level_path_cost(level *l, level_path_element* element, position target);
static level_path_element *level_path_element_in_list(level_path_element* el, GPtrArray *list);
static level_path_element *level_path_find_best(level *l, level_path *path);
static GPtrArray *level_path_get_neighbours(level *l, position pos);

const level_tile_data level_tiles[LT_MAX] =
{
    /* type         img  color      desc           pa tr */
    { LT_NONE,      ' ', DC_NONE,   NULL,          0, 0 },
    { LT_MOUNTAIN,  '^', DC_WHITE,  "a mountain",  0, 0 },
    { LT_GRASS,     '"', DC_GREEN,  "grass",       1, 1 },
    { LT_DIRT,      '.', DC_YELLOW, "dirt",        1, 1 },
    { LT_TREE,      '&', DC_GREEN,  "a tree",      0, 0 },
    { LT_FLOOR,     ' ', DC_NONE,   "floor",       1, 1 },
    { LT_WATER,     '~', DC_BLUE,   "water",       1, 1 },
    { LT_DEEPWATER, '~', DC_BLUE,   "deep water",  0, 1 },
    { LT_LAVA,      '=', DC_RED,    "lava",        0, 1 },
    { LT_FIRE,      '*', DC_RED,    "fire",        1, 1 },
    { LT_CLOUD,     '*', DC_WHITE,  "a gas cloud", 1, 1 },
    { LT_WALL,      '#', DC_WHITE,  "a wall",      0, 0 },
};

const level_stationary_data level_stationaries[LS_MAX] =
{
    { LS_NONE,          ' ',  DC_NONE,    "You see nothing special.",                     1, 1, },
    { LS_ALTAR,         '_',  DC_WHITE,   "There is a holy altar.",                       1, 1, },
    { LS_THRONE,        '\\', DC_MAGENTA, "A handsome, jewel-encrusted throne.",          1, 1, },
    { LS_THRONE2,       '\\', DC_MAGENTA, "A handsome, jewel-encrusted throne.",          1, 1, },
    { LS_DEADTHRONE,    '\\', DC_WHITE,   "There is a massive throne.",                   1, 1, },
    { LS_STAIRSDOWN,    '>',  DC_WHITE,   "There is a circular staircase here.",          1, 1, },
    { LS_STAIRSUP,      '<',  DC_WHITE,   "There is a circular staircase here.",          1, 1, },
    { LS_ELEVATORDOWN,  'I',  DC_RED,     "There is a volcanic shaft leaning downward.",  1, 1, },
    { LS_ELEVATORUP,    'I',  DC_RED,     "You behold the base of a volcanic shaft.",     1, 1, },
    { LS_FOUNTAIN,      '{',  DC_BLUE,    "There is a bubbling fountain here.",           1, 1, },
    { LS_DEADFOUNTAIN,  '{',  DC_WHITE,   "There is a dead fountain here.",               1, 1, },
    { LS_STATUE,        '|',  DC_WHITE,   "This is a great marble statue.",               1, 1, },
    { LS_URN,           'u',  DC_YELLOW,  "This is a golden urn.",                        1, 1, },
    { LS_MIRROR,        '\'', DC_WHITE,   "There is a mirror here.",                      1, 1, },
    { LS_OPENDOOR,      '/',  DC_YELLOW,  "There is an open door.",                       1, 1, },
    { LS_CLOSEDDOOR,    '+',  DC_YELLOW,  "You see a closed door.",                       0, 0, },
    { LS_ENTRANCE,      'O',  DC_RED,     "This is the dungeon entrance.",                1, 0, },
    { LS_HOME,          'H',  DC_WHITE,   "This is your home.",                           1, 0, },
    { LS_DNDSTORE,      'D',  DC_WHITE,   "There is a DND store here.",                   1, 0, },
    { LS_TRADEPOST,     'T',  DC_WHITE,   "You have found the Larn trading Post.",        1, 0, },
    { LS_LRS,           'L',  DC_WHITE,   "There is an LRS office here.",                 1, 0, },
    { LS_SCHOOL,        'S',  DC_WHITE,   "This is the College of Larn.",                 1, 0, },
    { LS_BANK,          'B',  DC_WHITE,   "This is the bank of Larn.",                    1, 0, },
    { LS_BANK2,         'B',  DC_WHITE,   "This is a branch office of the bank of Larn.", 1, 0, },
};

/* keep track which levels have been used before */
static int level_used[23] = { 1, 0 };

const char *level_names[LEVEL_MAX] =
{
    "Town",
    "D1",
    "D2",
    "D3",
    "D4",
    "D5",
    "D6",
    "D7",
    "D8",
    "D9",
    "D10",
    "V1",
    "V2",
    "V3"
};

level *level_new(int nlevel, char *mazefile)
{
    gboolean level_loaded = FALSE;
    int x, y;

    level *l = g_malloc0(sizeof(level));
    l->nlevel = nlevel;

    /* initialize monster and sphere list */
    l->mlist = g_ptr_array_new();
    l->slist = g_ptr_array_new();

    /* create map */
    if ((l->nlevel == 0)
            || (l->nlevel == LEVEL_DMAX - 1)
            || (l->nlevel == LEVEL_MAX - 1)
            || (l->nlevel > 1 && chance(25)))
    {
        /* read maze from data file */
        level_loaded = level_load_from_file(l, mazefile,
                                            (l->nlevel == 0) ? 0 : -1);
    }

    if (!level_loaded)
    {
        /* clear map */
        for (y = 0; y < LEVEL_MAX_Y; y++)
            for (x = 0; x < LEVEL_MAX_X; x++)
                l->map[y][x].type = LT_WALL;

        /* generate random map */
        level_make_maze(l);

        /* home town is safe */
        if (l->nlevel > 0)
            level_fill_with_live(l);

        /* add treasure room */
        if (l->nlevel > 1
                && (l->nlevel == (LEVEL_DMAX - 1)
                    || l->nlevel == (LEVEL_MAX - 1)
                    || chance(15)))
        {
            level_add_treasure_room(l);
        }
    }

    if (l->nlevel > 0)
    {
        /* add static content */
        level_fill_with_stationary(l);

        /* home town is not filled with crap */
        level_fill_with_objects(l);

        /* and not trapped */
        level_fill_with_traps(l);
    }

    return l;
}

char *level_dump(level *l)
{
    position pos;
    GString *map;

    map = g_string_new_len(NULL, LEVEL_SIZE);

    pos.z = l->nlevel;

    for (pos.y = 0; pos.y < LEVEL_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < LEVEL_MAX_X; pos.x++)
        {
            if (level_trap_at(l, pos))
            {
                g_string_append_c(map, '^');
            }
            else if (level_stationary_at(l, pos))
            {
                g_string_append_c(map, ls_get_image(level_stationary_at(l, pos)));
            }
            else
            {
                g_string_append_c(map, lt_get_image(level_tiletype_at(l, pos)));
            }
        }
        g_string_append_c(map, '\n');
    }

    return g_string_free(map, FALSE);
}

void level_destroy(level *l)
{
    guint idx;
    int x, y;

    assert(l != NULL);

    /* free monster list */
    if (l->mlist != NULL)
    {
        while (l->mlist->len > 0)
        {
            idx = l->mlist->len - 1;
            monster_destroy(g_ptr_array_index(l->mlist, idx));
        }

        g_ptr_array_free(l->mlist, TRUE);
    }

    /* free spheres list */
    if (l->slist != NULL)
    {
        while (l->slist->len > 0)
        {
            idx = l->slist->len - 1;
            sphere_destroy(g_ptr_array_index(l->slist, idx));
        }

        g_ptr_array_free(l->slist, TRUE);
    }

    for (y = 0; y < LEVEL_MAX_Y; y++)
    {
        for (x = 0; x < LEVEL_MAX_X; x++)
        {
            /* free items */
            if (l->map[y][x].ilist != NULL)
                inv_destroy(l->map[y][x].ilist);
        }
    }

    g_free(l);
}

/* return coordinates of a free space */
position level_find_space(level *l, level_element_t element)
{
    rectangle lvl_area = rect_new(1, 1, LEVEL_MAX_X - 2, LEVEL_MAX_Y - 2);

    return level_find_space_in(l, lvl_area, element);
}

position level_find_space_in(level *l, rectangle where, level_element_t element)
{
    position pos, pos_orig;

    assert (l != NULL && element > LE_NONE && element < LE_MAX);

    pos_orig.x = pos.x = rand_m_n(where.x1, where.x2);
    pos_orig.y = pos.y = rand_m_n(where.y1, where.y2);
    pos_orig.z = pos.z = l->nlevel;

    do
    {
        pos.x += 1;
        pos.y += 1;

        if (pos.x > where.x2)
            pos.x = where.x1;

        if (pos.x < where.x1)
            pos.x = where.x2;

        if (pos.y > where.y2)
            pos.y = where.y1;

        if (pos.y < where.y1)
            pos.y = where.y2;
    }
    while (!level_validate_position(l, pos, element)
            && !pos_identical(pos, pos_orig));

    if (pos_identical(pos, pos_orig))
    {
        pos.x = G_MAXINT16;
        pos.y = G_MAXINT16;
    }

    return pos;
}

int *level_get_surrounding(level *l, position pos, level_stationary_t type)
{
    position p;
    int move = 1;
    int *dirs;

    dirs = g_malloc0(sizeof(int) * GD_MAX);

    while (move < GD_MAX)
    {
        p = pos_move(pos, move);

        if (pos_valid(p) && level_stationary_at(l, p) == type)
        {
            dirs[move] = TRUE;
        }

        move++;
    }

    return dirs;
}

position level_find_stationary_in(level *l, level_stationary_t stationary, rectangle area)
{
    position pos;

    assert(l != NULL);

    pos.z = l->nlevel;

    for (pos.y = area.y1; pos.y <= area.y2; pos.y++)
        for (pos.x = area.x1; pos.x <= area.x2; pos.x++)
            if (level_stationary_at(l,pos) == stationary)
                return pos;

    /* if we reach this point, the stationary is not on the map */
    return pos_new(G_MAXINT16, G_MAXINT16, G_MAXINT16);
}

position level_find_stationary(level *l, level_stationary_t stationary)
{
    position pos;

    assert(l != NULL);

    pos.z = l->nlevel;

    for (pos.y = 0; pos.y < LEVEL_MAX_Y; pos.y++)
        for (pos.x = 0; pos.x < LEVEL_MAX_X; pos.x++)
            if (level_stationary_at(l,pos) == stationary)
                return pos;

    /* if we reach this point, the stationary is not on the map */
    return pos_new(G_MAXINT16, G_MAXINT16, G_MAXINT16);
}

gboolean level_validate_position(level *l, position pos, level_element_t element)
{
    level_tile *tile;

    assert(l != NULL && element > LT_NONE && element < LE_MAX);

    /* if the position is invalid it is invalid for the level as well */
    if (!pos_valid(pos))
        return FALSE;

    /* if the position is on another level it is invalid for this lavel */
    if (pos.z != l->nlevel)
        return FALSE;

    /* make shortcut */
    tile = level_tile_at(l, pos);

    switch (element)
    {

    case LE_GROUND:
        /* why should we need this case? */
        return TRUE;
        break;

    case LE_STATIONARY:
        if (lt_is_passable(tile->type) && (tile->stationary == LS_NONE) )
        {
            /* FIXME: don't want two stationaries next to each other */
            /* FIXME: don't want stationaries next to the wall */
            return TRUE;
        }
        break;

    case LE_TRAP:
        if (lt_is_passable(tile->type)
                && (tile->stationary == LS_NONE)
                && (tile->trap == TT_NONE) )
            return TRUE;
        break;

    case LE_ITEM:
        /* we can stack like mad, so we only need to check if there is an open space */
        if (level_pos_passable(l, pos) && (tile->stationary == LS_NONE))
            return TRUE;
        break;

    case LE_MONSTER:
        /* not ok if player is standing on that tile */
        if (pos_identical(pos, nlarn->p->pos))
            return FALSE;

        if (level_pos_passable(l, pos) && !level_is_monster_at(l, pos))
            return TRUE;
        break;

    case LE_NONE:
    case LE_MAX:
        return FALSE;
        break;

    } /* switch */

    return FALSE;
}

/**
 * determine if a position can be seen from another position
 * @param the level
 * @param first position
 * @param second position
 * @return TRUE or FALSE
 */
int level_pos_is_visible(level *l, position s, position t)
{
    int delta_x, delta_y;
    int x, y;
    signed int ix, iy;
    int error;

    /* positions on different levels? */
    if (s.z != t.z)
        return FALSE;

    x = s.x;
    y = s.y;

    delta_x = abs(t.x - s.x) << 1;
    delta_y = abs(t.y - s.y) << 1;

    /* if x1 == x2 or y1 == y2, then it does not matter what we set here */
    ix = t.x > s.x ? 1 : -1;
    iy = t.y > s.y ? 1 : -1;

    if (delta_x >= delta_y)
    {
        /* error may go below zero */
        error = delta_y - (delta_x >> 1);

        while (x != t.x)
        {
            if (error >= 0)
            {
                if (error || (ix > 0))
                {
                    y += iy;
                    error -= delta_x;
                }
            }

            x += ix;
            error += delta_y;

            if (!lt_is_transparent(l->map[y][x].type) || !ls_is_transparent(l->map[y][x].stationary))
                return FALSE;
        }
    }
    else
    {
        /* error may go below zero */
        int error = delta_x - (delta_y >> 1);

        while (y != t.y)
        {
            if (error >= 0)
            {
                if (error || (iy > 0))
                {
                    x += ix;
                    error -= delta_y;
                }
            }

            y += iy;
            error += delta_x;

            if (!lt_is_transparent(l->map[y][x].type) || !ls_is_transparent(l->map[y][x].stationary))
                return FALSE;
        }
    }

    return TRUE;
}

level_path *level_find_path(level *l, position start, position goal)
{
    assert(l != NULL && (start.z == goal.z));

    level_path *path;
    level_path_element *curr, *next;
    gboolean next_is_better;
    GPtrArray *neighbours;

    path = level_path_new(start, goal);

    /* add start to open list */
    curr = level_path_element_new(start);
    g_ptr_array_add(path->open, curr);

    while (path->open->len)
    {
        curr = level_path_find_best(l, path);

        g_ptr_array_remove_fast(path->open, curr);
        g_ptr_array_add(path->closed, curr);

        if (curr->pos.x == path->goal.x && curr->pos.y == path->goal.y)
        {
            /* arrived at goal */

            /* reconstruct path */
            do
            {
                /* don't need the starting point in the path */
                if (curr->parent != NULL)
                    g_queue_push_head(path->path, curr);

                curr = curr->parent;
            }
            while (curr != NULL);

            return path;
        }

        neighbours = level_path_get_neighbours(l, curr->pos);

        while (neighbours->len)
        {
            next = g_ptr_array_remove_index_fast(neighbours,
                                                 neighbours->len - 1);

            next_is_better = FALSE;

            if (level_path_element_in_list(next, path->closed))
            {
                g_free(next);
                continue;
            }

            if (!level_path_element_in_list(next, path->open))
            {
                g_ptr_array_add(path->open, next);
                next_is_better = TRUE;
            }
            else if (level_path_cost(l, curr, path->goal) > level_path_cost(l, next, path->goal))
            {
                next_is_better = TRUE;
            }
            else
            {
                g_free(next);
            }

            if (next_is_better)
            {
                next->parent = curr;
            }
        }

        g_ptr_array_free(neighbours, TRUE);
    }

    return NULL;
}

void level_path_destroy(level_path *path)
{
    guint idx;

    assert(path != NULL);

    /* cleanup open list */
    for (idx = 0; idx < path->open->len; idx++)
    {
        g_free(g_ptr_array_index(path->open, idx));
    }

    g_ptr_array_free(path->open, TRUE);

    for (idx = 0; idx < path->closed->len; idx++)
    {
        g_free(g_ptr_array_index(path->closed, idx));
    }

    g_ptr_array_free(path->closed, TRUE);

    g_queue_free(path->path);

    g_free(path);
}

area *level_get_obstacles(level *l, position center, int radius)
{
    area *narea;
    position pos;
    int x, y;

    assert(l != NULL);

    if (!pos_valid(center))
    {
        return NULL;
    }

    narea = area_new(center.x - radius, center.y - radius,
                     radius * 2 + 1, radius * 2 + 1);

    pos.z = l->nlevel;

    for (pos.y = center.y - radius, y = 0; pos.y <= center.y + radius; pos.y++, y++)
    {
        for (pos.x = center.x - radius, x = 0; pos.x <= center.x + radius; pos.x++, x++)
        {
            if (!pos_valid(pos) || !level_pos_passable(l,pos))
            {
                area_point_set(narea, x, y);
            }
        }
    }

    return narea;
}

void level_set_tiletype(level *l, area *area, level_tile_t type, guint8 duration)
{
    position pos;
    int x, y;

    assert (l != NULL && area != NULL);

    for (pos.y = area->start_y, y = 0;
            pos.y < area->start_y + area->size_y;
            pos.y++, y++)
    {
        for (pos.x = area->start_x, x = 0;
                pos.x < area->start_x + area->size_x;
                pos.x++, x++)
        {
            /* if the position is marked in area set the tile to type */
            if (area_point_get(area, x, y))
            {
                level_tile *tile = level_tile_at(l, pos);
                tile->base_type = level_tiletype_at(l, pos);
                tile->type = type;
                tile->timer = duration;
            }
        }
    }
}

level_tile *level_tile_at(level *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return &l->map[pos.y][pos.x];
}

inventory **level_ilist_at(level *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return &l->map[pos.y][pos.x].ilist;
}

level_tile_t level_tiletype_at(level *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->map[pos.y][pos.x].type;
}

level_tile_t level_basetype_at(level *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->map[pos.y][pos.x].base_type;
}

guint8 level_timer_at(level *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->map[pos.y][pos.x].timer;
}

trap_t level_trap_at(level *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->map[pos.y][pos.x].trap;
}

void level_trap_set(level *l, position pos, trap_t type)
{
    assert(l != NULL && pos_valid(pos));

    l->map[pos.y][pos.x].trap = type;
}

level_stationary_t level_stationary_at(level *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->map[pos.y][pos.x].stationary;
}

void level_stationary_set(level *l, position pos, level_stationary_t type)
{
    assert(l != NULL && pos_valid(pos));

    l->map[pos.y][pos.x].stationary = type;
}

damage *level_tile_damage(level *l, position pos)
{
    assert (l != NULL && pos_valid(pos));

    switch (level_tiletype_at(l, pos))
    {
    case LT_CLOUD:
        return damage_new(DAM_ACID, 3 + rand_0n(2), NULL);
        break;

    case LT_FIRE:
        return damage_new(DAM_FIRE, 5 + rand_0n(2), NULL);
        break;

    case LT_WATER:
        return damage_new(DAM_WATER, 4 + rand_0n(2), NULL);
        break;

    default:
        return NULL;
    }
}

monster *level_get_monster_at(level *l, position pos)
{
    guint idx = 0;
    monster *m, *match = NULL;

    assert(l != NULL);

    while (idx < l->mlist->len)
    {
        m = (monster *)g_ptr_array_index(l->mlist, idx);

        if ((m->pos.x == pos.x) && (m->pos.y == pos.y))
        {
            match = m;
            break;	/* found it! */
        }

        idx++;
    }

    return match;
}

int level_is_monster_at(level *l, position pos)
{
    assert(l != NULL);

    if (level_get_monster_at(l, pos))
        return TRUE;
    else
        return FALSE;
}

GPtrArray *level_get_monsters_in(level *l, rectangle area)
{
    GPtrArray *monsters;
    monster *m;
    guint idx;

    assert(l != NULL);

    monsters = g_ptr_array_new();

    for (idx = 0; idx < l->mlist->len; idx++)
    {
        m = (monster *)g_ptr_array_index(l->mlist, idx);

        if (pos_in_rect(m->pos, area))
        {
            g_ptr_array_add(monsters, m);
        }
    }

    return monsters;
}

/**
 * creates an entire set of monsters for a level
 * @param a level
 */
int level_fill_with_live(level *l)
{
    int new_monster_count;
    int i;

    assert(l != NULL);

    new_monster_count = rand_m_n(2, 14) + (l->nlevel >> 1);

    /* if there are some monsters on the level, add only a few */
    new_monster_count -= l->mlist->len;

    if (new_monster_count < 0)
    {
        /* sanity check */
        new_monster_count = 1;
    }

    for (i = 0; i <= new_monster_count; i++)
    {
        monster_level_enter(monster_new_by_level(l->nlevel), l);
    }

    return(new_monster_count);
}

void level_timer(level *l, guint8 count)
{
    position pos;
    item *it;
    guint idx;
    int impact;
    char *impact_desc;
    char item_desc[61];

    assert (l != NULL);

    pos.z = l->nlevel;

    for (pos.y = 0; pos.y < LEVEL_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < LEVEL_MAX_X; pos.x++)
        {
            if (level_timer_at(l, pos))
            {
                level_tile *tile = level_tile_at(l, pos);
                tile->timer -= min(level_timer_at(l, pos), count);

                /* affect items */
                if (tile->ilist != NULL)
                {
                    for (idx = 0; idx < inv_length(tile->ilist); idx++)
                    {
                        it = inv_get(tile->ilist, idx);

                        switch (tile->type)
                        {
                        case LT_FIRE:
                            impact = item_burn(it);
                            impact_desc = "burn";
                            break;

                        case LT_CLOUD:
                            impact = item_corrode(it);
                            impact_desc = "corrode";
                            break;

                        case LT_WATER:
                            impact = item_rust(it);
                            impact_desc = "rust";
                            break;
                        default:
                            impact = PI_NONE;
                            break;
                        }

                        /* notifiy player of impact if the item is visible */
                        if (impact && level_pos_is_visible(l, nlarn->p->pos, pos))
                        {
                            item_describe(it, player_item_known(nlarn->p, it),
                                          (it->count == 1), TRUE, item_desc, 60);

                            if (impact < PI_DESTROYED)
                            {
                                log_add_entry(nlarn->p->log, "The %s %s%s.", item_desc,
                                              impact_desc, (it->count == 1) ? "s" : "");
                            }
                            else
                            {
                                log_add_entry(nlarn->p->log, "The %s %s destroyed.", item_desc,
                                              (it->count == 1) ? "is" : "are");
                            }
                        }

                        if (impact == PI_DESTROYED)
                        {
                            inv_del_element(&tile->ilist, it);
                            item_destroy(it);
                        }
                    } /* foreach item */
                } /* if leve_ilist_at */

                /* reset tile type if temporary effect has expired */
                if (tile->timer == 0)
                {
                    if ((tile->type == LT_FIRE)
                            && (tile->type == LT_GRASS))
                    {
                        tile->type = LT_DIRT;
                        tile->base_type = LT_DIRT;
                    }
                    else
                    {
                        tile->type = tile->base_type;
                    }
                }
            } /* if level_timer_at */
        } /* for pos.x */
    } /* for pos.y */
}

static void level_fill_with_stationary(level *l)
{
    position pos;
    int i;						/* loop var */

    /* volcano shaft up from the temple */
    if (l->nlevel == LEVEL_MAX -1)
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_ELEVATORUP);
    }

    /*  make the fixed objects in the maze: STAIRS */
    if ((l->nlevel > 0) && (l->nlevel != LEVEL_DMAX - 1) && (l->nlevel != LEVEL_MAX - 1))
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l,pos, LS_STAIRSDOWN);
    }

    if ((l->nlevel > 1) && (l->nlevel != LEVEL_DMAX))
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_STAIRSUP);
    }

    /* make the random objects in the maze */
    /* 33 percent chance for an altar */
    if (chance(33))
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_ALTAR);
    }

    /* up to three statues */
    for (i = 0; i < rand_0n(3); i++)
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_STATUE);
    }

    /* up to three fountains */
    for (i = 0; i < rand_0n(3); i++)
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_FOUNTAIN);
    }

    /* up to two thrones */
    for (i = 0; i < rand_0n(2); i++)
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_THRONE);
    }

    /* up to two  mirrors */
    for (i = 0; i < rand_0n(2); i++)
    {
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_MIRROR);
    }

    if (l->nlevel == 5)
    {
        /* branch office of the bank */
        pos = level_find_space(l, LE_STATIONARY);
        level_stationary_set(l, pos, LS_BANK2);
    }
} /* level_fill_with_stationary */

static void level_fill_with_objects(level *l)
{
    int i,j;                    /* loop vars */
    item_t it;
    item *container = NULL;

    /* up to two pieces of armour */
    for (i = 0; i < rand_0n(2); i++)
    {
        level_add_item(l, item_new_by_level(IT_ARMOUR, l->nlevel));
    }

    /* up to three books */
    for (i = 0; i <= rand_0n(3); i++)
    {
        level_add_item(l, item_new_by_level(IT_BOOK, l->nlevel));
    }

    /* up to two containers */
    for (i = 1; i <= rand_0n(2); i++)
    {
        /* random container type */
        container = item_new(IT_CONTAINER, rand_1n(CT_MAX), 0);

        /* up to 5 items inside the container */
        for (j = 0; j < rand_0n(5); j++)
        {
            /* prevent containers inside the container */
            do
            {
                it = rand_1n(IT_MAX - 1);
            }
            while (it == IT_CONTAINER);

            inv_add(&(container->content), item_new_by_level(it, l->nlevel));
        }

        /* add the container to the level */
        level_add_item(l, container);
    }

    /* up to 10 piles of gold */
    for (i = 0; i < rand_0n(10); i++)
    {
        /* There is nothing like a newly minted pound. */
        level_add_item(l, item_new(IT_GOLD, rand_m_n(10, (l->nlevel + 1) * 15), 0));
    }

    /* up to three gems */
    for (i = 0; i < rand_0n(3); i++)
    {
        level_add_item(l, item_new(IT_GEM, rand_1n(item_max_id(IT_GEM)),
                                   rand_0n(6 * (l->nlevel + 1))));
    }

    /* up to four potions */
    for (i = 0; i < rand_0n(4); i++)
    {
        level_add_item(l, item_new_by_level(IT_POTION, l->nlevel));
    }

    /* up to three scrolls */
    for (i = 0; i < rand_0n(3); i++)
    {
        level_add_item(l, item_new_by_level(IT_SCROLL, l->nlevel));
    }

    /* up to two rings */
    for (i = 0; i < rand_0n(3); i++)
    {
        level_add_item(l, item_new_by_level(IT_RING, l->nlevel));
    }

    /* up to two weapons */
    for (i = 0; i < rand_0n(2); i++)
    {
        level_add_item(l, item_new_by_level(IT_WEAPON, l->nlevel));
    }

} /* level_fill_with_objects */

static void level_fill_with_traps(level *l)
{
    int count;
    position pos;
    int trapdoor = FALSE;

    assert(l != NULL);

    /* Trapdoor cannot be placed in the last dungeon level and the last vulcano level */
    trapdoor = ((l->nlevel != LEVEL_DMAX - 1) && (l->nlevel != LEVEL_MAX - 1));

    for (count = 0; count < rand_0n((trapdoor ? 8 : 6)); count++)
    {
        pos = level_find_space(l, LE_TRAP);
        level_trap_set(l, pos, rand_1n(trapdoor ? TT_MAX : TT_TRAPDOOR));
    }
} /* level_fill_with_traps */

/* subroutine to make the caverns for a given level. only walls are made. */
static void level_make_maze(level *l)
{
    position pos;
    int mx, mxl, mxh;
    int my, myl, myh;
    int nrooms;
    monster *m = NULL;
    gboolean want_monster = FALSE;

    level_make_maze_eat(l, 1, 1);

    /* add exit to town on level 1 */
    if (l->nlevel == 1)
    {
        l->map[LEVEL_MAX_Y - 1][(LEVEL_MAX_X - 1) / 2].type = LT_FLOOR;
        l->map[LEVEL_MAX_Y - 1][(LEVEL_MAX_X - 1) / 2].stationary = LS_ENTRANCE;
    }

    /*  now for open spaces -- not on level 10  */
    if (l->nlevel != (LEVEL_DMAX - 1))
    {
        for (nrooms = 0; nrooms < rand_1n(3) + 3; nrooms++)
        {
            my = rand_1n(11) + 2;
            myl = my - rand_1n(2);
            myh = my + rand_1n(2);
            if (l->nlevel < LEVEL_DMAX)
            {
                mx = rand_1n(44)+5;
                mxl = mx - rand_1n(4);
                mxh = mx + rand_1n(12)+3;
            }
            else
            {
                mx = rand_1n(60)+3;
                mxl = mx - rand_1n(2);
                mxh = mx + rand_1n(2);
                want_monster = TRUE;
            }

            pos.z = l->nlevel;

            for (pos.y = myl ; pos.y < myh ; pos.y++)
            {
                for (pos.x = mxl ; pos.x < mxh ; pos.x++)
                {
                    level_tile *tile = level_tile_at(l, pos);
                    tile->type = LT_FLOOR;
                    tile->base_type = LT_FLOOR;

                    if (want_monster == TRUE)
                    {
                        m = monster_new_by_level(l->nlevel);
                        monster_level_enter(m, l);
                        monster_position(m, pos);

                        want_monster = FALSE;
                    }
                }
            }
        }
    }
}

/* function to eat away a filled in maze */
static void level_make_maze_eat(level *l, int x, int y)
{
    int dir;
    int try = 2;

    dir = rand_1n(4);

    while (try)
    {
        switch (dir)
        {
        case 1: /* west */
            if ((x > 2) &&
                    (l->map[y][x - 1].type == LT_WALL) &&
                    (l->map[y][x - 2].type == LT_WALL))
            {
                l->map[y][x - 1].type = l->map[y][x - 2].type = LT_FLOOR;
                level_make_maze_eat(l, x - 2, y);
            }
            break;

        case 2: /* east */
            if (x < (LEVEL_MAX_X - 3) &&
                    (l->map[y][x + 1].type == LT_WALL) &&
                    (l->map[y][x + 2].type == LT_WALL))
            {
                l->map[y][x + 1].type = l->map[y][x + 2].type = LT_FLOOR;
                level_make_maze_eat(l, x + 2, y);
            }
            break;

        case 3: /* south */
            if ((y > 2) &&
                    (l->map[y - 1][x].type == LT_WALL) &&
                    (l->map[y - 2][x].type == LT_WALL))
            {
                l->map[y - 1][x].type = l->map[y - 2][x].type = LT_FLOOR;
                level_make_maze_eat(l, x, y - 2);
            }

            break;

        case 4: /* north */
            if ((y < LEVEL_MAX_Y - 3) &&
                    (l->map[y + 1][x].type == LT_WALL) &&
                    (l->map[y + 2][x].type == LT_WALL))
            {
                l->map[y + 1][x].type = l->map[y + 2][x].type = LT_FLOOR;
                level_make_maze_eat(l, x, y + 2);
            }

            break;
        };
        if (++dir > 4)
        {
            dir = 1;
            --try;
        }
    }
}

/*
 *  function to read in a maze from a data file
 *
 *  Format of maze data file:
 *  For each maze:  LEVEL_MAX_Y + 1 lines (LEVEL_MAX_Y used)
 *                  LEVEL_MAX_X characters per line
 *
 *  Special characters in maze data file:
 *
 *      #   wall
 *      D   door
 *      .   random monster
 *      ~   eye of larn
 *      !   potion of cure dianthroritis
 *      -   random object
 */
static int level_load_from_file(level *l, char *mazefile, int which)
{
    position pos;       /* current position on map */
    int lt, ls;         /* selected tile and static object type */
    int level_num = 0;  /* number of selected level */
    monster *monst;     /* placeholder for monster */
    void *itm;          /* placeholder for objects */
    item_t it;          /* item type for random objects */

    FILE *levelfile;

    if (!(levelfile = fopen(mazefile, "r")))
    {
        return FALSE;
    }

    if (feof(levelfile))
    {
        /* FIXME: debug output */
        fclose(levelfile);

        return FALSE;
    }

    /* FIXME: calculate how many levels are in the file  */
    /* roll the dice: which level? we can currently choose from a variety of 24 */

    if (which >= 0 && which < 25)
    {
        level_num = which;
    }
    else
    {
        while (level_used[level_num])
        {
            level_num = rand_1n(25);
        }
        level_used[level_num] = TRUE;
    }

    /* advance to desired maze */
    fseek(levelfile, (level_num * ((LEVEL_MAX_X + 1) * LEVEL_MAX_Y + 1)), SEEK_SET);

    pos.z = l->nlevel;

    for (pos.y = 0; pos.y < LEVEL_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < LEVEL_MAX_X ; pos.x++)
        {
            level_tile *tile = level_tile_at(l, pos);

            monst = NULL;	/* to make checks below work */
            itm = NULL;		/* default: no new item */
            lt = LT_FLOOR;	/* floor is default */
            ls = LS_NONE;	/* no default static element */

            switch (fgetc(levelfile))
            {

            case '^': /* mountain */
                lt = LT_MOUNTAIN;
                break;

            case '"': /* grass */
                lt = LT_GRASS;
                break;

            case '.': /* dirt */
                lt = LT_DIRT;
                break;

            case '&': /* tree */
                lt = LT_TREE;
                break;

            case '~': /* deep water */
                lt = LT_DEEPWATER;
                break;

            case '=': /* lava */
                lt = LT_LAVA;
                break;

            case '#': /* wall */
                lt =  LT_WALL;
                break;

            case '+': /* door */
                ls = LS_CLOSEDDOOR;
                break;

            case 'O': /* dungeon entrance */
                ls = LS_ENTRANCE;
                break;

            case 'I': /* elevator */
                ls = LS_ELEVATORDOWN;
                break;

            case 'H': /* home */
                ls = LS_HOME;
                break;

            case 'D': /* dnd store */
                ls = LS_DNDSTORE;
                break;

            case 'T': /* trede post */
                ls = LS_TRADEPOST;
                break;

            case 'L': /* LRS */
                ls = LS_LRS;
                break;

            case 'S': /* school */
                ls = LS_SCHOOL;
                break;

            case 'B': /*  */
                ls = LS_BANK;
                break;

            case '*': /* eye of larn */
                if (l->nlevel != LEVEL_DMAX - 1)
                {
                    break;
                }
                itm = item_new(IT_AMULET, AM_LARN, 0);

                monst = monster_new(MT_DEMONLORD_I + rand_0n(7));
                monster_level_enter(monst, l);
                break;

            case '!':	/* potion of cure dianthroritis */
                if (l->nlevel != LEVEL_MAX - 1)
                    break;

                itm = item_new(IT_POTION, PO_CURE_DIANTHR, 0);
                monst = monster_new(MT_DAEMON_PRINCE);
                monster_level_enter(monst, l);
                break;

            case 'M':	/* random monster */
                monster_level_enter(monster_new_by_level(l->nlevel), l);
                break;

            case '-':
                do
                {
                    it = rand_1n(IT_MAX - 1);
                }
                while (it == IT_CONTAINER);

                itm = item_new_by_level(it, l->nlevel);
                break;
            };

            tile->type = tile->base_type = lt;
            tile->stationary = ls;

            if (itm != NULL)
            {
                inv_add(&tile->ilist, itm);
            }
        }
        (void)fgetc(levelfile); /* eat EOL */
    }

    fclose(levelfile);

    return TRUE;
}

/*
 * function to make a treasure room on a level
 */
static void level_add_treasure_room(level *l)
{
    int x1, y1;         /* upper left corner of room */
    int x2, y2;         /* room dimensions */
    position pos, npos;
    level_tile *tile;
    item *itm;
    monster *monst;
    int success;

    x1 = rand_1n(LEVEL_MAX_X - 9);
    y1 = rand_1n(LEVEL_MAX_Y - 9);

    x2 = x1 + rand_m_n(3, 6);
    y2 = y1 + rand_m_n(3, 6);

    pos.z = npos.z = l->nlevel;

    for (pos.y = y1; pos.y <= y2; pos.y++)
    {
        for (pos.x = x1; pos.x <= x2; pos.x++)
        {
            tile = level_tile_at(l, pos);

            if ( (pos.y == y1) || (pos.y == y2) || (pos.x == x1) || (pos.x == x2) )
            {
                /* if we are on the border of a room, make wall */
                tile->type = tile->base_type = LT_WALL;
            }
            else
            {
                /* clear out space */
                tile->type = tile->base_type = LT_FLOOR;

                /* create loot */
                itm = item_new_random(IT_GOLD);
                inv_add(&tile->ilist, itm);

                /* create a monster */
                monst = monster_new_by_level(l->nlevel);
                monster_level_enter(monst, l);
                monster_position(monst, pos);
            }

            /* now clear out interior */
            if (level_stationary_at(l,pos) > LS_NONE)
            {
                success = FALSE;
                do
                {
                    npos = level_find_space(l, LE_STATIONARY);
                    if ( (npos.x > x1) && (npos.x < x2) && (npos.y > y1) && (npos.y < y2) )
                    {
                        /* pos is outside of room */
                        level_stationary_set(l, npos, level_stationary_at(l,pos));
                        level_stationary_set(l, pos, LS_NONE);

                        success = TRUE;
                    }
                }
                while (!success);
            } /* if stationary */
        } /* for x */
    } /* for y */

    /* locate the door on the treasure room */
    /* FIXME: ensure door is reachable */
    switch (rand_1n(2))
    {
    case 1: /* horizontal */
        pos.x = rand_m_n(x1, x2);
        pos.y = rand_0n(1) ? y1 : y2;
        break;

    case 2: /* vertical */
        pos.x = rand_0n(1) ? x1 : x2;
        pos.y = rand_m_n(y1, y2);
        break;
    };

    tile = level_tile_at(l, pos);

    tile->type = tile->base_type = LT_FLOOR;
    tile->stationary = LS_CLOSEDDOOR;
}

/* subroutine to put an item onto an empty space */
static void level_add_item(level *l, item *what)
{
    position pos;

    pos = level_find_space(l, LE_ITEM);

    inv_add(level_ilist_at(l, pos), what);
}

static level_path *level_path_new(position start, position goal)
{
    level_path *path;

    path = g_malloc0(sizeof(level_path));

    path->open = g_ptr_array_new();
    path->closed = g_ptr_array_new();
    path->path = g_queue_new();

    path->start = start;
    path->goal  = goal;

    return path;
}

static level_path_element *level_path_element_new(position pos)
{
    level_path_element *lpe;

    lpe = g_malloc0(sizeof(level_path_element));
    lpe->pos = pos;

    return lpe;
}

/* Returns cost from position defined by element to goal.*/
static int level_path_cost(level *l, level_path_element* element, position target)
{
    element->h_score = pos_distance(element->pos, target);

    /* penalize fields occupied by monsters */
    if (level_is_monster_at(l, element->pos))
        element->h_score += 10;

    /* penalize fields covered with water, fire or cloud */
    if ((level_tiletype_at(l, element->pos) == LT_FIRE)
            || (level_tiletype_at(l, element->pos) == LT_WATER)
            || (level_tiletype_at(l, element->pos) == LT_CLOUD))
    {
        element->h_score += 50;
    }


    return element->g_score + element->h_score;
}

static level_path_element *level_path_element_in_list(level_path_element* el, GPtrArray *list)
{
    guint idx;
    level_path_element *li;

    assert(el != NULL && list != NULL);

    for (idx = 0; idx < list->len; idx++)
    {
        li = g_ptr_array_index(list, idx);

        if (pos_identical(li->pos, el->pos))
        {
            return li;
        }
    }

    return NULL;
}

static level_path_element *level_path_find_best(level *l, level_path *path)
{
    level_path_element *el, *best = NULL;
    guint idx;

    for (idx = 0; idx < path->open->len; idx++)
    {
        el = g_ptr_array_index(path->open, idx);

        if (best == NULL || level_path_cost(l, el, path->goal) < level_path_cost(l, best, path->goal))
        {
            best = el;
        }
    }

    return best;
}

static GPtrArray *level_path_get_neighbours(level *l, position pos)
{
    GPtrArray *neighbours;
    level_path_element *pe;
    position npos;
    direction dir;

    neighbours = g_ptr_array_new();

    for (dir = GD_NONE + 1; dir < GD_MAX; dir++)
    {
        if (dir == GD_CURR)
            continue;

        npos = pos_move(pos, dir);

        if (pos_valid(npos)
                && lt_is_passable(level_tiletype_at(l, npos)))
        {
            pe = level_path_element_new(npos);
            g_ptr_array_add(neighbours, pe);
        }
    }

    return neighbours;
}
