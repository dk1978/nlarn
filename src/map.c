/*
 * map.c
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

#include "container.h"
#include "display.h"
#include "items.h"
#include "map.h"
#include "nlarn.h"
#include "spheres.h"

static int map_fill_with_stationary_objects(map *maze);
static void map_fill_with_objects(map *l);
static void map_fill_with_traps(map *l);

static int map_load_from_file(map *l, char *mazefile, int which);
static void map_make_maze(map *maze, int treasure_room);
static void map_make_maze_eat(map *l, int x, int y);
static void map_make_river(map *map, int rivertype);
static void map_make_lake(map *map, int laketype);
static void map_make_treasure_room(map *maze, rectangle **rooms);
static int map_validate(map *maze);

static map_path *map_path_new(position start, position goal);
static map_path_element *map_path_element_new(position pos);
static int map_path_cost(map *l, map_path_element* element, position target);
static map_path_element *map_path_element_in_list(map_path_element* el, GPtrArray *list);
static map_path_element *map_path_find_best(map *l, map_path *path);
static GPtrArray *map_path_get_neighbours(map *l, position pos,
                                          map_element_t element);

static gboolean map_monster_destroy(gpointer key, monster *monst, map *m);
static gboolean map_sphere_destroy(sphere *s, map *m);

const map_tile_data map_tiles[LT_MAX] =
{
    /* type         img  color          desc           pa tr */
    { LT_NONE,      ' ', DC_NONE,       NULL,          0, 0 },
    { LT_MOUNTAIN,  '^', DC_LIGHTGRAY,  "a mountain",  0, 0 },
    { LT_GRASS,     '"', DC_LIGHTGREEN, "grass",       1, 1 },
    { LT_DIRT,      '.', DC_BROWN,      "dirt",        1, 1 },
    { LT_TREE,      '&', DC_GREEN,      "a tree",      0, 0 },
    { LT_FLOOR,     '.', DC_LIGHTGRAY,  "floor",       1, 1 },
    { LT_WATER,     '~', DC_LIGHTBLUE,  "water",       1, 1 },
    { LT_DEEPWATER, '~', DC_BLUE,       "deep water",  0, 1 },
    { LT_LAVA,      '~', DC_RED,        "lava",        0, 1 },
    { LT_FIRE,      '*', DC_RED,        "fire",        1, 1 },
    { LT_CLOUD,     '*', DC_WHITE,      "a gas cloud", 1, 1 },
    { LT_WALL,      '#', DC_LIGHTGRAY,  "a wall",      0, 0 },
};

const map_sobject_data map_sobjects[LS_MAX] =
{
    /* type             img   color         desc                                   pa tr */
    { LS_NONE,          ' ',  DC_NONE,      NULL,                                  1, 1, },
    { LS_ALTAR,         '_',  DC_WHITE,     "a holy altar",                        1, 1, },
    { LS_THRONE,        '\\', DC_MAGENTA,   "a handsome, jewel-encrusted throne",  1, 1, },
    { LS_THRONE2,       '\\', DC_MAGENTA,   "a handsome, jewel-encrusted throne",  1, 1, },
    { LS_DEADTHRONE,    '\\', DC_LIGHTGRAY, "a massive throne",                    1, 1, },
    { LS_STAIRSDOWN,    '>',  DC_WHITE,     "a circular staircase",                1, 1, },
    { LS_STAIRSUP,      '<',  DC_WHITE,     "a circular staircase",                1, 1, },
    { LS_ELEVATORDOWN,  'I',  DC_LIGHTGRAY, "a volcanic shaft leaning downward",   1, 1, },
    { LS_ELEVATORUP,    'I',  DC_WHITE,     "the base of a volcanic shaft",        1, 1, },
    { LS_FOUNTAIN,      '{',  DC_BLUE,      "a bubbling fountain",                 1, 1, },
    { LS_DEADFOUNTAIN,  '{',  DC_LIGHTGRAY, "a dead fountain",                     1, 1, },
    { LS_STATUE,        '|',  DC_LIGHTGRAY, "a great marble statue",               1, 1, },
    { LS_URN,           'u',  DC_YELLOW,    "a golden urn",                        1, 1, },
    { LS_MIRROR,        '|',  DC_WHITE,     "a mirror",                            1, 1, },
    { LS_OPENDOOR,      '-',  DC_BROWN,     "an open door",                        1, 1, },
    { LS_CLOSEDDOOR,    '+',  DC_BROWN,     "a closed door",                       0, 0, },
    { LS_DNGN_ENTRANCE, 'O',  DC_LIGHTGRAY, "the dungeon entrance",                1, 1, },
    { LS_DNGN_EXIT,     'O',  DC_WHITE,     "the exit to town",                    1, 1, },
    { LS_HOME,          'H',  DC_LIGHTGRAY, "your home",                           1, 0, },
    { LS_DNDSTORE,      'D',  DC_LIGHTGRAY, "a DND store",                         1, 0, },
    { LS_TRADEPOST,     'T',  DC_LIGHTGRAY, "the Larn trading Post",               1, 0, },
    { LS_LRS,           'L',  DC_LIGHTGRAY, "an LRS office",                       1, 0, },
    { LS_SCHOOL,        'S',  DC_LIGHTGRAY, "the College of Larn",                 1, 0, },
    { LS_BANK,          'B',  DC_LIGHTGRAY, "the bank of Larn",                    1, 0, },
    { LS_BANK2,         'B',  DC_WHITE,     "a branch office of the bank of Larn", 1, 0, },
};

/* keep track which levels have been used before */
static int map_used[MAP_MAZE_NUM + 1] = { 1, 0 };

const char *map_names[MAP_MAX] =
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

static gboolean is_volcano_map(int nlevel)
{
    return (nlevel >= MAP_DMAX);
}

map *map_new(int num, char *mazefile)
{
    gboolean map_loaded = FALSE;
    gboolean treasure_room = FALSE;
    gboolean keep_maze = TRUE;

    map *nmap = nlarn->maps[num] = g_malloc0(sizeof(map));
    nmap->nlevel = num;

    /* create map */
    if ((num == 0) /* town is stored in file */
            || (num == MAP_DMAX - 1) /* level 10 */
            || (num == MAP_MAX - 1)  /* volcano level 3 */
            || (num > 1 && chance(25)))
    {
        /* read maze from data file */
        map_loaded = map_load_from_file(nmap, mazefile, (num == 0) ? 0 : -1);

        /* add stationary objects (not to the town) */
        if (num > 0)
        {
            if (!map_fill_with_stationary_objects(nmap))
            {
                /* adding stationary objects failed; generate a new map */
                map_destroy(nmap);
                return NULL;
            }
        }
    }

    if (!map_loaded)
    {
        /* determine if to add treasure room */
        treasure_room = num > 1 && chance(25);

        /* generate random map */
        do
        {
            /* dig cave */
            map_make_maze(nmap, treasure_room);

            /* check if entire map is reachable */
            keep_maze = map_validate(nmap);
        }
        while (!keep_maze);

        /* add monsters */
        map_fill_with_life(nmap);
    }

    if (num == 0)
    {
        // Place 4 town people on the map.
        int i;
        for (i = 0; i < 4; i++)
        {
            position pos = map_find_space(nmap, LE_MONSTER, FALSE);
            monster_new(MT_TOWN_PERSON, pos);
        }
    }
    else
    {
        /* home town is not filled with crap */
        map_fill_with_objects(nmap);

        /* and not trapped */
        map_fill_with_traps(nmap);
    }

    return nmap;
}

