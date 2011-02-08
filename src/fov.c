/*
 * fov.c
 * Copyright (C) 2009, 2010, 2011 Joachim de Groot <jdegroot@web.de>
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
#include <string.h>
#include "fov.h"
#include "game.h"
#include "map.h"
#include "nlarn.h"
#include "position.h"

static void fov_calculate_octant(fov *fov, map *m, position center,
                                 int infravision, int row, float start,
                                 float end, int radius, int xx,
                                 int xy, int yx, int yy);

static gint fov_visible_monster_sort(gconstpointer a, gconstpointer b, gpointer center);

struct _fov
{
    /* the actual field of vision */
    guchar data[MAP_MAX_Y][MAP_MAX_X];

    /* the center of the fov */
    position center;

    /* The list of visible monsters - it's a hash as fields may get visited
       twice, which means that monsters may get added to the list multiple
       times. The hash overwrites duplicate values. */
    GHashTable *mlist;
};

fov *fov_new()
{
    fov *nfov = g_malloc0(sizeof(fov));
    nfov->center = pos_invalid;
    nfov->mlist = g_hash_table_new(g_direct_hash, g_direct_equal);

    return nfov;
}
/* this and the function fov_calculate_octant() have been
 * ported from python to c using the example at
 * http://roguebasin.roguelikedevelopment.org/index.php?title=Python_shadowcasting_implementation
 */
void fov_calculate(fov *fov, map *m, position pos, int radius, gboolean infravision)
{
    const int mult[4][8] =
    {
        { 1,  0,  0, -1, -1,  0,  0,  1 },
        { 0,  1, -1,  0,  0, -1,  1,  0 },
        { 0,  1,  1,  0,  0, -1, -1,  0 },
        { 1,  0,  0,  1, -1,  0,  0, -1 }
    };

    int octant;

    /* reset the entire fov to unseen */
    fov_reset(fov);

    /* set the center of the fov */
    fov->center = pos;

    /* determine which fields are visible */
    for (octant = 0; octant < 8; octant++)
    {
        fov_calculate_octant(fov, m, pos, infravision,
                             1, 1.0, 0.0, radius,
                             mult[0][octant], mult[1][octant],
                             mult[2][octant], mult[3][octant]);
    }

    fov_set(fov, pos, TRUE);
}

gboolean fov_get(fov *fov, position pos)
{
    assert (fov != NULL);
    assert (pos_valid(pos));

    return fov->data[Y(pos)][X(pos)];
}

void fov_set(fov *fov, position pos, guchar visible)
{
    assert (fov != NULL);
    assert (pos_valid(pos));

    fov->data[Y(pos)][X(pos)] = visible;
}

void fov_reset(fov *fov)
{
    assert (fov != NULL);

    /* set fov_data to FALSE */
    memset(fov->data, 0, MAP_MAX_Y * MAP_MAX_X * sizeof(guchar));

    /* set the center to an invalid position */
    fov->center = pos_invalid;

    /* clean list of visible monsters */
    g_hash_table_remove_all(fov->mlist);
}

monster *fov_get_closest_monster(fov *fov)
{
    monster *closest_monster = NULL;

    if (g_hash_table_size(fov->mlist) > 0)
    {
        GList *mlist;

        /* get the list of all visible monsters */
        mlist = g_hash_table_get_keys(fov->mlist);

        /* sort the monsters list by distance */
        mlist = g_list_sort_with_data(mlist, fov_visible_monster_sort,
                                      &fov->center);

        /* get the first element in the list */
        closest_monster = mlist->data;
    }

    return closest_monster;
}

GList *fov_get_visible_monsters(fov *fov)
{
    GList *mlist = NULL;

    if (g_hash_table_size(fov->mlist) != 0)
    {
        /* get a GList of all visible monster all */
        mlist = g_hash_table_get_keys(fov->mlist);

        /* sort the list of monster by distance */
        mlist = g_list_sort_with_data(mlist, fov_visible_monster_sort,
                                      &fov->center);
    }

    return mlist;
}

void fov_free(fov *fov)
{
    assert (fov != NULL);

    /* free the allocated memory */
    g_hash_table_destroy(fov->mlist);
    g_free(fov);
}

static void fov_calculate_octant(fov *fov, map *m, position center,
                                 int infravision, int row, float start,
                                 float end, int radius, int xx,
                                 int xy, int yx, int yy)
{
    int radius_squared;
    int j;
    int dx, dy;
    int X, Y;
    int blocked;
    float l_slope, r_slope;
    float new_start = 0;

    if (start < end)
        return;

    radius_squared = radius * radius;

    for (j = row; j <= radius + 1; j++)
    {
        dx = -j - 1;
        dy = -j;

        blocked = FALSE;

        while (dx <= 0)
        {
            dx += 1;

            /* Translate the dx, dy coordinates into map coordinates: */
            X = X(center) + dx * xx + dy * xy;
            Y = Y(center) + dx * yx + dy * yy;

            /* check if coordinated are within bounds */
            if ((X < 0) || (X >= MAP_MAX_X))
                continue;

            if ((Y < 0) || (Y >= MAP_MAX_Y))
                continue;

            /* l_slope and r_slope store the slopes of the left and right
             * extremities of the square we're considering: */
            l_slope = (dx - 0.5) / (dy + 0.5);
            r_slope = (dx + 0.5) / (dy - 0.5);

            if (start < r_slope)
            {
                continue;
            }
            else if (end > l_slope)
            {
                break;
            }
            else
            {
                position pos = { { X, Y, m->nlevel } };

                /* Our light beam is touching this square; light it */
                if ((dx * dx + dy * dy) < radius_squared)
                {
                    monster *mon;

                    fov_set(fov, pos, TRUE);

                    /* check if there is a monster at that position
                       must not be an unknown mimic or invisible */
                    if ((mon = map_get_monster_at(m, pos))
                        && !monster_unknown(mon)
                        && (!monster_flags(mon, MF_INVISIBLE) || infravision))
                    {
                        /* found a visible monster -> add it to the list */
                        g_hash_table_insert(fov->mlist, mon, 0);
                    }
                }

                if (blocked)
                {
                    /* we're scanning a row of blocked squares */
                    if (!map_pos_transparent(m, pos))
                    {
                        new_start = r_slope;
                        continue;
                    }
                    else
                    {
                        blocked = FALSE;
                        start = new_start;
                    }
                }
                else
                {
                    if (!map_pos_transparent(m, pos) && (j < radius))
                    {
                        /* This is a blocking square, start a child scan */
                        blocked = TRUE;
                    }

                    fov_calculate_octant(fov, m, center, infravision,
                                         j + 1, start, l_slope,
                                         radius, xx, xy, yx, yy);

                    new_start = r_slope;
                }
            }
        }

        /* Row is scanned; do next row unless last square was blocked */
        if (blocked)
        {
            break;
        }
    }
}

static gint fov_visible_monster_sort(gconstpointer a, gconstpointer b, gpointer center)
{
    monster *ma, *mb;

    ma = (monster*)a;
    mb = (monster*)b;

    int da = pos_distance(*(position *)center, monster_pos(ma));
    int db = pos_distance(*(position *)center, monster_pos(mb));

    if (da < db)
        return -1;

    if (da > db)
        return 1;

    return 0;
}
