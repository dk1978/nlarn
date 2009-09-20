/*
 * position.c
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

#include "level.h"
#include "position.h"

position pos_new(int x, int y)
{
    position pos;

    assert((x >= 0 && x <= LEVEL_MAX_X) || x == G_MAXINT16);
    assert((y >= 0 && y <= LEVEL_MAX_Y) || y == G_MAXINT16);

    pos.x = x;
    pos.y = y;

    return pos;
}

position pos_move(position pos, direction dir)
{
    /* return given position if direction is not implemented */
    position npos;

    assert(dir > GD_NONE && dir < GD_MAX);

    switch (dir)
    {
    case GD_WEST:
        if (pos.x > 0)
            npos = pos_new(pos.x - 1, pos.y);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    case GD_NW:
        if ((pos.x > 0) && (pos.y > 0))
            npos = pos_new(pos.x - 1, pos.y - 1);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    case GD_NORTH:
        if (pos.y > 0)
            npos = pos_new(pos.x, pos.y - 1);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    case GD_NE:
        if ((pos.x < LEVEL_MAX_X - 1) && (pos.y > 0))
            npos = pos_new(pos.x + 1, pos.y - 1);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    case GD_EAST:
        if (pos.x < LEVEL_MAX_X - 1)
            npos = pos_new(pos.x + 1, pos.y);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    case GD_SE:
        if ((pos.x < LEVEL_MAX_X - 1) && (pos.y < LEVEL_MAX_Y - 1))
            npos = pos_new(pos.x + 1, pos.y + 1);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    case GD_SOUTH:
        if (pos.y < LEVEL_MAX_Y - 1)
            npos = pos_new(pos.x, pos.y + 1);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    case GD_SW:
        if ((pos.x > 0) && (pos.y < LEVEL_MAX_Y - 1))
            npos = pos_new(pos.x - 1, pos.y + 1);
        else
            npos = pos_new(G_MAXINT16, G_MAXINT16);

        break;

    default:
        npos = pos;

    }

    return npos;
}

/**
 * Create a new rectangle of given dimensions. Do a sanity check and
 * replace out-of-bounds values.
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 *
 * @return corrected area
 *
 */
rectangle rect_new(int x1, int y1, int x2, int y2)
{
    rectangle rect;

    rect.x1 = (x1 < 0) ? 0 : x1;
    rect.y1 = (y1 < 0) ? 0 : y1;
    rect.x2 = (x2 > LEVEL_MAX_X) ? LEVEL_MAX_X : x2;
    rect.y2 = (y2 > LEVEL_MAX_Y) ? LEVEL_MAX_Y : y2;

    return(rect);
}

rectangle rect_new_sized(position center, int size)
{
    return rect_new(center.x - size,
                    center.y - size,
                    center.x + size,
                    center.y + size);
}

int pos_in_rect(position pos, rectangle rect)
{
    if ((pos.x >= rect.x1)
            && (pos.x <= rect.x2)
            && (pos.y >= rect.y1)
            && (pos.y <= rect.y2))
        return TRUE;
    else
        return FALSE;
}

area *area_new(int start_x, int start_y, int size_x, int size_y)
{
    area *area;
    int y;

    area = g_malloc0(sizeof(area));

    area->start_x = start_x;
    area->start_y = start_y;

    area->size_x = size_x;
    area->size_y = size_y;

    area->area = g_malloc0(size_y * sizeof(int *));

    for (y = 0; y < size_y; y++)
        area->area[y] = g_malloc0(size_x * sizeof(int));

    return area;
}

/**
 * Draw a circle: Midpoint circle algorithm
 * from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 *
 * @param center point of the circle
 * @param radius of the circle
 * @return a new area.
 */