cJSON *map_serialize(map *m)
{
    int x, y;
    cJSON *mser, *grid, *tile;

    mser = cJSON_CreateObject();

    cJSON_AddNumberToObject(mser, "nlevel", m->nlevel);
    cJSON_AddNumberToObject(mser, "visited", m->visited);

    cJSON_AddItemToObject(mser, "grid", grid = cJSON_CreateArray());

    for (y = 0; y < MAP_MAX_Y; y++)
    {
        for (x = 0; x < MAP_MAX_X; x++)
        {
            cJSON_AddItemToArray(grid, tile = cJSON_CreateObject());

            cJSON_AddNumberToObject(tile, "type", m->grid[y][x].type);

            if (m->grid[y][x].base_type > 0
                    && m->grid[y][x].base_type != m->grid[y][x].type)
            {
                cJSON_AddNumberToObject(tile, "base_type",
                                        m->grid[y][x].base_type);
            }

            if (m->grid[y][x].sobject)
            {
                cJSON_AddNumberToObject(tile, "sobject",
                                        m->grid[y][x].sobject);
            }

            if (m->grid[y][x].trap)
            {
                cJSON_AddNumberToObject(tile, "trap",
                                        m->grid[y][x].trap);
            }

            if (m->grid[y][x].timer)
            {
                cJSON_AddNumberToObject(tile, "timer",
                                        m->grid[y][x].timer);
            }

            if (m->grid[y][x].monster)
            {
                cJSON_AddNumberToObject(tile, "monster",
                                        GPOINTER_TO_UINT(m->grid[y][x].monster));
            }

            if (m->grid[y][x].ilist )
            {
                cJSON_AddItemToObject(tile, "inventory",
                                      inv_serialize(m->grid[y][x].ilist));
            }
        }
    }

    return mser;
}

map *map_deserialize(cJSON *mser, game *g)
{
    int x, y;
    cJSON *grid, *tile, *obj;
    map *m;

    m = g_malloc0(sizeof(map));

    m->nlevel = cJSON_GetObjectItem(mser, "nlevel")->valueint;
    m->visited = cJSON_GetObjectItem(mser, "visited")->valueint;

    grid = cJSON_GetObjectItem(mser, "grid");

    for (y = 0; y < MAP_MAX_Y; y++)
    {
        for (x = 0; x < MAP_MAX_X; x++)
        {
            tile = cJSON_GetArrayItem(grid, x + (y * MAP_MAX_X));

            m->grid[y][x].type = cJSON_GetObjectItem(tile, "type")->valueint;

            obj = cJSON_GetObjectItem(tile, "base_type");
            if (obj != NULL) m->grid[y][x].base_type = obj->valueint;

            obj = cJSON_GetObjectItem(tile, "sobject");
            if (obj != NULL) m->grid[y][x].sobject = obj->valueint;

            obj = cJSON_GetObjectItem(tile, "trap");
            if (obj != NULL) m->grid[y][x].trap = obj->valueint;

            obj = cJSON_GetObjectItem(tile, "timer");
            if (obj != NULL) m->grid[y][x].timer = obj->valueint;

            obj = cJSON_GetObjectItem(tile, "monster");
            if (obj != NULL) m->grid[y][x].monster = GUINT_TO_POINTER(obj->valueint);

            obj = cJSON_GetObjectItem(tile, "inventory");
            if (obj != NULL) m->grid[y][x].ilist = inv_deserialize(obj);
        }
    }

    return m;
}

char *map_dump(map *l, position ppos)
{
    position pos;
    GString *map;
    monster *m;

    map = g_string_new_len(NULL, MAP_SIZE);

    pos.z = l->nlevel;

    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < MAP_MAX_X; pos.x++)
        {
            if (pos_identical(pos, ppos))
            {
                g_string_append_c(map, '@');
            }
            else if ((m = map_get_monster_at(l, pos)))
            {
                g_string_append_c(map, monster_glyph(m));
            }
            else if (map_trap_at(l, pos))
            {
                g_string_append_c(map, '^');
            }
            else if (map_sobject_at(l, pos))
            {
                g_string_append_c(map, ls_get_image(map_sobject_at(l, pos)));
            }
            else
            {
                g_string_append_c(map, lt_get_image(map_tiletype_at(l, pos)));
            }
        }
        g_string_append_c(map, '\n');
    }

    return g_string_free(map, FALSE);
}

void map_destroy(map *m)
{
    int x, y;

    assert(m != NULL);

    /* destroy monster and spheres on this level */
    g_hash_table_foreach_remove(nlarn->monsters, (GHRFunc) map_monster_destroy, m);
    g_ptr_array_foreach(nlarn->spheres, (GFunc)map_sphere_destroy, m);

    /* free items */
    for (y = 0; y < MAP_MAX_Y; y++)
        for (x = 0; x < MAP_MAX_X; x++)
        {
            if (m->grid[y][x].ilist != NULL)
                inv_destroy(m->grid[y][x].ilist, TRUE);
        }

    g_free(m);
}

/* return coordinates of a free space */
position map_find_space(map *maze, map_element_t element, int dead_end)
{
    rectangle entire_map = rect_new(1, 1, MAP_MAX_X - 2, MAP_MAX_Y - 2);

    return map_find_space_in(maze, entire_map, element, dead_end);
}

position map_find_space_in(map *maze, rectangle where, map_element_t element, int dead_end)
{
    position pos;
    int count, iteration = 0;

    assert (maze != NULL && element > LE_NONE && element < LE_MAX);

    pos.x = rand_m_n(where.x1, where.x2);
    pos.y = rand_m_n(where.y1, where.y2);
    pos.z = maze->nlevel;

    /* number of positions inside the rectangle */
    count = (where.x2 - where.x1 + 1) * (where.y2 - where.y1 + 1);

    do
    {
        pos.x++;

        if (pos.x > where.x2)
        {
            pos.x = where.x1;
            pos.y++;
        }

        if (pos.y > where.y2)
        {
            pos.y = where.y1;
        }

        iteration++;
    }
    while (!map_pos_validate(maze, pos, element, dead_end) && (iteration <= count));

    if (iteration > count )
    {
        pos.x = G_MAXINT16;
        pos.y = G_MAXINT16;
    }

    return pos;
}

int *map_get_surrounding(map *l, position pos, map_sobject_t type)
{
    position p;
    int move = 1;
    int *dirs;

    dirs = g_malloc0(sizeof(int) * GD_MAX);

    while (move < GD_MAX)
    {
        p = pos_move(pos, move);

        if (pos_valid(p) && map_sobject_at(l, p) == type)
        {
            dirs[move] = TRUE;
        }

        move++;
    }

    return dirs;
}

position map_find_sobject_in(map *l, map_sobject_t sobject, rectangle area)
{
    position pos;

    assert(l != NULL);

    pos.z = l->nlevel;

    for (pos.y = area.y1; pos.y <= area.y2; pos.y++)
        for (pos.x = area.x1; pos.x <= area.x2; pos.x++)
            if (map_sobject_at(l,pos) == sobject)
                return pos;

    /* if we reach this point, the sobject is not on the map */
    return pos_new(G_MAXINT16, G_MAXINT16, G_MAXINT16);
}

position map_find_sobject(map *l, map_sobject_t sobject)
{
    position pos;

    assert(l != NULL);

    pos.z = l->nlevel;

    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
        for (pos.x = 0; pos.x < MAP_MAX_X; pos.x++)
            if (map_sobject_at(l,pos) == sobject)
                return pos;

    /* if we reach this point, the sobject is not on the map */
    return pos_new(G_MAXINT16, G_MAXINT16, G_MAXINT16);
}

gboolean map_pos_validate(map *l, position pos, map_element_t element,
                          int dead_end)
{
    map_tile *tile;

    assert(l != NULL && element > LT_NONE && element < LE_MAX);

    /* if the position is invalid it is invalid for the map as well */
    if (!pos_valid(pos))
        return FALSE;

    /* if the position is on another map it is invalid for this level */
    if (pos.z != l->nlevel)
        return FALSE;

    /* make shortcut */
    tile = map_tile_at(l, pos);

    /* check for an dead end */
    if (dead_end)
    {
        int wall_count = 0;
        position p = pos;

        for (p.y = pos.y -1; p.y < pos.y + 2; p.y++)
            for (p.x = pos.x -1; p.x < pos.x + 2; p.x++)
                if (map_tiletype_at(l, p) == LT_WALL)
                    wall_count++;

        if (wall_count < 7)
        {
            /* not enclosed by walls */
            return FALSE;
        }
    }

    switch (element)
    {
    case LE_GROUND:
        /* why should we need this case? */
        return TRUE;
        break;

    case LE_SOBJECT:
        if (lt_is_passable(tile->type) && (tile->sobject == LS_NONE) )
        {
            /* find free space */
            position p = pos;

            for (p.y = pos.y -1; p.y < pos.y + 2; p.y++)
                for (p.x = pos.x -1; p.x < pos.x + 2; p.x++)
                    if (map_sobject_at(l, p) != LS_NONE)
                        return FALSE;

            return TRUE;

        }
        break;

    case LE_TRAP:
        if (lt_is_passable(tile->type)
                && (tile->sobject == LS_NONE)
                && (tile->trap == TT_NONE))
        {
            return TRUE;
        }
        break;

    case LE_ITEM:
        /* we can stack like mad, so we only need to check if there is an open space */
        if (map_pos_passable(l, pos) && (tile->sobject == LS_NONE))
            return TRUE;
        break;

    case LE_MONSTER:
    case LE_SWIMMING_MONSTER:
    case LE_FLYING_MONSTER:
    case LE_XORN:
        /* not ok if player is standing on that tile */
        if (pos_identical(pos, nlarn->p->pos))
            return FALSE;

        if (map_is_monster_at(l, pos))
            return FALSE;

        return valid_monster_movement_pos(l, pos, element);
        break;

    case LE_NONE:
    case LE_MAX:
        return FALSE;
        break;

    } /* switch */

    return FALSE;
}

int map_pos_is_visible(map *l, position s, position t)
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

            if (!lt_is_transparent(l->grid[y][x].type)
                    || !ls_is_transparent(l->grid[y][x].sobject))
            {
                return FALSE;
            }
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

            if (!lt_is_transparent(l->grid[y][x].type)
                    || !ls_is_transparent(l->grid[y][x].sobject))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

map_path *map_find_path(map *l, position start, position goal,
                        map_element_t element)
{
    assert(l != NULL && (start.z == goal.z));

    map_path *path;
    map_path_element *curr, *next;
    gboolean next_is_better;
    GPtrArray *neighbours;

    path = map_path_new(start, goal);

    /* add start to open list */
    curr = map_path_element_new(start);
    g_ptr_array_add(path->open, curr);

    while (path->open->len)
    {
        curr = map_path_find_best(l, path);

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

        neighbours = map_path_get_neighbours(l, curr->pos, element);

        while (neighbours->len)
        {
            next = g_ptr_array_remove_index_fast(neighbours,
                                                 neighbours->len - 1);

            next_is_better = FALSE;

            if (map_path_element_in_list(next, path->closed))
            {
                g_free(next);
                continue;
            }

            if (!map_path_element_in_list(next, path->open))
            {
                g_ptr_array_add(path->open, next);
                next_is_better = TRUE;
            }
            else if (map_path_cost(l, curr, path->goal)
                        > map_path_cost(l, next, path->goal))
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

void map_path_destroy(map_path *path)
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

area *map_get_obstacles(map *l, position center, int radius)
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
            if (!pos_valid(pos) || !map_pos_transparent(l, pos))
            {
                area_point_set(narea, x, y);
            }
        }
    }

    return narea;
}

void map_set_tiletype(map *l, area *area, map_tile_t type, guint8 duration)
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
            /* check if pos is inside the map */
            if (!pos_valid(pos))
            {
                continue;
            }

            /* if the position is marked in area set the tile to type */
            if (area_point_get(area, x, y))
            {
                map_tile *tile = map_tile_at(l, pos);

                /* store original type if it has not been set already
                   (this can occur when casting multiple flood
                   spells on the same tile) */
                if (tile->base_type == LT_NONE)
                {
                    tile->base_type = map_tiletype_at(l, pos);
                }

                tile->type = type;
                tile->timer = duration;
            }
        }
    }
}

map_tile *map_tile_at(map *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return &l->grid[pos.y][pos.x];
}

inventory **map_ilist_at(map *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return &l->grid[pos.y][pos.x].ilist;
}