area *area_new_circle(position center, int radius)
{
    area *area;

    int f = 1 - radius;
    int ddF_x = 1;
    int ddF_y = -2 * radius;
    int x = 0;
    int y = radius;

    int fill;

    if (!pos_valid(center))
    {
        return NULL;
    }

    area = area_new(center.x - radius,
                    center.y - radius,
                    2 * radius + 1,
                    2 * radius + 1);

    /* reposition center to relative values */
    center.x = radius;
    center.y = radius;

    area_point_set(area, center.x, center.y + radius);
    area_point_set(area, center.x, center.y - radius);
    area_point_set(area, center.x + radius, center.y);
    area_point_set(area, center.x - radius, center.y);

    while (x < y)
    {
        assert(ddF_x == 2 * x + 1);
        assert(ddF_y == -2 * y);
        assert(f == x * x + y * y - radius * radius + 2 * x - y + 1);

        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        area_point_set(area, center.x + x, center.y + y);
        area_point_set(area, center.x - x, center.y + y);
        area_point_set(area, center.x + x, center.y - y);
        area_point_set(area, center.x - x, center.y - y);
        area_point_set(area, center.x + y, center.y + x);
        area_point_set(area, center.x - y, center.y + x);
        area_point_set(area, center.x + y, center.y - x);
        area_point_set(area, center.x - y, center.y - x);
    }

    /* fill the circle
     * - set fill to 1 when spotting the left border
     * - set position if (fill == 1)
     * - set fill = 2 when spotting the right border
     *
     * do not need to fill the first and last row
     */

    for (y = 1; y < area->size_y - 1; y++)
    {
        fill = 0;

        for (x = 0; x < area->size_x; x++)
        {
            /* there are double dots at the beginning and the end of the square */
            if (area_point_get(area, x, y) && (!area_point_get(area, x + 1, y)))
            {
                fill = !fill;
                continue;
            }

            if (fill)
            {
                area_point_set(area, x, y);
            }
        }
    }

    return area;
}

area *area_new_circle_flooded(position center, int radius, area *obstacles)
{
    area *narea, *circle;
    int x, y;

    assert(obstacles != NULL);

    if (!pos_valid(center))
    {
        return NULL;
    }

    narea = area_new(center.x - radius,
                     center.y - radius,
                     2 * radius + 1,
                     2 * radius + 1);

    circle = area_new_circle(center, radius);

    /* fill narea */
    for (y = 0; y < narea->size_y; y++)
    {
        for (x = 0; x < narea->size_x; x++)
        {
            if (area_point_get(circle, x, y) && !area_point_get(obstacles, x, y))
            {
                area_point_set(narea, x, y);
            }
        }
    }

    area_destroy(circle);
    area_destroy(obstacles);

    return narea;
}

area *area_new_ray(position source, position target, area *obstacles)
{
    area *narea;
    int delta_x, delta_y;
    int offset_x, offset_y;
    int x, y;
    signed int ix, iy;
    int error;

    narea = area_new(min(source.x, target.x),
                     min(source.y, target.y),
                     abs(target.x - source.x) + 1,
                     abs(target.y - source.y) + 1);

    /* offset = offset to level map */
    offset_x = narea->start_x;
    offset_y = narea->start_y;

    /* reposition source and target to get a position inside narea */
    source.x -= offset_x;
    source.y -= offset_y;
    target.x -= offset_x;
    target.y -= offset_y;

    /* offset = relative offset for obstacle lookup */
    offset_x -= obstacles->start_x;
    offset_y -= obstacles->start_y;

    x = source.x;
    y = source.y;

    delta_x = abs(target.x - source.x) << 1;
    delta_y = abs(target.y - source.y) << 1;

    /* if x1 == x2 or y1 == y2, then it does not matter what we set here */
    ix = target.x > source.x ? 1 : -1;
    iy = target.y > source.y ? 1 : -1;

    if (delta_x >= delta_y)
    {
        /* error may go below zero */
        error = delta_y - (delta_x >> 1);

        while (x != target.x)
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

            if (area_point_get(obstacles, x + offset_x, y + offset_y))
            {
                /* stop painting ray */
                break;
            }
            else
            {
                area_point_set(narea, x, y);
            }
        }
    }
    else
    {
        /* error may go below zero */
        int error = delta_x - (delta_y >> 1);

        while (y != target.y)
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

            if (area_point_get(obstacles, x + offset_x, y + offset_y))
            {
                /* stop painting ray */
                break;
            }
            else
            {
                area_point_set(narea, x, y);
            }
        }
    }

    area_destroy(obstacles);

    return narea;
}

void area_destroy(area *area)
{
    int y;

    assert(area != NULL);

    for (y = 0; y < area->size_y; y++)
        g_free(area->area[y]);

    g_free(area->area);

    g_free(area);
}