map_tile_t map_tiletype_at(map *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->grid[pos.y][pos.x].type;
}

void map_tiletype_set(map *l, position pos, map_tile_t type)
{
    assert(l != NULL && pos_valid(pos));

    l->grid[pos.y][pos.x].type = type;
}

map_tile_t map_basetype_at(map *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->grid[pos.y][pos.x].base_type;
}

void map_basetype_set(map *l, position pos, map_tile_t type)
{
    assert(l != NULL && pos_valid(pos));

    l->grid[pos.y][pos.x].base_type = type;
}

guint8 map_timer_at(map *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->grid[pos.y][pos.x].timer;
}

trap_t map_trap_at(map *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->grid[pos.y][pos.x].trap;
}

void map_trap_set(map *l, position pos, trap_t type)
{
    assert(l != NULL && pos_valid(pos));

    l->grid[pos.y][pos.x].trap = type;
}

map_sobject_t map_sobject_at(map *l, position pos)
{
    assert(l != NULL && pos_valid(pos));

    return l->grid[pos.y][pos.x].sobject;
}

void map_sobject_set(map *l, position pos, map_sobject_t type)
{
    assert(l != NULL && pos_valid(pos));

    l->grid[pos.y][pos.x].sobject = type;
}

damage *map_tile_damage(map *l, position pos)
{
    assert (l != NULL && pos_valid(pos));

    switch (map_tiletype_at(l, pos))
    {
    case LT_CLOUD:
        return damage_new(DAM_ACID, ATT_NONE, 3 + rand_0n(2), NULL);
        break;

    case LT_FIRE:
        return damage_new(DAM_FIRE, ATT_NONE, 5 + rand_0n(2), NULL);
        break;

    case LT_WATER:
        return damage_new(DAM_WATER, ATT_NONE, 4 + rand_0n(2), NULL);
        break;

    default:
        return NULL;
    }
}

char *map_pos_examine(position pos)
{
    map *cm = game_map(nlarn, pos.z);
    monster *monst;
    item *it;
    char item_desc[81];
    guint idx;
    char *tmp = NULL, *where;
    GString *desc = g_string_new(NULL);

    assert(pos_valid(pos));

    if (pos_identical(pos, nlarn->p->pos))
        where = "here";
    else
        where = "there";

    /* describe the level tile */
    tmp = g_strdup(lt_get_desc(map_tiletype_at(cm, pos)));
    tmp[0] = g_ascii_toupper(tmp[0]);
    g_string_append_printf(desc, "%s. ", tmp);
    g_free(tmp);

    /* add description of monster, if there is one on the tile */
    if ((monst = map_get_monster_at(cm, pos)))
    {
        if (game_wizardmode(nlarn) || monster_in_sight(monst))
        {
            tmp = monster_desc(monst);

            tmp[0] = g_ascii_toupper(tmp[0]);
            g_string_append_printf(desc, "%s. ", tmp);
            g_free(tmp);
        }
    }

    /* add message if target tile contains a stationary object */
    if (map_sobject_at(cm, pos) > LS_NONE)
    {
        g_string_append_printf(desc, "You see %s %s. ",
                               ls_get_desc(map_sobject_at(cm, pos)), where);
    }

    /* add message if target tile contains a known trap */
    if (player_memory_of(nlarn->p, pos).trap)
    {
        g_string_append_printf(desc, "There is %s %s %s. ",
                               a_an(trap_description(map_trap_at(cm, pos))),
                               trap_description(map_trap_at(cm, pos)), where);
    }

    /* add message if target tile contains items */
    if (inv_length(*map_ilist_at(cm, pos)) > 0)
    {
        if (inv_length(*map_ilist_at(cm, pos)) > 3)
        {
            g_string_append_printf(desc, "There are multiple items %s.", where);
        }
        else
        {
            GString *items_desc = NULL;

            for (idx = 0; idx < inv_length(*map_ilist_at(cm, pos)); idx++)
            {
                it = inv_get(*map_ilist_at(cm, pos), idx);
                item_describe(it, player_item_known(nlarn->p, it),
                              FALSE, FALSE, item_desc, 80);

                if (idx > 0)
                    g_string_append_printf(items_desc, " and %s", item_desc);
                else
                    items_desc = g_string_new(item_desc);
            }

            g_string_append_printf(desc, "You see %s %s.",
                                   items_desc->str, where);
            g_string_free(items_desc, TRUE);
        }
    }

    return g_string_free(desc, FALSE);
}

monster *map_get_monster_at(map *m, position pos)
{
    assert(m != NULL && m->nlevel == pos.z && pos_valid(pos));

    gpointer mid = m->grid[pos.y][pos.x].monster;
    return (mid != NULL) ? game_monster_get(nlarn, mid) : NULL;
}

int map_set_monster_at(map *map, position pos, monster *monst)
{
    assert(map != NULL && map->nlevel == pos.z && pos_valid(pos));

    map->grid[pos.y][pos.x].monster = (monst != NULL) ? monster_oid(monst) : NULL;

    return TRUE;
}

int map_is_monster_at(map *m, position pos)
{
    assert(m != NULL);

    return ((map_get_monster_at(m, pos) != NULL));
}

int map_fill_with_life(map *l)
{
    position pos;
    int new_monster_count;
    int i;

    assert(l != NULL);

    new_monster_count = rand_1n(14 + l->nlevel);

    if (l->mcount > new_monster_count)
        /* no monsters added */
        return FALSE;
    else
        new_monster_count -= l->mcount;

    for (i = 0; i <= new_monster_count; i++)
    {
        do
        {
            pos = map_find_space(l, LE_MONSTER, FALSE);
        }
        while (player_pos_visible(nlarn->p, pos));

        monster_new_by_level(pos);
    }

    return(new_monster_count);
}

gboolean map_is_exit_at(map *m, position pos)
{
    assert (m != NULL && pos_valid(pos));

    switch (map_sobject_at(m, pos))
    {
        case LS_DNGN_ENTRANCE:
        case LS_DNGN_EXIT:
        case LS_ELEVATORDOWN:
        case LS_ELEVATORUP:
        case LS_STAIRSUP:
        case LS_STAIRSDOWN:
            return TRUE;
            break;

        default:
            return FALSE;
            break;
    }
}

void map_timer(map *l)
{
    position pos;
    item_erosion_type erosion;

    assert (l != NULL);

    pos.z = l->nlevel;

    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < MAP_MAX_X; pos.x++)
        {
            if (map_timer_at(l, pos))
            {
                map_tile *tile = map_tile_at(l, pos);
                tile->timer--;

                /* affect items every three turns */
                if ((tile->ilist != NULL) && (tile->timer % 5 == 0))
                {
                    switch (tile->type)
                    {
                    case LT_CLOUD:
                        erosion = IET_CORRODE;
                        break;

                    case LT_FIRE:
                        erosion = IET_BURN;
                        break;

                    case LT_WATER:
                        erosion = IET_RUST;
                        break;
                    default:
                        erosion = IET_NONE;
                        break;
                    }

                    inv_erode(&tile->ilist, erosion, player_pos_visible(nlarn->p, pos));
                }

                /* reset tile type if temporary effect has expired */
                if (tile->timer == 0)
                {
                    if ((tile->type == LT_FIRE)
                            && (tile->type == LT_GRASS))
                    {
                        tile->type = LT_DIRT;
                    }
                    else
                    {
                        tile->type = tile->base_type;
                    }
                }
            } /* if map_timer_at */
        } /* for pos.x */
    } /* for pos.y */
}

static int map_fill_with_stationary_objects(map *maze)
{
    position pos;
    int i;						/* loop var */

    /* volcano shaft up from the temple */
    if (maze->nlevel == MAP_DMAX)
    {
        pos = map_find_space(maze, LE_SOBJECT, TRUE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_ELEVATORUP);
    }

    /*  make the fixed objects in the maze: STAIRS */
    if ((maze->nlevel > 0) && (maze->nlevel != MAP_DMAX - 1) && (maze->nlevel != MAP_MAX - 1))
    {
        pos = map_find_space(maze, LE_SOBJECT, TRUE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_STAIRSDOWN);
    }

    if ((maze->nlevel > 1) && (maze->nlevel != MAP_DMAX))
    {
        pos = map_find_space(maze, LE_SOBJECT, TRUE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_STAIRSUP);
    }

    /* make the random objects in the maze */
    /* 33 percent chance for an altar */
    if (chance(33))
    {
        pos = map_find_space(maze, LE_SOBJECT, FALSE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_ALTAR);
    }

    /* up to three statues */
    for (i = 0; i < rand_0n(3); i++)
    {
        pos = map_find_space(maze, LE_SOBJECT, FALSE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_STATUE);
    }

    /* up to three fountains */
    for (i = 0; i < rand_0n(3); i++)
    {
        pos = map_find_space(maze, LE_SOBJECT, FALSE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_FOUNTAIN);
    }

    /* up to two thrones */
    for (i = 0; i < rand_0n(2); i++)
    {
        pos = map_find_space(maze, LE_SOBJECT, FALSE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_THRONE);
    }

    /* up to two  mirrors */
    for (i = 0; i < rand_0n(2); i++)
    {
        pos = map_find_space(maze, LE_SOBJECT, FALSE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_MIRROR);
    }

    if (maze->nlevel == 5)
    {
        /* branch office of the bank */
        pos = map_find_space(maze, LE_SOBJECT, TRUE);
        if (!pos_valid(pos)) return FALSE;
        map_sobject_set(maze, pos, LS_BANK2);
    }

    return TRUE;
}

static void map_fill_with_objects(map *l)
{
    int i,j;                    /* loop vars */
    item_t it;
    item *container = NULL;

    /* up to two pieces of armour */
    for (i = 0; i < rand_0n(2); i++)
    {
        map_item_add(l, item_new_by_level(IT_ARMOUR, l->nlevel));
    }

    /* up to three books */
    for (i = 0; i <= rand_0n(3); i++)
    {
        map_item_add(l, item_new_by_level(IT_BOOK, l->nlevel));
    }

    /* up to two containers */
    for (i = 1; i <= rand_0n(2); i++)
    {
        /* random container type */
        container = item_new(IT_CONTAINER, rand_1n(CT_MAX));

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

        /* add the container to the map */
        map_item_add(l, container);
    }

    /* up to 10 piles of gold */
    for (i = 0; i < rand_0n(10); i++)
    {
        /* There is nothing like a newly minted pound. */
        map_item_add(l, item_new(IT_GOLD, rand_m_n(10, (l->nlevel + 1) * 15)));
    }

    /* up to three gems */
    for (i = 0; i < rand_0n(3); i++)
    {
        map_item_add(l, item_new_random(IT_GEM));
    }

    /* up to four potions */
    for (i = 0; i < rand_0n(4); i++)
    {
        map_item_add(l, item_new_by_level(IT_POTION, l->nlevel));
    }

    /* up to three scrolls */
    for (i = 0; i < rand_0n(3); i++)
    {
        map_item_add(l, item_new_by_level(IT_SCROLL, l->nlevel));
    }

    /* up to two rings */
    for (i = 0; i < rand_0n(3); i++)
    {
        map_item_add(l, item_new_by_level(IT_RING, l->nlevel));
    }

    /* up to two weapons */
    for (i = 0; i < rand_0n(2); i++)
    {
        map_item_add(l, item_new_by_level(IT_WEAPON, l->nlevel));
    }

} /* map_fill_with_objects */

static void map_fill_with_traps(map *l)
{
    int count;
    position pos;
    int trapdoor = FALSE;

    assert(l != NULL);

    /* Trapdoor cannot be placed in the last dungeon map and the last vulcano map */
    trapdoor = ((l->nlevel != MAP_DMAX - 1) && (l->nlevel != MAP_MAX - 1));

    for (count = 0; count < rand_0n((trapdoor ? 8 : 6)); count++)
    {
        pos = map_find_space(l, LE_TRAP, FALSE);
        map_trap_set(l, pos, rand_1n(trapdoor ? TT_MAX : TT_TRAPDOOR));
    }
} /* map_fill_with_traps */

/* subroutine to make the caverns for a given map. only walls are made. */
static void map_make_maze(map *maze, int treasure_room)
{
    position pos;
    int mx, my;
    int nrooms, room;
    rectangle **rooms = NULL;
    gboolean want_monster = FALSE;

    assert (maze != NULL);

    pos.z = maze->nlevel;

generate:
    /* reset map by filling it with walls */
    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
        for (pos.x = 0; pos.x < MAP_MAX_X; pos.x++)
        {
            monster *m;

            map_tiletype_set(maze, pos, LT_WALL);
            map_sobject_set(maze, pos, LS_NONE);

            if ((m = map_get_monster_at(maze, pos)))
            {
                monster_destroy(m);
            }

            if (map_tile_at(maze, pos)->ilist != NULL)
            {
                inv_destroy(map_tile_at(maze, pos)->ilist, TRUE);
                map_tile_at(maze, pos)->ilist = NULL;
            }
        }

    /* Maybe add a river or lake. */
    const int rivertype = (is_volcano_map(maze->nlevel) ? LT_LAVA
                                                        : LT_DEEPWATER);
    if (maze->nlevel > 1
            && (is_volcano_map(maze->nlevel) ? chance(90) : chance(40)))
    {
        if (chance(70))
            map_make_river(maze, rivertype);
        else
            map_make_lake(maze, rivertype);

        if (maze->grid[1][1].type == LT_WALL)
            map_make_maze_eat(maze, 1, 1);
    }
    else
        map_make_maze_eat(maze, 1, 1);

    /* add exit to town on map 1 */
    if (maze->nlevel == 1)
    {
        maze->grid[MAP_MAX_Y - 1][(MAP_MAX_X - 1) / 2].type = LT_FLOOR;
        maze->grid[MAP_MAX_Y - 1][(MAP_MAX_X - 1) / 2].sobject = LS_DNGN_EXIT;
    }

    /* generate open spaces */
    nrooms = rand_1n(3) + 3;
    if (treasure_room)
        nrooms++;

    rooms = g_malloc0(sizeof(rectangle *) * (nrooms + 1));

    for (room = 0; room < nrooms; room++)
    {
        rooms[room] = g_malloc0(sizeof(rectangle));

        my = rand_1n(11) + 2;
        rooms[room]->y1 = my - rand_1n(2);
        rooms[room]->y2 = my + rand_1n(2);

        if (is_volcano_map(maze->nlevel))
        {
            mx = rand_1n(60)+3;
            rooms[room]->x1 = mx - rand_1n(2);
            rooms[room]->x2 = mx + rand_1n(2);

            want_monster = TRUE;
        }
        else
        {
            mx = rand_1n(44)+5;
            rooms[room]->x1 = mx - rand_1n(4);
            rooms[room]->x2 = mx + rand_1n(12)+3;
        }

        for (pos.y = rooms[room]->y1 ; pos.y < rooms[room]->y2 ; pos.y++)
        {
            for (pos.x = rooms[room]->x1 ; pos.x < rooms[room]->x2 ; pos.x++)
            {
                map_tile *tile = map_tile_at(maze, pos);
                if (tile->type == rivertype)
                    continue;

                tile->type = LT_FLOOR;

                if (want_monster == TRUE)
                {
                    monster_new_by_level(pos);
                    want_monster = FALSE;
                }
            }
        }
    }

    /* mark the end of the rooms array */
    rooms[nrooms] = NULL;

    /* add stationary objects */
    if (!map_fill_with_stationary_objects(maze))
    {
        /* adding stationary objects failed; generate a new map */
        goto generate;
    }

    /* add treasure room if requested */
    if (treasure_room)
        map_make_treasure_room(maze, rooms);

    /* cleanup */
    for (room = 0; room < nrooms; room++)
        g_free(rooms[room]);

    g_free(rooms);
}

/* function to eat away a filled in maze */
static void map_make_maze_eat(map *l, int x, int y)
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
                    (l->grid[y][x - 1].type == LT_WALL) &&
                    (l->grid[y][x - 2].type == LT_WALL))
            {
                l->grid[y][x - 1].type = l->grid[y][x - 2].type = LT_FLOOR;
                map_make_maze_eat(l, x - 2, y);
            }
            break;

        case 2: /* east */
            if (x < (MAP_MAX_X - 3) &&
                    (l->grid[y][x + 1].type == LT_WALL) &&
                    (l->grid[y][x + 2].type == LT_WALL))
            {
                l->grid[y][x + 1].type = l->grid[y][x + 2].type = LT_FLOOR;
                map_make_maze_eat(l, x + 2, y);
            }
            break;

        case 3: /* south */
            if ((y > 2) &&
                    (l->grid[y - 1][x].type == LT_WALL) &&
                    (l->grid[y - 2][x].type == LT_WALL))
            {
                l->grid[y - 1][x].type = l->grid[y - 2][x].type = LT_FLOOR;
                map_make_maze_eat(l, x, y - 2);
            }
            break;

        case 4: /* north */
            if ((y < MAP_MAX_Y - 3) &&
                    (l->grid[y + 1][x].type == LT_WALL) &&
                    (l->grid[y + 2][x].type == LT_WALL))
            {
                l->grid[y + 1][x].type = l->grid[y + 2][x].type = LT_FLOOR;
                map_make_maze_eat(l, x, y + 2);
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

/* The river/lake creation algorithm has been copied in entirety
   from Dungeon Crawl Stone Soup, with only very slight changes. (jpeg) */
static void map_make_vertical_river(map *map, int rivertype)
{
    guint width  = 3 + rand_0n(4);
    guint startx = 6 - width + rand_0n(MAP_MAX_X - 8);

    const guint starty = rand_1n(4);
    const guint endy   = MAP_MAX_Y - (4 - starty);
    const guint minx   = rand_1n(3);
    const guint maxx   = MAP_MAX_X - rand_1n(3);

    position pos;
    pos.z = map->nlevel;
    for (pos.y = starty; pos.y < endy; pos.y++)
    {
        if (chance(33)) startx++;
        if (chance(33)) startx--;
        if (chance(50)) width++;
        if (chance(50)) width--;

        if (width < 2) width = 2;
        if (width > 6) width = 6;

        for (pos.x = startx; pos.x < startx + width; pos.x++)
        {
            if (pos.x > minx && pos.x < maxx && chance(99))
                map_tiletype_set(map, pos, rivertype);
        }
    }
}

static void map_make_river(map *map, int rivertype)
{
    if (chance(20))
    {
        map_make_vertical_river(map, rivertype);
        return;
    }

    guint width  = 3 + rand_0n(4);
    guint starty = 10 - width + rand_0n(MAP_MAX_Y - 12);

    const guint startx = rand_1n(7);
    const guint endx   = MAP_MAX_X - (7 - startx);
    const guint miny   = rand_1n(3);
    const guint maxy   = MAP_MAX_Y - rand_1n(3);

    position pos;
    pos.z = map->nlevel;
    for (pos.x = startx; pos.x < endx; pos.x++)
    {
        if (chance(33)) starty++;
        if (chance(33)) starty--;
        if (chance(50)) width++;
        if (chance(50)) width--;

        if (width < 2) width = 2;
        if (width > 6) width = 6;

        for (pos.y = starty; pos.y < starty + width; pos.y++)
        {
            if (pos.y > miny && pos.y < maxy && chance(99))
                map_tiletype_set(map, pos, rivertype);
        }
    }
}

static void map_make_lake(map *map, int laketype)
{
    guint x1 = 5 + rand_0n(MAP_MAX_X - 30);
    guint y1 = 3 + rand_0n(MAP_MAX_Y - 15);
    guint x2 = x1 + 4 + rand_0n(16);
    guint y2 = y1 + 4 + rand_0n(5);

    position pos;
    pos.z = map->nlevel;
    for (pos.y = y1; pos.y < y2; pos.y++)
    {
        if (pos.y <= 1 || pos.y >= MAP_MAX_Y - 1)
            continue;

        if (chance(50))  x1 += rand_0n(3);
        if (chance(50))  x1 -= rand_0n(3);
        if (chance(50))  x2 += rand_0n(3);
        if (chance(50))  x2 -= rand_0n(3);

        if (pos.y - y1 < (y2 - y1) / 2)
        {
            x2 += rand_0n(3);
            x1 -= rand_0n(3);
        }
        else
        {
            x2 -= rand_0n(3);
            x1 += rand_0n(3);
        }

        for (pos.x = x1; pos.x < x2 ; pos.x++)
        {
            if (pos.x <= 1 || pos.x >= MAP_MAX_X - 1)
                continue;

            if (chance(99))
                map_tiletype_set(map, pos, laketype);
        }
    }
}

/*
 *  function to read in a maze from a data file
 *
 *  Format of maze data file:
 *  For each maze:  MAP_MAX_Y + 1 lines (MAP_MAX_Y used)
 *                  MAP_MAX_X characters per line
 *
 *  Special characters in maze data file:
 *
 *      #   wall
 *      +   door
 *      M   random monster
 *      *   eye of larn
 *      !   potion of cure dianthroritis
 *      -   random object
 */
static int map_load_from_file(map *nmap, char *mazefile, int which)
{
    position pos;       /* current position on map */
    int map_num = 0;    /* number of selected map */
    item_t it;          /* item type for random objects */

    FILE *levelfile;

    if (!(levelfile = fopen(mazefile, "r")))
    {
        /* maze file cannot be opened */
        return FALSE;
    }

    if (feof(levelfile))
    {
        /* FIXME: debug output */
        fclose(levelfile);

        return FALSE;
    }

    /* FIXME: calculate how many levels are in the file  */

    /* roll the dice: which map? */
    if (which >= 0 && which <= MAP_MAX_MAZE_NUM)
    {
        map_num = which;
    }
    else
    {
        int tries = 0;
        do
        {
            map_num = rand_1n(MAP_MAX_MAZE_NUM);
        }
        while (map_used[map_num] && ++tries < 100);

        map_used[map_num] = TRUE;
    }

    /* advance to desired maze */
    fseek(levelfile, (map_num * ((MAP_MAX_X + 1) * MAP_MAX_Y + 1)), SEEK_SET);

    if (feof(levelfile))
    {
        /* FIXME: debug output */
        fclose(levelfile);

        return FALSE;
    }

    pos.z = nmap->nlevel;

    // Sometimes flip the maps. (Never the town)
    gboolean flip_vertical   = (map_num > 0 && chance(50));
    gboolean flip_horizontal = (map_num > 0 && chance(50));

    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < MAP_MAX_X ; pos.x++)
        {
            position map_pos = pos;
            if (flip_vertical)
                map_pos.x = MAP_MAX_X - pos.x - 1;
            if (flip_horizontal)
                map_pos.y = MAP_MAX_Y - pos.y - 1;

            map_tile *tile = map_tile_at(nmap, map_pos);

            tile->type = LT_FLOOR;	/* floor is default */

            switch (fgetc(levelfile))
            {

            case '^': /* mountain */
                tile->type = LT_MOUNTAIN;
                break;

            case '"': /* grass */
                tile->type = LT_GRASS;
                break;

            case '.': /* dirt */
                tile->type = LT_DIRT;
                break;

            case '&': /* tree */
                tile->type = LT_TREE;
                break;

            case '~': /* deep water */
                tile->type = LT_DEEPWATER;
                break;

            case '=': /* lava */
                tile->type = LT_LAVA;
                break;

            case '#': /* wall */
                tile->type =  LT_WALL;
                break;

            case '_': /* altar */
                tile->sobject = LS_ALTAR;
                break;

            case '+': /* door */
                tile->sobject = LS_CLOSEDDOOR;
                break;

            case 'O': /* dungeon entrance */
                tile->sobject = LS_DNGN_ENTRANCE;
                break;

            case 'I': /* elevator */
                tile->sobject = LS_ELEVATORDOWN;
                break;

            case 'H': /* home */
                tile->sobject = LS_HOME;
                break;

            case 'D': /* dnd store */
                tile->sobject = LS_DNDSTORE;
                break;

            case 'T': /* trede post */
                tile->sobject = LS_TRADEPOST;
                break;

            case 'L': /* LRS */
                tile->sobject = LS_LRS;
                break;

            case 'S': /* school */
                tile->sobject = LS_SCHOOL;
                break;

            case 'B': /*  */
                tile->sobject = LS_BANK;
                break;

            case '*': /* eye of larn */
                if (nmap->nlevel != MAP_DMAX - 1)
                {
                    break;
                }
                inv_add(&tile->ilist, item_new(IT_AMULET, AM_LARN));

                monster_new(MT_DEMONLORD_I + rand_0n(7), map_pos);
                break;

            case '!':	/* potion of cure dianthroritis */
                if (nmap->nlevel != MAP_MAX - 1)
                    break;

                inv_add(&tile->ilist, item_new(IT_POTION, PO_CURE_DIANTHR));
                monster_new(MT_DEMON_PRINCE, map_pos);
                break;

            case 'M':	/* random monster */
                monster_new_by_level(map_pos);
                break;

            case '-':
                do
                {
                    it = rand_1n(IT_MAX - 1);
                }
                while (it == IT_CONTAINER);

                inv_add(&tile->ilist, item_new_by_level(it, nmap->nlevel));
                break;
            };
        }
        (void)fgetc(levelfile); /* eat EOL */
    }

    fclose(levelfile);

    return TRUE;
}

/*
 * function to make a treasure room on a map
 */
static void map_make_treasure_room(map *maze, rectangle **rooms)
{
    position pos, npos;
    map_sobject_t mst;
    item *itm;
    int success;

    int nrooms; /* count of available rooms */
    int room;   /* number of chose room */

    /* determine number of rooms */
    for (nrooms = 0; rooms[nrooms] != NULL; nrooms++);

    /* choose a room to turn into an treasure room */
    room = rand_0n(nrooms);

    pos.z = npos.z = maze->nlevel;

    for (pos.y = rooms[room]->y1; pos.y <= rooms[room]->y2; pos.y++)
    {
        for (pos.x = rooms[room]->x1; pos.x <= rooms[room]->x2; pos.x++)
        {
            if ( (pos.y == rooms[room]->y1)
                    || (pos.y == rooms[room]->y2)
                    || (pos.x == rooms[room]->x1)
                    || (pos.x == rooms[room]->x2) )
            {
                /* if we are on the border of a room, make wall */
                map_tiletype_set(maze, pos, LT_WALL);
            }
            else
            {
                /* make sure there's floor here */
                map_tiletype_set(maze, pos, LT_FLOOR);

                /* create loot */
                itm = item_new_random(IT_GOLD);
                inv_add(map_ilist_at(maze, pos), itm);

                /* create a monster */
                monster_new_by_level(pos);
            }

            /* now clear out interior */
            if ((mst = map_sobject_at(maze, pos)))
            {
                success = FALSE;
                do
                {

                    npos = map_find_space(maze, LE_SOBJECT, FALSE);
                    if (!pos_in_rect(npos, *rooms[room]))
                    {
                        /* pos is outside of room */
                        map_sobject_set(maze, npos, mst);
                        map_sobject_set(maze, pos, LS_NONE);

                        success = TRUE;
                    }
                }
                while (!success);
            } /* if map_sobject_at() */
        } /* for x */
    } /* for y */

    /* place the door on the treasure room */
    switch (rand_1n(2))
    {
    case 1: /* horizontal */
        pos.x = rand_m_n(rooms[room]->x1 + 1, rooms[room]->x2 - 1);
        pos.y = rand_0n(1) ? rooms[room]->y1 : rooms[room]->y2;
        break;

    case 2: /* vertical */
        pos.x = rand_0n(1) ? rooms[room]->x1 : rooms[room]->x2;
        pos.y = rand_m_n(rooms[room]->y1 + 1, rooms[room]->y2 - 1);
        break;
    };

    map_tiletype_set(maze, pos, LT_FLOOR);
    map_sobject_set(maze, pos, LS_CLOSEDDOOR);
}

/* verify that every space on the map can be reached */
static int map_validate(map *maze)
{
    position pos;
    int connected = TRUE;
    area *floodmap = NULL;
    area *obsmap = area_new(0, 0, MAP_MAX_X, MAP_MAX_Y);

    pos.z = maze->nlevel;

    /* generate an obstacle map */
    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
        for (pos.x = 0; pos.x < MAP_MAX_X; pos.x++)
            if (!map_pos_passable(maze, pos)
                    && (map_sobject_at(maze, pos) != LS_CLOSEDDOOR))
            {
                area_point_set(obsmap, pos.x, pos.y);
            }

    /* get position of entrance */
    switch (maze->nlevel)
    {
        /* caverns entrance */
    case 1:
        pos = map_find_sobject(maze, LS_DNGN_EXIT);
        break;

        /* volcano entrance */
    case MAP_DMAX:
        pos = map_find_sobject(maze, LS_ELEVATORUP);
        break;

    default:
        pos = map_find_sobject(maze, LS_STAIRSDOWN);
        break;
    }

    /* flood fill the maze starting at the entrance */
    floodmap = area_flood(obsmap, pos.x, pos.y);

    /* compare flooded area with obstacle map */
    for (pos.y = 0; pos.y < MAP_MAX_Y; pos.y++)
    {
        for (pos.x = 0; pos.x < MAP_MAX_X; pos.x++)
        {
            int pp = map_pos_passable(maze, pos);
            int cd = (map_sobject_at(maze, pos) == LS_CLOSEDDOOR);

            /* point should be set on floodmap if it is passable */
            if (area_point_get(floodmap, pos.x, pos.y) != (pp || cd))
            {
                connected = FALSE;
                break;
            }
        }

        if (!connected)
            break;
    }

    area_destroy(floodmap);

    return connected;
}

/* subroutine to put an item onto an empty space */
void map_item_add(map *maze, item *what)
{
    position pos;

    pos = map_find_space(maze, LE_ITEM, FALSE);

    inv_add(map_ilist_at(maze, pos), what);
}

static map_path *map_path_new(position start, position goal)
{
    map_path *path;

    path = g_malloc0(sizeof(map_path));

    path->open   = g_ptr_array_new();
    path->closed = g_ptr_array_new();
    path->path   = g_queue_new();

    path->start = start;
    path->goal  = goal;

    return path;
}

static map_path_element *map_path_element_new(position pos)
{
    map_path_element *lpe;

    lpe = g_malloc0(sizeof(map_path_element));
    lpe->pos = pos;

    return lpe;
}

/* Returns cost from position defined by element to goal.*/
static int map_path_cost(map *l, map_path_element* element, position target)
{
    element->h_score = pos_distance(element->pos, target);

    /* penalize fields occupied by monsters */
    if (map_is_monster_at(l, element->pos))
        element->h_score += 10;

    /* penalize fields covered with water, fire or cloud */
    if ((map_tiletype_at(l, element->pos) == LT_FIRE)
            || (map_tiletype_at(l, element->pos) == LT_WATER)
            || (map_tiletype_at(l, element->pos) == LT_CLOUD))
    {
        element->h_score += 50;
    }

    return element->g_score + element->h_score;
}

static map_path_element *map_path_element_in_list(map_path_element* el, GPtrArray *list)
{
    guint idx;
    map_path_element *li;

    assert(el != NULL && list != NULL);

    for (idx = 0; idx < list->len; idx++)
    {
        li = g_ptr_array_index(list, idx);

        if (pos_identical(li->pos, el->pos))
            return li;
    }

    return NULL;
}

static map_path_element *map_path_find_best(map *l, map_path *path)
{
    map_path_element *el, *best = NULL;
    guint idx;

    for (idx = 0; idx < path->open->len; idx++)
    {
        el = g_ptr_array_index(path->open, idx);

        if (best == NULL || map_path_cost(l, el, path->goal)
                                < map_path_cost(l, best, path->goal))
        {
            best = el;
        }
    }

    return best;
}

static GPtrArray *map_path_get_neighbours(map *l, position pos,
                                          map_element_t element)
{
    GPtrArray *neighbours;
    map_path_element *pe;
    position npos;
    direction dir;

    neighbours = g_ptr_array_new();

    for (dir = GD_NONE + 1; dir < GD_MAX; dir++)
    {
        if (dir == GD_CURR)
            continue;

        npos = pos_move(pos, dir);

        if (pos_valid(npos) && valid_monster_movement_pos(l, npos, element))
        {
            pe = map_path_element_new(npos);
            g_ptr_array_add(neighbours, pe);
        }
    }

    return neighbours;
}

static gboolean map_monster_destroy(gpointer key, monster *monst, map *m)
{
    if (monster_pos(monst).z != m->nlevel)
    {
        return FALSE;
    }

    monster_oid_set(monst, 0);
    monster_destroy(monst);

    return TRUE;

}

static gboolean map_sphere_destroy(sphere *s, map *m)
{
    if (s->pos.z != m->nlevel)
        return FALSE;

    sphere_destroy(s, nlarn);

    return TRUE;
}
