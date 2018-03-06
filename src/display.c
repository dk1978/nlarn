/*
 * display.c
 * Copyright (C) 2009-2012, 2014 Joachim de Groot <jdegroot@web.de>
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

#include <ctype.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#ifdef SDLPDCURSES
#  include <SDL.h>
void PDC_update_rects(void);
#endif

#include "display.h"
#include "fov.h"
#include "map.h"
#include "nlarn.h"
#include "spheres.h"

typedef struct _display_colset
{
    const char *name;
    const int  val;
} display_colset;

const display_colset display_default_colset[] =
{
    { "",             DC_NONE },
    { "black",        DC_BLACK },
    { "red",          DC_RED },
    { "green",        DC_GREEN },
    { "brown",        DC_BROWN },
    { "blue",         DC_BLUE },
    { "magenta",      DC_MAGENTA },
    { "cyan",         DC_CYAN },
    { "lightgray",    DC_LIGHTGRAY },
    { "darkgrey",     DC_DARKGRAY },
    { "lightred",     DC_LIGHTRED },
    { "lightgreen",   DC_LIGHTGREEN },
    { "yellow",       DC_YELLOW },
    { "lightblue",    DC_LIGHTBLUE },
    { "lightmagenta", DC_LIGHTMAGENTA },
    { "lightcyan",    DC_LIGHTCYAN },
    { "white",        DC_WHITE },
    { NULL,           0 },
};

const display_colset display_dialog_colset[] =
{
    { "",             DDC_NONE },
    { "black",        DDC_BLACK },
    { "red",          DDC_RED },
    { "green",        DDC_GREEN },
    { "brown",        DDC_BROWN },
    { "blue",         DDC_BLUE },
    { "magenta",      DDC_MAGENTA },
    { "cyan",         DDC_CYAN },
    { "lightgray",    DDC_LIGHTGRAY },
    { "darkgrey",     DDC_DARKGRAY },
    { "lightred",     DDC_LIGHTRED },
    { "lightgreen",   DDC_LIGHTGREEN },
    { "yellow",       DDC_YELLOW },
    { "lightblue",    DDC_LIGHTBLUE },
    { "lightmagenta", DDC_LIGHTMAGENTA },
    { "lightcyan",    DDC_LIGHTCYAN },
    { "white",        DDC_WHITE },
    { NULL,           0 },
};

static gboolean display_initialised = FALSE;

/* linked list of opened windows */
static GList *windows = NULL;

static int mvwcprintw(WINDOW *win, int defattr, const display_colset *colset,
                      int y, int x, const char *fmt, ...);

static int display_get_colval(const display_colset *colset, const char *name);

static void display_inventory_help(GPtrArray *callbacks);

static display_window *display_window_new(int x1, int y1, int width,
        int height, const char *title);

static int display_window_move(display_window *dwin, int key);
static void display_window_update_title(display_window *dwin, const char *title);
static void display_window_update_caption(display_window *dwin, char *caption);
static void display_window_update_arrow_up(display_window *dwin, gboolean on);
static void display_window_update_arrow_down(display_window *dwin, gboolean on);

static display_window *display_item_details(guint x1, guint y1, guint width,
                                            item *it, player *p, gboolean shop);

static void display_spheres_paint(sphere *s, player *p);

void display_init()
{
    /* taken from Angband 3.1.1 */
    /* overwrite bit is false, so users on real (serial) terminals can override
       this; terminal emulators drop an entire sequence into the input buffer at
       once, and I/O-buffering makes the packet very unlikely to be split even
       over networks */
#ifndef WIN32
    g_setenv("ESCDELAY", "0", 0);
#endif

#ifdef SDLPDCURSES
    /* default to 90 columns when using SDL PDCurses */
    g_setenv("PDC_COLS", "90", 0);
    g_setenv("PDC_LINES", "25", 0);
#endif

    /* Start curses mode */
    initscr();

#ifdef SDLPDCURSES
    /* These initialisations have to be done after initscr(), otherwise
       the window is not yet available. */
    /* Set the window title */
    char *window_title = g_strdup_printf("NLarn %d.%d.%d%s", VERSION_MAJOR,
            VERSION_MINOR, VERSION_PATCH, GITREV);

    PDC_set_title(window_title);
    g_free(window_title);

    /* Set the window icon */
    char *icon_name = g_strdup_printf("%s/nlarn-128.bmp", nlarn->libdir);
    SDL_Surface *image = SDL_LoadBMP(icon_name);
    g_free(icon_name);

    Uint32 colorkey = SDL_MapRGB(image->format, 0, 0, 0);
    SDL_SetColorKey(image, SDL_SRCCOLORKEY, colorkey);
    SDL_WM_SetIcon(image,NULL);
#endif

    /* initialize colours */
    start_color();

    /* black background */
    init_pair(DCP_WHITE_BLACK,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(DCP_RED_BLACK,     COLOR_RED,     COLOR_BLACK);
    init_pair(DCP_GREEN_BLACK,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(DCP_BLUE_BLACK,    COLOR_BLUE,    COLOR_BLACK);
    init_pair(DCP_YELLOW_BLACK,  COLOR_YELLOW,  COLOR_BLACK);
    init_pair(DCP_MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(DCP_CYAN_BLACK,    COLOR_CYAN,    COLOR_BLACK);
    init_pair(DCP_BLACK_BLACK,   COLOR_BLACK,   COLOR_BLACK);

    /* these colour pairs are used by dialogs */
    init_pair(DCP_WHITE_RED,    COLOR_WHITE,    COLOR_RED);
    init_pair(DCP_RED_RED,      COLOR_RED,      COLOR_RED);
    init_pair(DCP_GREEN_RED,    COLOR_GREEN,    COLOR_RED);
    init_pair(DCP_BLUE_RED,     COLOR_BLUE,     COLOR_RED);
    init_pair(DCP_YELLOW_RED,   COLOR_YELLOW,   COLOR_RED);
    init_pair(DCP_MAGENTA_RED,  COLOR_MAGENTA,  COLOR_RED);
    init_pair(DCP_CYAN_RED,     COLOR_CYAN,     COLOR_RED);
    init_pair(DCP_BLACK_RED,    COLOR_BLACK,    COLOR_RED);
    init_pair(DCP_BLACK_WHITE,  COLOR_BLACK,    COLOR_WHITE);
    init_pair(DCP_RED_WHITE,    COLOR_RED,      COLOR_WHITE);

    /* control special keys in application */
    raw();

    /* suppress input echo */
    noecho();

    /* enable function keys */
    keypad(stdscr, TRUE);

    /* want all 8 bits */
    meta(stdscr, TRUE);

    /* make cursor invisible */
    curs_set(0);

    /* update display initialisation status */
    display_initialised = TRUE;
}

static int attr_colour(int colour, int reverse)
{
    if (reverse)
        return (A_REVERSE | colour);

    return colour;
}

int display_paint_screen(player *p)
{
    guint x, y, i;
    position pos = pos_invalid;
    map *vmap;
    int attrs;              /* curses attributes */
    message_log_entry *le;  /* needed to display messages */
    GPtrArray *text = NULL; /* storage for formatted messages */
    guint *ttime = NULL;    /* storage for the game time of messages */

    /* draw line around map */
    (void)mvhline(MAP_MAX_Y, 0, ACS_HLINE, MAP_MAX_X);
    (void)mvvline(0, MAP_MAX_X, ACS_VLINE, MAP_MAX_Y);
    (void)mvaddch(MAP_MAX_Y, MAP_MAX_X, ACS_LRCORNER);

    /* make shortcut to the visible map */
    vmap = game_map(nlarn, Z(p->pos));

    /* draw map */
    Z(pos) = Z(p->pos);
    for (Y(pos) = 0; Y(pos) < MAP_MAX_Y; Y(pos)++)
    {
        /* position cursor */
        move(Y(pos), 0);

        for (X(pos) = 0; X(pos) < MAP_MAX_X; X(pos)++)
        {
            if (game_fullvis(nlarn) || fov_get(p->fv, pos))
            {
                /* draw the truth */
                inventory **inv = map_ilist_at(vmap, pos);
                const gboolean has_items = inv_length(*inv) > 0;

                if (map_sobject_at(vmap, pos))
                {
                    /* draw stationary objects first */
                    attron(attrs = attr_colour(so_get_colour(map_sobject_at(vmap, pos)), has_items));
                    if (map_sobject_at(vmap, pos) == LS_CLOSEDDOOR || map_sobject_at(vmap, pos) == LS_OPENDOOR)
                        addch(map_get_door_glyph(vmap, pos));
                    else
                        addch(so_get_glyph(map_sobject_at(vmap, pos)));
                    attroff(attrs);
                }
                else if (has_items)
                {
                    /* draw items */
                    item *it;

                    /* memorize the most interesting item on the tile */
                    if (inv_length_filtered(*inv, item_filter_gems) > 0)
                    {
                        /* there's a gem in the stack */
                        it = inv_get_filtered(*inv, 0, item_filter_gems);
                    }
                    else if (inv_length_filtered(*inv, item_filter_gold) > 0)
                    {
                        /* there is gold in the stack */
                        it = inv_get_filtered(*inv, 0, item_filter_gold);
                    }
                    else
                    {
                        /* memorize the topmost item on the stack */
                        it = inv_get(*inv, inv_length(*inv) - 1);
                    }

                    const gboolean has_trap = (map_trap_at(vmap, pos)
                                               && player_memory_of(p, pos).trap);

                    attron(attrs = attr_colour(item_colour(it), has_trap));
                    addch(item_glyph(it->type));
                    attroff(attrs);
                }
                else if (map_trap_at(vmap, pos) && (game_fullvis(nlarn) || player_memory_of(p, pos).trap))
                {
                    /* FIXME - displays trap when unknown!! */
                    attron(attrs = trap_colour(map_trap_at(vmap, pos)));
                    addch('^');
                    attroff(attrs);
                }
                else
                {
                    /* draw tile */
                    attron(attrs = mt_get_colour(map_tiletype_at(vmap, pos)));
                    addch(mt_get_glyph(map_tiletype_at(vmap, pos)));
                    attroff(attrs);
                }
            }
            else /* i.e. !fullvis && !visible: draw players memory */
            {
                const gboolean has_items = player_memory_of(p, pos).item;
                if (player_memory_of(p, pos).sobject)
                {
                    /* draw stationary object */
                    sobject_t ms = map_sobject_at(vmap, pos);

                    attron(attrs = attr_colour(so_get_colour(ms), has_items));

                    if (ms == LS_CLOSEDDOOR || ms == LS_OPENDOOR)
                        addch(map_get_door_glyph(vmap, pos));
                    else
                        addch(so_get_glyph(ms));

                    attroff(attrs);
                }
                else if (has_items)
                {
                    /* draw items */
                    const gboolean has_trap = (player_memory_of(p, pos).trap);

                    attron(attrs = attr_colour(player_memory_of(p, pos).item_colour,
                                               has_trap));
                    addch(item_glyph(player_memory_of(p, pos).item));
                    attroff(attrs);
                }
                else if (player_memory_of(p, pos).trap)
                {
                    /* draw trap */
                    attron(attrs = trap_colour(map_trap_at(vmap, pos)));
                    addch('^');
                    attroff(attrs);
                }
                else
                {
                    /* draw tile */
                    attron(attrs = DC_DARKGRAY);
                    addch(mt_get_glyph(player_memory_of(p, pos).type));
                    attroff(attrs);
                }
            }

            /* draw monsters */
            monster *monst = map_get_monster_at(vmap, pos);

            if (monst == NULL)
            {
                /* no monster found */
                continue;
            }

            if (game_fullvis(nlarn)
                    || player_effect(p, ET_DETECT_MONSTER)
                    || monster_in_sight(monst))
            {
                attron(attrs = monster_color(monst));
                position mpos = monster_pos(monst);
                (void)mvaddch(Y(mpos), X(mpos), monster_glyph(monst));
                attroff(attrs);
            }
        }
    }

    /* draw spheres */
    g_ptr_array_foreach(nlarn->spheres, (GFunc)display_spheres_paint, p);

    /* draw player */
    char pc;
    if (player_effect(p, ET_INVISIBILITY))
    {
        pc = ' ';
        attrs = A_REVERSE | DC_WHITE;
    }
    else
    {
        pc = '@';
        attrs = DC_WHITE;
    }

    attron(attrs);
    (void)mvaddch(Y(p->pos), X(p->pos), pc);
    attroff(attrs);


    /* *** status line *** */
    move(MAP_MAX_Y + 1, 0);
    clrtoeol();

    /* player name and level */
    if (p->name)
    {
        /* the player's name can be NULL directly after starting the game */
        printw("%s, %s", p->name, player_get_level_desc(p));
    }

    /* current HPs */
    if (p->hp <= ((int)p->hp_max / 10)) /* 10% hp left */
        attrs = DC_LIGHTRED;
    else if (p->hp <= ((int)p->hp_max / 4))  /* 25% hp left */
        attrs = DC_RED;
    else if (p->hp <= ((int)p->hp_max / 2))  /* 50% hp left */
        attrs = DC_GREEN;
    else
        attrs = DC_LIGHTGREEN;

    attron(attrs);
    mvprintw(MAP_MAX_Y + 1, MAP_MAX_X - 21, "HP %3d", p->hp, player_get_hp_max(p));
    attroff(attrs);

    /* max HPs */
    attron(attrs = DC_LIGHTGREEN);
    mvprintw(MAP_MAX_Y + 1, MAP_MAX_X - 15, "/%-3d", player_get_hp_max(p));
    attroff(attrs);

    /* current MPs */
    if (p->mp <= ((int)p->mp_max / 10)) /* 10% mp left */
        attrs = DC_LIGHTMAGENTA;
    else if (p->mp <= ((int)p->mp_max / 4))  /* 25% mp left */
        attrs = DC_MAGENTA;
    else if (p->mp <= ((int)p->mp_max / 2))  /* 50% mp left */
        attrs = DC_CYAN;
    else
        attrs = DC_LIGHTCYAN;

    attron(attrs);
    mvprintw(MAP_MAX_Y + 1, MAP_MAX_X - 10, "MP %3d", p->mp);
    attroff(attrs);

    /* max MPs */
    attron(attrs = DC_LIGHTCYAN);
    mvprintw(MAP_MAX_Y + 1, MAP_MAX_X - 4, "/%-3d", player_get_mp_max(p));
    attroff(attrs);

    /* game time */
    mvprintw(MAP_MAX_Y + 1, MAP_MAX_X + 1, "T %-6d", game_turn(nlarn));

    /* experience points / level */
    move(MAP_MAX_Y + 2, 0);
    clrtoeol();

    attron(attrs = DC_LIGHTBLUE);

    mvprintw(MAP_MAX_Y + 2, MAP_MAX_X - 21, "XP %3d/%-5d",
             p->level, p->experience);

    attroff(attrs);

    /* dungeon map */
    mvprintw(MAP_MAX_Y + 2, MAP_MAX_X + 1, "Lvl: %s", map_name(vmap));


    /* *** RIGHT STATUS *** */

    /* strength */
    mvprintw(1, MAP_MAX_X + 3, "STR ");

    if (player_get_str(p) > (int)p->strength)
        attrs = DC_YELLOW;
    else if (player_get_str(p) < (int)p->strength)
        attrs = DC_LIGHTRED;
    else
        attrs = DC_WHITE;

    attron(attrs);
    printw("%2d", player_get_str(p));
    attroff(attrs);
    clrtoeol();

    /* dexterity */
    mvprintw(2, MAP_MAX_X + 3, "DEX ");

    if (player_get_dex(p) > (int)p->dexterity)
        attrs = DC_YELLOW;
    else if (player_get_dex(p) < (int)p->dexterity)
        attrs = DC_LIGHTRED;
    else
        attrs = DC_WHITE;

    attron(attrs);
    printw("%2d", player_get_dex(p));
    attroff(attrs);
    clrtoeol();

    /* constitution */
    mvprintw(3, MAP_MAX_X + 3, "CON ");

    if (player_get_con(p) > (int)p->constitution)
        attrs = DC_YELLOW;
    else if (player_get_con(p) < (int)p->constitution)
        attrs = DC_LIGHTRED;
    else
        attrs = DC_WHITE;

    attron(attrs);
    printw("%2d", player_get_con(p));
    attroff(attrs);
    clrtoeol();

    /* intelligence */
    mvprintw(4, MAP_MAX_X + 3, "INT ");

    if (player_get_int(p) > (int)p->intelligence)
        attrs = DC_YELLOW;
    else if (player_get_int(p) < (int)p->intelligence)
        attrs = DC_LIGHTRED;
    else
        attrs = DC_WHITE;

    attron(attrs);
    printw("%2d", player_get_int(p));
    attroff(attrs);
    clrtoeol();

    /* wisdom */
    mvprintw(5, MAP_MAX_X + 3, "WIS ");

    if (player_get_wis(p) > (int)p->wisdom)
        attrs = DC_YELLOW;
    else if (player_get_wis(p) < (int)p->wisdom)
        attrs = DC_LIGHTRED;
    else
        attrs = DC_WHITE;

    attron(attrs);
    printw("%2d", player_get_wis(p));
    attroff(attrs);
    clrtoeol();

    /* clear line below Wisdom */
    move(6, MAP_MAX_X + 1);
    clrtoeol();

    /* wielded weapon */
    if (p->eq_weapon)
    {
        char *wdesc = weapon_shortdesc(p->eq_weapon, COLS - MAP_MAX_X - 4);
        mvprintw(7, MAP_MAX_X + 3, "%s", wdesc);
        g_free(wdesc);
    }
    else
    {
        attron(attrs = DC_LIGHTRED);
        mvprintw(7, MAP_MAX_X + 3, "unarmed");
        attroff(attrs);
    }
    clrtoeol();

    /* armour class */
    mvprintw(8, MAP_MAX_X + 3, "AC: %2d", player_get_ac(p));
    clrtoeol();

    /* gold */
    mvprintw(9, MAP_MAX_X + 3, "$%-7d", player_get_gold(p));
    clrtoeol();

    /* clear line below gold */
    move(10, MAP_MAX_X + 1);
    clrtoeol();

    /* clear lines */
    for (int i = 0; i < 7; i++)
    {
        move(11 + i, MAP_MAX_X + 3);
        clrtoeol();
    }

    /* display effect descriptions */
    if (p->effects->len > 0)
    {
        const guint available_space = COLS - MAP_MAX_X - 4;
        char **efdescs = strv_new();

        /* collect effect descriptions */
        for (i = 0; i < p->effects->len; i++)
        {
            effect *e = game_effect_get(nlarn, g_ptr_array_index(p->effects, i));

            if (effect_get_desc(e) == NULL)
                continue;

            char *desc = g_strdup(effect_get_desc(e));

            if (strlen(desc) > available_space)
            {
                desc[available_space - 1] = '.';
                desc[available_space] = '\0';
            }

            if ((e->type == ET_WALL_WALK || e->type == ET_LEVITATION)
                    && e->turns < 6)
            {
                /* fading effects */
                gchar *cdesc = g_strdup_printf("`lightred`%s`end`", desc);
                strv_append_unique(&efdescs, cdesc);
                g_free(cdesc);
            }
            else if (e->type > ET_LEVITATION)
            {
                /* negative effects */
                gchar *cdesc = g_strdup_printf("`lightmagenta`%s`end`", desc);
                strv_append_unique(&efdescs, cdesc);
                g_free(cdesc);
            }
            else
            {
                /* other effects */
                strv_append_unique(&efdescs, desc);
            }

            g_free(desc);
        }

        /* display effect descriptions */
        for (i = 0; i < g_strv_length(efdescs); i++)
        {
            mvwcprintw(stdscr, DC_LIGHTCYAN, display_default_colset, 11 + i,
                       MAP_MAX_X + 3, efdescs[i]);

        }

        g_strfreev(efdescs);
    }

    /* *** MESSAGES *** */
    /* number of lines which can be displayed */
    y = LINES - 20;

    /* storage for game time of message */
    ttime = g_new0(guint, y);

    /* hold original length of text */
    x = 1;

    /* line counter */
    i = 0;

    /* if log contains buffered messaged, display them */
    if (log_buffer(nlarn->log))
    {
        text = text_wrap(log_buffer(nlarn->log), COLS, 2);
        for (x = 1; x <= (unsigned)min(text->len, y); x++)
            ttime[x - 1] = game_turn(nlarn);
    }

    /* retrieve game log and reformat messages to window width */
    while (((text == NULL) || (text->len < y)) && (log_length(nlarn->log) > i))
    {
        le = log_get_entry(nlarn->log, log_length(nlarn->log) - 1 - i);

        if (text == NULL)
            text = text_wrap(le->message, COLS, 2);
        else
            text = text_append(text, text_wrap(le->message, COLS, 2));

        /* store game time for associated text line */
        while ((x <= text->len) && (x <= y))
        {
            ttime[x - 1] = le->gtime;
            x++;
        }

        i++;
    }

    /* clear the message area */
    move(20, 0);
    clrtobot();

    for (y = 20, i = 0; (y < (unsigned)LINES) && (i < text->len); i++, y++)
    {
        /* default color for the line */
        int def_attrs;

        if ((nlarn->log->gtime - 15) < ttime[i])
            def_attrs = DC_WHITE;
        else
            def_attrs = DC_LIGHTGRAY;

        mvwcprintw(stdscr, def_attrs, display_default_colset,
                   y, 0, g_ptr_array_index(text, i));
    }

    text_destroy(text);
    g_free(ttime);

    return display_draw();
}

void display_shutdown()
{
    /* only terminate curses mode when the display has been initialised */
    if (display_initialised)
    {
        /* end curses mode */
        endwin();

        /* update display initialisation status */
        display_initialised = FALSE;
    }
}

gboolean display_available()
{
    return display_initialised;
}

int display_draw()
{
    /* mark stdscr and all panels for redraw */
    update_panels();

    /* finally commit all the prepared updates */
#ifdef SDLPDCURSES
    /* SDL PDCurses does a little trick to increase the performance,
       have a look at the docs to see why this is required. */
    int ret = doupdate();
    PDC_update_rects();
    return ret;
#else
    return doupdate();
#endif
}

static int item_sort_normal(gconstpointer a, gconstpointer b, gpointer data)
{
    return item_sort(a, b, data, FALSE);
}

static int item_sort_shop(gconstpointer a, gconstpointer b, gpointer data)
{
    return item_sort(a, b, data, TRUE);
}

item *display_inventory(const char *title, player *p, inventory **inv,
                        GPtrArray *callbacks, gboolean show_price,
                        gboolean show_weight, gboolean show_account,
                        int (*ifilter)(item *))
{
    /* the inventory window */
    display_window *iwin = NULL;
    /* the item description popup */
    display_window *ipop = NULL;

    /* the dialog width */
    const guint width = COLS - 4;

    guint len_orig, len_curr;
    gboolean redraw = FALSE;

    /* the windows title used for shops */
    char *stitle = NULL;

    gboolean keep_running = TRUE;
    int key;

    /* string array used to assemble the window caption
       from the callback descriptions */
    char **captions;

    /* offset to element position (when displaying more than maxvis items) */
    guint offset = 0;

    /* position of currently selected item */
    guint curr = 1;

    item *it;

    /* curses attributes */
    int attrs;

    g_assert(p != NULL && inv != NULL);

    /* sort inventory by item type */
    if (show_price)
        inv_sort(*inv, (GCompareDataFunc)item_sort_shop, (gpointer)p);
    else
        inv_sort(*inv, (GCompareDataFunc)item_sort_normal, (gpointer)p);

    /* store inventory length */
    len_orig = len_curr = inv_length_filtered(*inv, ifilter);

    /* main loop */
    do
    {
        /* calculate the dialog height */
        guint height = min((LINES - 10), len_curr + 2);

        /* calculate how many items can be displayed at a time */
        guint maxvis = min(len_curr, height - 2);

        /* fix selected item */
        if (curr > len_curr)
            curr = len_curr;

        /* rebuild screen if needed */
        if (iwin != NULL && redraw)
        {
            display_window_destroy(iwin);
            iwin = NULL;

            display_paint_screen(p);
            redraw = FALSE;

            /* check for inventory modifications */
            if (len_orig > len_curr)
            {
                /* inventory length is smaller than before */
                /* if on the last page, reduce offset */
                if ((offset > 0) && ((offset + maxvis) > len_curr))
                    offset--;

                /* remember current length */
                len_orig = len_curr;
            }
            else if (len_curr > len_orig)
            {
                /* inventory has grown - sort inventory again */
                if (show_price)
                    inv_sort(*inv, (GCompareDataFunc)item_sort_shop, (gpointer)p);
                else
                    inv_sort(*inv, (GCompareDataFunc)item_sort_normal, (gpointer)p);
            }
        }

        if (!iwin)
        {
            iwin = display_window_new(2, 2, width, height, title);
        }

        /* draw all items */
        for (guint pos = 1; pos <= (unsigned)min(len_curr, maxvis); pos++)
        {
            it = inv_get_filtered(*inv, (pos - 1) + offset, ifilter);

            gboolean item_equipped = FALSE;

            if (!show_price)
            {
                /* shop items are definitely not equipped */
                item_equipped = player_item_is_equipped(p, it);
            }

            /* currently selected */
            if (curr == pos)
            {
                if (item_equipped)
                    attrs = COLOR_PAIR(DCP_BLACK_WHITE);
                else
                    attrs = COLOR_PAIR(DCP_RED_WHITE);
            }
            else if (item_equipped)
                attrs = COLOR_PAIR(DCP_WHITE_RED) | A_BOLD;
            else
                attrs = COLOR_PAIR(DCP_WHITE_RED);

            wattron(iwin->window, attrs);

            if (show_price)
            {
                /* inside shop */
                gchar *item_desc = item_describe(it, TRUE, FALSE, FALSE);
                mvwprintw(iwin->window, pos, 1, " %-*s %5d$ ",
                          width - 11, item_desc, item_price(it));

                g_free(item_desc);
            }
            else
            {
                gchar *item_desc = item_describe(it, player_item_known(p, it), FALSE, FALSE);
                mvwprintw(iwin->window, pos, 1, " %-*s %c ",
                          width - 6, item_desc,
                          player_item_is_equipped(p, it) ? '*' : ' ');

                g_free(item_desc);
            }

            wattroff(iwin->window, attrs);
        }

        /* prepare the window title */
        if (show_account)
        {
            /* show the balance of the bank account */
            stitle = g_strdup_printf("%s - %d gp on bank account",
                                     title, p->bank_account);

            display_window_update_title(iwin, stitle);
            g_free(stitle);
        }
        else if (show_weight)
        {
            /* show the weight of the inventory */
            stitle = g_strdup_printf("%s - %s of %s carried",
                                     title, player_inv_weight(p),
                                     player_can_carry(p));

            display_window_update_title(iwin, stitle);
            g_free(stitle);
        }

        /* get the currently selected item */
        it = inv_get_filtered(*inv, curr + offset - 1, ifilter);

        /* prepare the string array which will hold all the captions */
        captions = strv_new();

        /* assemble window caption (if callbacks have been defined) */
        for (guint cb_nr = 0; callbacks != NULL && cb_nr < callbacks->len; cb_nr++)
        {
            display_inv_callback *cb = g_ptr_array_index(callbacks, cb_nr);

            /* check if callback is appropriate for this item */
            /* if no check function is set, always display item */
            if ((cb->checkfun == NULL) || cb->checkfun(p, it))
            {
                cb->active = TRUE;
                strv_append(&captions, cb->description);
            }
            else
            {
                /* it isn't */
                cb->active = FALSE;
            }
        }

        /* refresh the item description popup */
        if (ipop != NULL)
            display_window_destroy(ipop);

        ipop = display_item_details(iwin->x1, iwin->y1 + iwin->height,
                                    iwin->width, it, p, show_price);

        if (g_strv_length(captions) > 0)
        {
            /* append "(?) help" to trigger the help popup */
            strv_append(&captions, "(?) help");

            /* update the window's caption with the assembled array of captions */
            display_window_update_caption(iwin, g_strjoinv(" ", captions));
        }
        else
        {
            /* reset the window caption */
            display_window_update_caption(iwin, NULL);
        }

        /* free the array of caption strings */
        g_strfreev(captions);

        display_window_update_arrow_up(iwin, offset > 0);
        display_window_update_arrow_down(iwin, (offset + maxvis) < len_curr);

        wrefresh(iwin->window);

        switch (key = getch())
        {

        case '7':
        case KEY_HOME:
        case KEY_A1:

            curr = 1;
            offset = 0;

            break;

        case '9':
        case KEY_PPAGE:
        case KEY_A3:
        case 21: /* ^U */

            if ((curr == maxvis) || offset == 0)
                curr = 1;
            else
                offset = (offset > maxvis) ? (offset - maxvis) : 0;

            break;

        case 'k':
        case '8':
        case KEY_UP:
#ifdef KEY_A2
        case KEY_A2:
#endif

            if (curr > 1)
                curr--;

            else if ((curr == 1) && (offset > 0))
                offset--;

            break;

        case 'j':
        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            if ((curr + offset) < len_curr)
            {
                if (curr == maxvis)
                    offset++;
                else
                    curr++;
            }

            break;

        case '3':
        case KEY_NPAGE:
        case KEY_C3:
        case 4: /* ^D */

            if (curr == 1)
            {
                curr = maxvis;
            }
            else
            {
                offset = offset + maxvis;

                if ((offset + maxvis) >= len_curr)
                {
                    curr = min(len_curr, maxvis);
                    offset = len_curr - curr;
                }
            }
            break;

        case '1':
        case KEY_END:
        case KEY_C1:

            if (len_curr > maxvis)
            {
                curr = maxvis;
                offset = len_curr - maxvis;
            }
            else
            {
                curr = len_curr;
            }
            break;

        case KEY_ESC:
            keep_running = FALSE;
            break;

        case '?':
            display_inventory_help(callbacks);
            break;

        case KEY_LF:
        case KEY_CR:
#ifdef PADENTER
        case PADENTER:
#endif
        case KEY_ENTER:
            if (callbacks == NULL)
            {
                /* if no callbacks have been defined, enter selects item */
                keep_running = FALSE;
            }
            break;

        default:
            /* check callback function keys (if defined) */
            for (guint cb_nr = 0; callbacks != NULL && cb_nr < callbacks->len; cb_nr++)
            {
                display_inv_callback *cb = g_ptr_array_index(callbacks, cb_nr);

                if ((cb->key == key) && cb->active)
                {
                    /* trigger callback */
                    cb->function(p, cb->inv, inv_get_filtered(*inv, curr + offset - 1, ifilter));

                    redraw = TRUE;

                    /* don't check other callback functions */
                    break;
                }
            }
        };

        len_curr = inv_length_filtered(*inv, ifilter);
    }
    while (keep_running && (len_curr > 0)); /* ESC pressed or empty inventory*/

    display_window_destroy(ipop);
    display_window_destroy(iwin);

    if ((callbacks == NULL) && (key != KEY_ESC))
    {
        /* return selected item if no callbacks have been provided */
        return inv_get_filtered(*inv, offset + curr - 1, ifilter);
    }
    else
    {
        return NULL;
    }
}

void display_inv_callbacks_clean(GPtrArray *callbacks)
{
    if (!callbacks) return;

    while (callbacks->len > 0)
    {
        g_free(g_ptr_array_remove_index_fast(callbacks, callbacks->len - 1));
    }

    g_ptr_array_free(callbacks, TRUE);
}

void display_config_autopickup(player *p)
{
    display_window *cwin;
    int width, height;
    int startx, starty;
    int RUN = TRUE;
    int attrs; /* curses attributes */

    height = 6;
    width = max(36 /* length of first label */, IT_MAX * 2 + 1);

    starty = (LINES - height) / 2;
    startx = (min(MAP_MAX_X, COLS) - width) / 2;

    cwin = display_window_new(startx, starty, width, height, "Autopickup");

    wattron(cwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));
    mvwprintw(cwin->window, 1, 2, "Enabled types are shown inverted");
    mvwprintw(cwin->window, 2, 7, "type symbol to toggle");
    wattroff(cwin->window, attrs);

    do
    {
        int key; /* keyboard input */

        for (item_t it = 1; it < IT_MAX; it++)
        {
            if (p->settings.auto_pickup[it])
                attrs = COLOR_PAIR(DCP_RED_WHITE);
            else
                attrs = COLOR_PAIR(DCP_WHITE_RED);

            wattron(cwin->window, attrs);
            mvwprintw(cwin->window, 4, 6 + it * 2, "%c", item_glyph(it));
            wattroff(cwin->window, attrs);
        }

        wrefresh(cwin->window);

        switch (key = getch())
        {
        case KEY_LF:
        case KEY_CR:
#ifdef PADENTER
        case PADENTER:
#endif
        case KEY_ENTER:
        case KEY_ESC:
        case KEY_SPC:
            RUN = FALSE;
            break;

        default:
            if (!display_window_move(cwin, key))
            {
                for (item_t it = 1; it < IT_MAX; it++)
                {
                    if (item_glyph(it) == key)
                    {
                        p->settings.auto_pickup[it] = !p->settings.auto_pickup[it];
                        break;
                    }
                }
            }
        }
    }
    while (RUN);

    display_window_destroy(cwin);
}

spell *display_spell_select(const char *title, player *p)
{
    display_window *swin;
    guint width, height;
    guint startx, starty;
    guint maxvis;
    int key; /* keyboard input */
    int RUN = TRUE;

    /* currently displayed spell; return value */
    spell *sp;

    /* offset to element position (when displaying more than maxvis items) */
    guint offset = 0;

    /* currently selected item */
    guint curr = 1;

    /* curses attributes */
    int attrs;

    /* window caption */
    gchar *caption;

    g_assert(p != NULL);

    /* buffer for spell code type ahead */
    char *code_buf = g_malloc0(sizeof(char) * 4);

    /* sort spell list  */
    g_ptr_array_sort(p->known_spells, &spell_sort);

    /* set height according to spell count */
    height = min((LINES - 3), (p->known_spells->len + 2));
    maxvis = min(p->known_spells->len, height - 2);

    width = 44;
    starty = (LINES - height) / 2;
    startx = (min(MAP_MAX_X, COLS) - width) / 2;

    swin = display_window_new(startx, starty, width, height, title);

    int prev_key = 0;
    do
    {
        /* display spells */
        for (guint pos = 1; pos <= maxvis; pos++)
        {
            sp = g_ptr_array_index(p->known_spells, pos + offset - 1);

            if (curr == pos) attrs = COLOR_PAIR(DCP_RED_WHITE);
            else attrs = COLOR_PAIR(DCP_WHITE_RED);

            wattron(swin->window, attrs);

            mvwprintw(swin->window, pos, 1, " %3s - %-23s (Lvl %d) %2d ",
                      spell_code(sp),
                      spell_name(sp),
                      spell_level(sp),
                      sp->knowledge);

            wattroff(swin->window, attrs);
        }

        /* display up / down markers */
        display_window_update_arrow_up(swin, (offset > 0));
        display_window_update_arrow_down(swin, ((offset + maxvis) < p->known_spells->len));

        /* construct the window caption: display type ahead keys */
        caption = g_strdup_printf("%s%s%s%s(?) description",
                                  (strlen(code_buf) ? "[" : ""),
                                  code_buf,
                                  (strlen(code_buf) ? "]" : ""),
                                  (strlen(code_buf) ? " " : ""));

        display_window_update_caption(swin, caption);

        /* store currently highlighted spell */
        sp = g_ptr_array_index(p->known_spells, curr + offset - 1);


        switch ((key = getch()))
        {
        case '7':
        case KEY_HOME:
        case KEY_A1:

            curr = 1;
            offset = 0;
            code_buf[0] = '\0';

            break;

        case '9':
        case KEY_PPAGE:
        case KEY_A3:
        case 21: /* ^U */

            if ((curr == maxvis) || offset == 0)
                curr = 1;
            else
                offset = (offset > maxvis) ? (offset - maxvis) : 0;

            code_buf[0] = '\0';
            break;

        case 'k':
        case '8':
        case KEY_UP:
#ifdef KEY_A2
        case KEY_A2:
#endif
            if (key == 'k' && strlen(code_buf) > 0)
                /* yuck, I *hate* gotos, but this one makes sense:
                   it allows to type e.g. 'ckl' */
                goto mnemonics;

            if (curr > 1)
                curr--;

            else if ((curr == 1) && (offset > 0))
                offset--;

            code_buf[0] = '\0';
            break;

        case 'j':
        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            if (key == 'j' && strlen(code_buf) > 0)
                /* see lame excuse above */
                goto mnemonics;

            if ((curr + offset) < p->known_spells->len)
            {
                if (curr == maxvis)
                    offset++;
                else
                    curr++;
            }

            code_buf[0] = '\0';
            break;

        case '3':
        case KEY_NPAGE:
        case KEY_C3:
        case 4: /* ^D */
            if (curr == 1)
            {
                curr = maxvis;
            }
            else
            {
                offset = offset + maxvis;

                if ((offset + maxvis) >= p->known_spells->len)
                {
                    curr = min(p->known_spells->len, maxvis);
                    offset = p->known_spells->len - curr;
                }
            }

            code_buf[0] = '\0';
            break;

        case '1':
        case KEY_END:
        case KEY_C1:
            if (p->known_spells->len > maxvis)
            {
                curr = maxvis;
                offset = p->known_spells->len - maxvis;
            }
            else
            {
                curr = p->known_spells->len;
            }

            code_buf[0] = '\0';
            break;

        case '?':
        case KEY_F(1):
            display_show_message(spell_name(sp), spell_desc(sp), 0);
            break;

        case KEY_ESC:
            RUN = FALSE;
            sp = NULL;

            break;

        case KEY_LF:
        case KEY_CR:
#ifdef PADENTER
        case PADENTER:
#endif
        case KEY_ENTER:
        case KEY_SPC:
            // It is much too easy to accidentally cast alter reality,
            // simply by pressing m + Enter. If the first keypress in
            // the menu confirms this autoselected first spell, prompt.
            if (curr == 1 && prev_key == 0 && sp->id == SP_ALT)
            {
                char prompt[60];
                g_snprintf(prompt, 80, "Really cast %s?", spell_name(sp));
                if (!display_get_yesno(prompt, NULL, NULL))
                    sp = NULL;
            }
            RUN = FALSE;
            break;

        case KEY_BS:
        case KEY_BACKSPACE:
            if (strlen(code_buf))
            {
                code_buf[strlen(code_buf) - 1] = '\0';
            }
            else
            {
                if (!beep())
                    flash();
            }
            break;

        default:
            /* check if the key is used for window placement */
            if (display_window_move(swin, key))
            {
                break;
            }
mnemonics:
            /* add key to spell code buffer */
            if ((key >= 'a') && (key <= 'z'))
            {
                if (strlen(code_buf) < 3)
                {
                    code_buf[strlen(code_buf)] = key;
                    /* search for match */

                    for (guint pos = 1; pos <= p->known_spells->len; pos++)
                    {
                        sp = g_ptr_array_index(p->known_spells, pos - 1);

                        if (g_str_has_prefix(spell_code(sp), code_buf))
                        {
                            /* match found - reposition selection */
                            if (pos > maxvis)
                            {
                                offset = pos - maxvis;
                                curr = pos - offset;
                            }
                            else
                            {
                                offset = 0;
                                curr = pos;
                            }

                            break;
                        }
                    }

                    /* if no match has been found remove key from buffer */
                    sp = g_ptr_array_index(p->known_spells, curr + offset - 1);
                    if (!g_str_has_prefix(spell_code(sp), code_buf))
                    {
                        code_buf[strlen(code_buf) - 1] = '\0';

                        if (!beep())
                            flash();
                    }

                }
                else
                {
                    if (!beep())
                        flash();
                }
            }

            break;
        }
        prev_key = key;
    }
    while (RUN);

    g_free(code_buf);

    display_window_destroy(swin);

    return sp;
}

int display_get_count(const char *caption, int value)
{
    display_window *mwin;
    int height, width, basewidth;
    int startx, starty;

    GPtrArray *text;

    /* curses attributes */
    int attrs;

    /* toggle insert / overwrite mode; start with overwrite */
    int insert_mode = FALSE;

    /* user input */
    int key;

    /* cursor position */
    int ipos = 0;

    /* input as char */
    char ivalue[8] = { 0 };

    /* continue editing the number */
    int cont = TRUE;

    /* 8: input field width; 5: 3 spaces between border, caption + input field, 2 border */
    basewidth = 8 + 5;

    /* choose a sane dialog width */
    width = min(basewidth + strlen(caption), COLS - 4);

    text = text_wrap(caption, width - basewidth, 0);
    height = 2 + text->len;

    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;

    mwin = display_window_new(startx, starty, width, height, NULL);
    wattron(mwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));

    for (guint line = 0; line < text->len; line++)
    {
        /* fill the box background */
        mvwprintw(mwin->window, 1 + line, 1, "%-*s", width - 2, "");
        /* print text */
        mvwprintw(mwin->window, 1 + line, 2, g_ptr_array_index(text, line));
    }

    wattroff(mwin->window, attrs);

    /* prepare string to edit */
    g_snprintf(ivalue, 8, "%d", value);

    wattron(mwin->window, (attrs = COLOR_PAIR(DCP_BLACK_WHITE)));

    do
    {
        int ilen = strlen(ivalue); /* input length */

        /* make cursor visible and set according to insert mode */
        if (insert_mode)
            curs_set(1); /* underline */
        else
            curs_set(2); /* block */

        mvwprintw(mwin->window,  mwin->height - 2, mwin->width - 10,
                  "%-8s", ivalue);

        wmove(mwin->window, mwin->height - 2, mwin->width - 10 + ipos);
        wrefresh(mwin->window);

#ifdef PDCURSES
        key = wgetch(mwin->window);
#else
        key = getch();
#endif

        switch (key)
        {
        case KEY_LEFT:
            if (ipos > 0)
                ipos--;
            break;

        case KEY_RIGHT:
            if (ipos < ilen)
                ipos++;
            break;

        case KEY_BS:
        case KEY_BACKSPACE:
            if ((ipos == ilen) && (ipos > 0))
            {
                ivalue[ipos - 1] = '\0';
                ipos--;
            }
            else if (ipos > 0)
            {
                for (int tmp = ipos - 1; tmp < ilen; tmp++)
                    ivalue[tmp] = ivalue[tmp + 1];

                ipos--;
            }
            break;

        case KEY_IC:
            /* toggle insert mode */
            insert_mode = !insert_mode;
            break;

        case KEY_DC:
            if (ipos < ilen)
            {
                for (int tmp = ipos; tmp < ilen; tmp++)
                    ivalue[tmp] = ivalue[tmp + 1];
            }
            break;

        case KEY_END:
            ipos = ilen;
            break;

        case KEY_HOME:
            ipos = 0;
            break;

            /* special cases to speed up getting/dropping multiple items */
        case 'y': /* yes */
        case 'd': /* drop */
        case 'g': /* get */
        case 'p': /* put */
            /* reset value to original value */
            g_snprintf(ivalue, 8, "%d", value);
            cont = FALSE;
            break;

            /* special case to speed up aborting */
        case 'n': /* no */
            /* set value to 0 */
            g_snprintf(ivalue, 8, "%d", 0);
            cont = FALSE;
            break;


        case KEY_LF:
        case KEY_CR:
#ifdef PADENTER
        case PADENTER:
#endif
        case KEY_ENTER:
        case KEY_ESC:
            cont = FALSE;
            break;

        default:
            if ((key >= '0') && (key <= '9'))
            {
                if (insert_mode)
                {
                    /* insert */
                    if (strlen(ivalue) < 7)
                    {
                        int cppos = strlen(ivalue) + 1;

                        while (cppos >= ipos)
                        {
                            ivalue[cppos] = ivalue[cppos - 1];
                            cppos--;
                        }

                        ivalue[ipos] = key;

                        /* move position */
                        if (ipos < 7) ipos++;
                    }
                    else if (!beep()) flash();
                }
                else
                {
                    /* overwrite */
                    if (ipos < 8)
                    {
                        ivalue[ipos] = key;
                        /* move position */
                        if (ipos < 7) ipos++;
                    }

                    else if (!beep()) flash();
                }
            }
            else if (!display_window_move(mwin, key))
            {
                if (!beep())
                    flash();
            }

            break;
        }
    }
    while (cont);

    wattroff(mwin->window, attrs);

    /* hide cursor */
    curs_set(0);

    text_destroy(text);
    display_window_destroy(mwin);

    if (key == KEY_ESC)
        return 0;

    return atoi(ivalue);
}

char *display_get_string(const char *caption, const char *value, size_t max_len)
{
    /* user input */
    int key;

    /* curses attributes */
    int attrs;

    /* toggle insert / overwrite mode */
    int insert_mode = TRUE;

    /* cursor position */
    guint ipos = 0;

    /* text to be edited */
    GString *string = g_string_new(value);

    /* continue editing the number */
    int cont = TRUE;

    /* 3 spaces between border, caption + input field, 2 border */
    int basewidth = 5;

    /* choose a sane dialog width */
    guint width;
    guint maxwidth = COLS - 4;

    /* prevent overly large input fields */
    if (max_len + basewidth > maxwidth)
    {
        max_len = maxwidth - basewidth;
    }

    if (basewidth + strlen(caption) + max_len > maxwidth)
    {
        if (strlen(caption) + basewidth > maxwidth)
        {
            width = maxwidth - basewidth;
        }
        else
        {
            width = basewidth + max(strlen(caption), max_len);
        }
    }
    else
    {
        /* input box fits on same line as caption */
        width = basewidth + strlen(caption) + max_len + 1;
    }

    GPtrArray *text = text_wrap(caption, width - basewidth, 0);

    /* determine if the input box fits on the last line */
    int box_start = 3 + strlen(g_ptr_array_index(text, text->len - 1));
    if (box_start + max_len + 2 > width)
        box_start = 2;

    int height = 2 + text->len; /* borders and text length */
    if (box_start == 2) height += 1; /* grow the dialog if input box doesn't fit */

    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    display_window *mwin = display_window_new(startx, starty, width, height, NULL);

    wattron(mwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));

    for (guint line = 0; line < text->len; line++)
    {
        /* print text */
        mvwprintw(mwin->window, 1 + line, 1, " %-*s ", width - 4,
                  g_ptr_array_index(text, line));
    }

    wattroff(mwin->window, attrs);
    wattron(mwin->window, (attrs = COLOR_PAIR(DCP_BLACK_WHITE)));

    do
    {
        mvwprintw(mwin->window,  mwin->height - 2, box_start,
                  "%-*s", max_len + 1, string->str);
        wmove(mwin->window, mwin->height - 2, box_start + ipos);

        /* make cursor visible and set according to insert mode */
        if (insert_mode)
            curs_set(1); /* underline */
        else
            curs_set(2); /* block */

        wrefresh(mwin->window);

#ifdef PDCURSES
        key = wgetch(mwin->window);
#else
        key = getch();
#endif

        switch (key)
        {
        case KEY_LEFT:
            if (ipos > 0)
                ipos--;
            break;

        case KEY_RIGHT:
            if (ipos < string->len)
                ipos++;
            break;

        case KEY_BACKSPACE:
        case 8: /* backspace generates 8, not KEY_BACKSPACE on Windows */
            if (ipos > 0)
            {
                g_string_erase(string, ipos - 1, 1);
                ipos--;
            }
            break;

        case KEY_IC:
            /* toggle insert mode */
            insert_mode = !insert_mode;
            break;

        case KEY_DC:
            if (ipos < string->len)
            {
                g_string_erase(string, ipos, 1);
            }
            break;

        case KEY_END:
            ipos = string->len;
            break;

        case KEY_HOME:
            ipos = 0;
            break;

        case KEY_LF:
        case KEY_CR:
#ifdef PADENTER
        case PADENTER:
#endif
        case KEY_ENTER:
        case KEY_ESC:
            cont = FALSE;
            break;

        default:
            /* handle window movement first */
            if (display_window_move(mwin, key))
                break;

            /* filter out unwanted keys */
            if (!g_ascii_isalnum(key) && !g_ascii_ispunct(key) && (key != ' '))
            {
                if (!beep()) flash();
                break;
            }

            if (insert_mode)
            {
                if (string->len < max_len)
                {
                    g_string_insert_c(string, ipos, key);
                    if (ipos < max_len) ipos++;
                }
                else
                {
                    if (!beep()) flash();
                }
            }
            else
            {
                if (ipos < string->len)
                {
                    string->str[ipos] = key;
                    if (ipos < max_len) ipos++;
                }
                else if (string->len < max_len)
                {
                    g_string_append_c(string, key);
                    ipos++;
                }
                else
                {
                    if (!beep()) flash();
                }
            }
            break;
        }
    }
    while (cont);

    wattroff(mwin->window, attrs);

    /* hide cursor */
    curs_set(0);

    text_destroy(text);
    display_window_destroy(mwin);

    if (key == KEY_ESC || string->len == 0)
    {
        g_string_free(string, TRUE);
        return NULL;
    }

    return g_string_free(string, FALSE);
}

int display_get_yesno(const char *question, const char *yes, const char *no)
{
    display_window *ywin;
    guint startx, starty;
    guint width, text_width;
    int RUN = TRUE;
    int selection = FALSE;
    guint line;
    int attrs; /* curses attributes */
    GPtrArray *text;

    const guint padding = 1;
    const guint margin = 2;

    /* default values */
    if (!yes)
        yes = "Yes";

    if (!no)
        no = "No";

    /* determine text width, either defined by space available  for the window
     * or the length of question */
    text_width = min(COLS - 2 /* borders */
                     - (2 * margin) /* space outside window */
                     - (2 * padding), /* space between border and text */
                     strlen(question));

    /* broad windows are hard to read */
    if (text_width > 60)
        text_width = 60;

    /* wrap question according to width */
    text = text_wrap(question, text_width + 1, 0);

    /* determine window width. either defined by the length of the button
     * labels or width of the text */
    width = max(strlen(yes) + strlen(no)
                + 2 /* borders */
                + (4 * padding)  /* space between "button" border and label */
                + margin, /* space between "buttons" */
                text_width + 2 /* borders */ + (2 * padding));

    /* set startx and starty to something that makes sense */
    startx = (min(MAP_MAX_X, COLS) / 2) - (width / 2);
    starty = (LINES / 2) - 4;

    ywin = display_window_new(startx, starty, width, text->len + 4, NULL);

    wattron(ywin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));

    for (line = 0; line < text->len; line++)
    {
        mvwprintw(ywin->window,
                  line + 1,
                  1 + padding,
                  g_ptr_array_index(text, line));
    }

    wattroff(ywin->window, attrs);

    text_destroy(text);

    do
    {
        /* paint */
        if (selection) wattron(ywin->window, (attrs = COLOR_PAIR(DCP_RED_WHITE)));
        else           wattron(ywin->window, (attrs = COLOR_PAIR(DCP_BLACK_WHITE) | A_BOLD));

        mvwprintw(ywin->window, line + 2, margin, "%*s%s%*s", padding, " ",
                  yes, padding, " ");

        wattroff(ywin->window, attrs);

        if (selection) wattron(ywin->window, (attrs = COLOR_PAIR(DCP_BLACK_WHITE) | A_BOLD));
        else           wattron(ywin->window, (attrs = COLOR_PAIR(DCP_RED_WHITE)));

        mvwprintw(ywin->window, line + 2,
                  width - margin - strlen(no) - (2 * padding),
                  "%*s%s%*s", padding, " ", no, padding, " ");

        wattroff(ywin->window, attrs);
        wrefresh(ywin->window);

        int key = tolower(getch()); /* input key buffer */
        // Specialcase for the movement keys and y/n.
        if (key != 'h' && key != 'l' && key != 'y' && key != 'n')
        {
            char input_yes = g_ascii_tolower(yes[0]);
            char input_no  = g_ascii_tolower(no[0]);
            // If both answers share the same initial letter, we're out of luck.
            if (input_yes != input_no || input_yes == 'n' || input_no == 'y')
            {
                if (key == input_yes)
                    key = 'y';
                else if (key == input_no)
                    key = 'n';
            }
        }


        /* wait for input */
        switch (key)
        {
        case KEY_ESC:
            selection = FALSE;
            /* fall-through */

        case KEY_LF:
        case KEY_CR:
#ifdef PADENTER
        case PADENTER:
#endif
        case KEY_ENTER:
        case KEY_SPC:
            RUN = FALSE;
            break;

        case 'h':
        case '4':
#ifdef KEY_B1
        case KEY_B1:
#endif
        case KEY_LEFT:
            if (!selection)
                selection = TRUE;
            break;

        case 'l':
        case '6':
#ifdef KEY_B3
        case KEY_B3:
#endif
        case KEY_RIGHT:
            if (selection)
                selection = FALSE;
            break;

            /* shortcuts */
        case 'y':
            selection = TRUE;
            RUN = FALSE;
            break;

        case 'n':
            selection = FALSE;
            RUN = FALSE;
            break;

        default:
            /* perhaps the window shall be moved */
            display_window_move(ywin, key);
        }
    }
    while (RUN);

    display_window_destroy(ywin);

    return selection;
}

direction display_get_direction(const char *title, int *available)
{
    display_window *dwin;

    int *dirs = NULL;
    int startx, starty;
    int width;
    int attrs; /* curses attributes */
    int RUN = TRUE;

    /* direction to return */
    direction dir = GD_NONE;

    if (!available)
    {
        dirs = g_malloc0(sizeof(int) * GD_MAX);
        for (int x = 0; x < GD_MAX; x++)
            dirs[x] = TRUE;

        dirs[GD_CURR] = FALSE;
    }
    else
    {
        dirs = available;
    }


    width = max(9, strlen(title) + 4);

    /* set startx and starty to something that makes sense */
    startx = (min(MAP_MAX_X, COLS) / 2) - (width / 2);
    starty = (LINES / 2) - 4;

    dwin = display_window_new(startx, starty, width, 9, title);

    wattron(dwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));
    mvwprintw(dwin->window, 3, 3, "\\|/");
    mvwprintw(dwin->window, 4, 3, "- -");
    mvwprintw(dwin->window, 5, 3, "/|\\");
    wattroff(dwin->window, attrs);

    wattron(dwin->window, (attrs = COLOR_PAIR(DCP_YELLOW_RED)));

    for (int x = 0; x < 3; x++)
        for (int y = 0; y < 3; y++)
        {
            if (dirs[(x + 1) + (y * 3)])
                mvwprintw(dwin->window,
                          6 - (y * 2), /* start in the last row, move up, skip one */
                          (x * 2) + 2, /* start in the second col, skip one */
                          "%d",
                          (x + 1) + (y * 3));

        }

    wattroff(dwin->window, attrs);

    if (!available)
        g_free(dirs);

    wrefresh(dwin->window);

    do
    {
        int key; /* input key buffer */

        switch ((key = getch()))
        {

        case 'h':
        case '4':
        case KEY_LEFT:
#ifdef KEY_B1
        case KEY_B1:
#endif
            if (dirs[GD_WEST]) dir = GD_WEST;
            break;

        case 'y':
        case '7':
        case KEY_HOME:
        case KEY_A1:
            if (dirs[GD_NW]) dir = GD_NW;
            break;

        case 'l':
        case '6':
        case KEY_RIGHT:
#ifdef KEY_B3
        case KEY_B3:
#endif
            if (dirs[GD_EAST]) dir = GD_EAST;
            break;

        case 'n':
        case '3':
        case KEY_NPAGE:
        case KEY_C3:
            if (dirs[GD_SE]) dir = GD_SE;
            break;

        case 'k':
        case '8':
        case KEY_UP:
#ifdef KEY_A2
        case KEY_A2:
#endif
            if (dirs[GD_NORTH]) dir = GD_NORTH;
            break;

        case 'u':
        case '9':
        case KEY_PPAGE:
        case KEY_A3:
            if (dirs[GD_NE]) dir = GD_NE;
            break;

        case 'j':
        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            if (dirs[GD_SOUTH]) dir = GD_SOUTH;
            break;

        case 'b':
        case '1':
        case KEY_END:
        case KEY_C1:
            if (dirs[GD_SW]) dir = GD_SW;
            break;

        case '.':
        case '5':
            if (dirs[GD_CURR]) dir = GD_CURR;
            break;

        case KEY_ESC:
            RUN = FALSE;
            break;

        default:
            /* perhaps the window shall be moved */
            display_window_move(dwin, key);
        }
    }
    while ((dir == GD_NONE) && RUN);

    if (!available)
    {
        g_free(dirs);
    }

    display_window_destroy(dwin);

    return dir;
}

position display_get_position(player *p,
                              const char *message,
                              gboolean ray,
                              gboolean ball,
                              guint radius,
                              gboolean passable,
                              gboolean visible)
{
    /* start at player's position */
    position start = p->pos;

    /* the position chosen by the player */
    position cpos;

    /* if the player has recently targeted a monster.. */
    if (visible && p->ptarget != NULL)
    {
        monster *m = game_monster_get(nlarn, p->ptarget);

        /* ..check if it is still alive */
        if (m == NULL)
        {
            /* the monster has been eliminated */
            p->ptarget = NULL;
        }
        else
        {
            /* start at the monster's current position */
            start = monster_pos(m);

            /* don't use invisible position if unwanted */
            if (visible && !fov_get(p->fv, start))
            {
                start = p->pos;
            }
        }
    }

    /* check for visible opponents if no previous opponent has been found */
    if (visible && pos_identical(p->pos, start))
    {
        monster *m = fov_get_closest_monster(p->fv);

        /* found a visible monster -> use it as target */
        if (m != NULL)
            start = monster_pos(m);
    }

    cpos = display_get_new_position(p, start, message, ray, ball, FALSE,
                                    radius, passable, visible);

    if (pos_valid(cpos))
    {
        /* the player has chosen a valid position, check if there
         * is a monster at that position. */
        map *gmap = game_map(nlarn, Z(cpos));
        monster *m = map_get_monster_at(gmap, cpos);

        if (m != NULL)
        {
            /* there is a monster, store its oid for later use */
            p->ptarget = monster_oid(m);
        }
    }

    return cpos;
}

position display_get_new_position(player *p,
                                  position start,
                                  const char *message,
                                  gboolean ray,
                                  gboolean ball,
                                  gboolean travel,
                                  guint radius,
                                  gboolean passable,
                                  gboolean visible)
{
    gboolean RUN = TRUE;
    direction dir = GD_NONE;
    position pos, npos = pos_invalid, cursor = pos_invalid;
    map *vmap;
    int attrs; /* curses attributes */
    display_window *msgpop = NULL;

    /* list of visible monsters and the iterator for these */
    GList *mlist = NULL, *miter = NULL;

    /* variables for ray or ball painting */
    area *b = NULL;       /* a ball area */
    GList *r = NULL;      /* a ray position list */
    monster *target, *m;

    /* check the starting position makes sense */
    if (pos_valid(start) && Z(start) == Z(p->pos))
        pos = start;
    else
        pos = p->pos;

    /* make shortcut to visible map */
    vmap = game_map(nlarn, Z(p->pos));

    /* get the list of visible monsters if looking for a visible position */
    if (visible)
        miter = mlist = fov_get_visible_monsters(p->fv);

    if (!visible)
        msgpop = display_popup(3, min(MAP_MAX_Y + 4, LINES - 4), 0, NULL, message);

    /* if a starting position for a ray has been provided, check if it works */
    if (ray && !pos_identical(p->pos, start))
    {
        /* paint a ray to validate the starting position */
        r = map_ray(vmap, p->pos, pos);

        if (r == NULL)
        {
            /* it's not possible to draw a ray between the points
               -> leave everything as it has been */
            pos = p->pos;
        }
        else
        {
            /* position is valid */
            g_list_free(r);
            r = NULL;
        }
    } /* ray starting position validity check */

    do
    {
        /* refresh the popup content for every position change
           while looking for visible positions */
        if (visible)
        {
            /* clean old popup windows */
            if (msgpop != NULL)
                display_window_destroy(msgpop);

            /* display message or description of selected position */
            if (pos_identical(pos, p->pos))
            {
                msgpop = display_popup(3, min(MAP_MAX_Y + 4, LINES - 4), 0, NULL, message);
            }
            else
            {
                char *desc = map_pos_examine(pos);
                msgpop = display_popup(3, min(MAP_MAX_Y + 4, LINES - 4), 0, message, desc);
                g_free(desc);
            }
        } /* visible */

#ifdef PDCURSES
        /* I have no idea why, but the message popup window is hidden when
        using PDCurses without calling touchwin for it. */
        touchwin(msgpop->window);
#endif
        /* redraw screen to erase previous modifications */
        display_paint_screen(p);

        /* reset npos to an invalid position */
        npos = pos_invalid;

        /* draw a ray if the starting position is not the player's position */
        if (ray && !pos_identical(pos, p->pos))
        {
            r = map_ray(vmap, p->pos, pos);

            if (r == NULL)
            {
                /* It wasn't possible to paint a ray to the target position.
                   Revert to the player's position.*/
                pos = p->pos;
            }
        }

        if (ray && (r != NULL))
        {
            /* draw a line between source and target if told to */
            target = map_get_monster_at(vmap, pos);

            if (target && monster_in_sight(target)) attrs = DC_LIGHTRED;
            else                                    attrs = DC_LIGHTCYAN;

            attron(attrs);
            GList *iter = r;

            do
            {
                position tpos;
                pos_val(tpos) = GPOINTER_TO_UINT(iter->data);

                /* skip the player's position */
                if (pos_identical(p->pos, tpos))
                    continue;

                if (target && pos_identical(monster_pos(target), tpos)
                    && monster_in_sight(target))
                {
                    /* ray is targeted at a visible monster */
                    (void)mvaddch(Y(tpos), X(tpos), monster_glyph(target));
                }
                else if ((m = map_get_monster_at(vmap, tpos))
                         && monster_in_sight(m))
                {
                    /* ray sweeps over a visible monster */
                    (void)mvaddch(Y(tpos), X(tpos), monster_glyph(m));
                }
                else
                {
                    /* a position with no or an invisible monster on it */
                    (void)mvaddch(Y(tpos), X(tpos), '*');
                }
            } while ((iter = iter->next));

            attroff(attrs);
            g_list_free(r);
            r = NULL;
        }
        else if (ball && radius)
        {
            /* paint a ball if told to */
            area *obstacles = map_get_obstacles(vmap, pos, radius, FALSE);
            b = area_new_circle_flooded(pos, radius, obstacles);
            cursor = pos;

            for (Y(cursor) = b->start_y; Y(cursor) < b->start_y + b->size_y; Y(cursor)++)
            {
                for (X(cursor) = b->start_x; X(cursor) < b->start_x + b->size_x; X(cursor)++)
                {
                    if (area_pos_get(b, cursor))
                    {
                        move(Y(cursor), X(cursor));

                        if ((m = map_get_monster_at(vmap, cursor)) && monster_in_sight(m))
                        {
                            attron(attrs = DC_RED);
                            addch(monster_glyph(m));
                        }
                        else if (pos_identical(p->pos, cursor))
                        {
                            attron(attrs = DC_LIGHTRED);
                            addch('@');
                        }
                        else
                        {
                            attron(attrs = DC_LIGHTCYAN);
                            addch('*');
                        }

                        attroff(attrs);
                    }
                }
            }
            area_destroy(b); b = NULL;
        }
        else
        {
            /* show the position of the cursor by inverting the attributes */
            (void)mvwchgat(stdscr, Y(pos), X(pos), 1, A_BOLD | A_STANDOUT, DCP_WHITE_BLACK, NULL);
        }

        /* wait for input */
        const int ch = getch();
        switch (ch)
        {
            /* abort */
        case KEY_ESC:
            pos = pos_invalid;
            RUN = FALSE;
            break;

            /* finish */
        case KEY_LF:
        case KEY_CR:
#ifdef PADENTER
        case PADENTER:
#endif
        case KEY_ENTER:
            RUN = FALSE;
            /* if a passable position has been requested check if it
               actually is passable. Only known positions are allowed. */
            if (passable
                && (!(player_memory_of(nlarn->p, pos).type > LT_NONE
                      || game_wizardmode(nlarn))
                    || !map_pos_passable(vmap, pos)))
            {
                if (!beep()) flash();
                RUN = TRUE;
            }
            break;

            /* jump to next visible monster */
        case KEY_SPC:
            if ((mlist == NULL) && visible)
            {
                flash();
            }
            else if (mlist != NULL)
            {
                /* jump to the next list entry or to the start of
                   the list if the end has been reached */
                if ((miter = miter->next) == NULL)
                    miter = mlist;

                /* get the currently selected monster */
                m = (monster *)miter->data;

                /* jump to the selected monster */
                npos = monster_pos(m);
            }
            break;

            /* move cursor */
        case 'h':
        case '4':
        case KEY_LEFT:
#ifdef KEY_B1
        case KEY_B1:
#endif
            dir = GD_WEST;
            break;

        case 'y':
        case '7':
        case KEY_HOME:
        case KEY_A1:
            dir = GD_NW;
            break;

        case 'l':
        case '6':
        case KEY_RIGHT:
#ifdef KEY_B3
        case KEY_B3:
#endif
            dir = GD_EAST;
            break;

        case 'n':
        case '3':
        case KEY_NPAGE:
        case KEY_C3:
            dir = GD_SE;
            break;

        case 'k':
        case '8':
        case KEY_UP:
#ifdef KEY_A2
        case KEY_A2:
#endif
            dir = GD_NORTH;
            break;

        case 'u':
        case '9':
        case KEY_PPAGE:
        case KEY_A3:
            dir = GD_NE;
            break;

        case 'j':
        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            dir = GD_SOUTH;
            break;

        case 'b':
        case '1':
        case KEY_END:
        case KEY_C1:
            dir = GD_SW;
            break;

        case '@':
            /* bring the cursor back to the player */
            npos = p->pos;
            break;

        default:
            /* if traveling, use sobject glyphs as shortcuts */
            if (travel)
            {
                sobject_t sobj = LS_NONE;
                for (int i = LS_NONE + 1; i < LS_MAX; i++)
                    if (so_get_glyph(i) == (char) ch)
                    {
                        sobj = i;
                        log_add_entry(nlarn->log, "Looking for '%c' (%s).\n",
                                      (char) ch, so_get_desc(sobj));
                        break;
                    }

                /* found a matching glyph, now search the remembered level */
                if (sobj != LS_NONE)
                {
                    position origin = pos;
                    while (TRUE)
                    {
                        if (++X(pos) >= MAP_MAX_X)
                        {
                            X(pos) = 0;
                            if (++Y(pos) >= MAP_MAX_Y)
                                Y(pos) = 0;
                        }
                        if (pos_identical(origin, pos))
                            break;

                        /* When in wizard mode, compare the selected glyph
                           with the real map, otherwise with the player's
                           memory of the map.
                           As multiple objects share the same glyph, it is
                           required to compare the glyph of the present
                           object with the glyph of the requested object. */
                        if ((game_wizardmode(nlarn)
                             && so_get_glyph(map_sobject_at(vmap, pos)) == (char) ch)
                            || (player_memory_of(nlarn->p, pos).sobject != LS_NONE
                                && so_get_glyph(player_memory_of(nlarn->p, pos).sobject) == (char) ch))
                        {
                            break;
                        }
                    }
                }
            } /* if (travel) */
            break;
        }

        /* get new position if cursor has been move */
        if (dir)
        {
            npos = pos_move(pos, dir);
            dir = GD_NONE;
        }

        /* don't want to deal with invalid positions */
        if (pos_valid(npos))
        {
            /* don't use invisible positions */
            if (visible && !fov_get(p->fv, npos))
                npos = pos;

            if (ray)
            {
                /* paint a ray to validate the new position */
                r = map_ray(vmap, p->pos, npos);

                if (r == NULL)
                {
                    /* it's not possible to draw a ray between the points
                       -> return to previous position */
                    npos = pos;
                }
                else
                {
                    g_list_free(r);
                    r = NULL;
                }
            }

            /* new position is within bounds and visible */
            pos = npos;
        } /* if (pos_valid(npos) */
    }
    while (RUN);

    /* destroy list of visible monsters */
    if (mlist != NULL)
        g_list_free(mlist);

    /* destroy the message popup */
    display_window_destroy(msgpop);

    /* hide cursor */
    curs_set(0);

    return pos;
}

void display_show_history(message_log *log, const char *title)
{
    GString *text = g_string_new(NULL);
    char intrep[11] = { 0 }; /* string representation of the game time */
    int twidth; /* the number of characters of the current game time */

    /* determine the width of the current game turn */
    g_snprintf(intrep, 10, "%d", nlarn->gtime);
    twidth = strlen(intrep);

    /* assemble reversed game log */
    for (guint idx = log_length(log); idx > 0; idx--)
    {
        message_log_entry *le = log_get_entry(log, idx - 1);
        g_string_append_printf(text, "%*d: %s\n", twidth, le->gtime, le->message);
    }

    /* display the log */
    display_show_message(title, text->str, twidth + 2);

    /* free the assembled log */
    g_string_free(text, TRUE);
}

int display_show_message(const char *title, const char *message, int indent)
{
    guint height, width, max_len = 0;
    guint startx, starty;
    display_window *mwin;
    int key;

    GPtrArray *text;
    guint maxvis = 0;
    guint offset = 0;

    /* Number of columns required for
         a) the window border and the text margin
         b) the padding around the window */
    const guint wred = 4;

    gboolean RUN = TRUE;

    /* default window width according to available screen space;
       padded by 2 chars on each side */
    width = COLS - wred;

    /* wrap message according to width */
    text = text_wrap(message, width - wred, indent);

    /* determine the length of longest text line */
    for (guint idx = 0; idx < text->len; idx++)
        max_len = max(max_len, strlen(g_ptr_array_index(text, idx)));

    /* shrink the window width if the default width is not required */
    if (max_len + wred < width)
        width = max_len + wred;

    /* set height according to message line count */
    height = min((LINES - 3), (text->len + 2));

    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;

    mwin = display_window_new(startx, starty, width, height, title);

    maxvis = min(text->len, height - 2);

    do
    {
        /* display the window content */
        for (guint idx = 0; idx < maxvis; idx++)
        {
            guint count;
            count = mvwcprintw(mwin->window, DDC_LIGHTGRAY, display_dialog_colset,
                               idx + 1, 1, " %-*s ", width - wred,
                               g_ptr_array_index(text, idx + offset));

            /* If the function has printed less characters than the window
               content width, overwrite the rest of the line with blanks.
               Actually this should be fixed in mvwcprintw(), but that seems
               like lot of work. */

            wattron(mwin->window, DDC_LIGHTGRAY);
            for (guint pos = count; pos < (width - wred); pos++)
                waddch(mwin->window, ' ');
            wattroff(mwin->window, DDC_LIGHTGRAY);
        }

        display_window_update_arrow_up(mwin, offset > 0);
        display_window_update_arrow_down(mwin, (offset + maxvis) < text->len);

        wrefresh(mwin->window);
        key = getch();

        switch (key)
        {
        case 'k':
        case '8':
        case KEY_UP:
#ifdef KEY_A2
        case KEY_A2:
#endif
            if (offset > 0)
                offset--;
            break;

        case '9':
        case KEY_PPAGE:
        case KEY_A3:
        case 21: /* ^U */
            if (offset > maxvis + 1)
                offset -= maxvis;
            else
                offset = 0;
            break;

        case '7':
        case KEY_HOME:
        case KEY_A1:
            offset = 0;
            break;

        case 'j':
        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            if (text->len > (maxvis + offset))
                offset++;
            break;

        case '3':
        case KEY_NPAGE:
        case KEY_C3:
        case 4: /* ^D */
            offset = min((offset + maxvis - 1), text->len - maxvis);
            break;

        case '1':
        case KEY_END:
        case KEY_C1:
            offset = text->len - maxvis;
            break;

        default:
            /* perhaps the window shall be moved */
            if (!display_window_move(mwin, key))
            {
                /* some other key -> close window */
                RUN = FALSE;
            }
        }
    }
    while (RUN);

    display_window_destroy(mwin);
    text_destroy(text);

    return key;
}

display_window *display_popup(int x1, int y1, int width, const char *title, const char *msg)
{
    display_window *win;
    GPtrArray *text;
    int height;
    const guint max_width = COLS - x1 - 1;
    const guint max_height = LINES - y1;

    if (width == 0)
    {
        guint maxlen;

        /* The title is padded by 6 additional characters */
        if ((title != NULL) && (strlen(title) + 6 > strlen(msg)))
            maxlen = strlen(title) + 6;
        else
            maxlen = strlen(msg);

        /* determine window width */
        if (maxlen > (max_width - 4))
            width = max_width - 4;
        else
            width = maxlen + 4;
    }
    else
    {
        /* width supplied. sanity check */
        if ((unsigned)width > max_width)
            width = max_width;
    }

    text = text_wrap(msg, width - 3, 0);
    height = min(text->len + 2, max_height);

    win = display_window_new(x1, y1, width, height, title);

    /* display message */
    for (guint idx = 0; idx < text->len; idx++)
    {
        mvwcprintw(win->window, DDC_WHITE, display_dialog_colset, idx + 1, 1,
                  " %-*s ", width - 4, g_ptr_array_index(text, idx));
    }

    /* clean up */
    text_destroy(text);

    /* show the window */
    wrefresh(win->window);

    return win;
}

void display_window_destroy(display_window *dwin)
{
    del_panel(dwin->panel);
    delwin(dwin->window);

    /* remove window from the list of windows */
    windows = g_list_remove(windows, dwin);

    /* free title as it is a clone */
    if (dwin->title != NULL) g_free(dwin->title);

    g_free(dwin);

    if (nlarn != NULL && nlarn->p != NULL)
    {
        /* repaint the screen if the game has been initialized */
        display_paint_screen(nlarn->p);
    }
    else
    {
        /* refresh the screen */
        display_draw();
    }
}

void display_windows_hide()
{
    GList *iterator = windows;

    while (iterator)
    {
        display_window *win = (display_window *)iterator->data;
        hide_panel(win->panel);
        iterator = iterator->next;
    }
}

void display_windows_show()
{
    GList *iterator = windows;

    while (iterator)
    {
        display_window *win = (display_window *)iterator->data;
        show_panel(win->panel);
        iterator = iterator->next;
    }
}

static int mvwcprintw(WINDOW *win, int defattr, const display_colset *colset,
                      int y, int x, const char *fmt, ...)
{
    va_list argp;
    gchar *msg;
    int count = 0;
    int attr;

    /* assemble the message */
    va_start(argp, fmt);
    msg = g_strdup_vprintf(fmt, argp);
    va_end(argp);

    /* move to the starting position */
    wmove(win, y, x);

    /* set the default attribute */
    wattron(win, attr = defattr);

    for (guint pos = 0; pos < strlen(msg); pos++)
    {
        /* parse tags */
        if (msg[pos] == '`')
        {
            /* position of the tag terminator */
            int tpos;

            /* the tag value */
            char *tval = NULL;

            /* find position of tag terminator */
            for (tpos = pos + 1; msg[tpos] != '`'; tpos++);

            /* extract string between the %s */
            tval = g_strndup(&msg[pos + 1], tpos - pos - 1);

            /* find color value for the tag content */
            if (strcmp(tval, "end") == 0)
            {
                wattroff(win, attr);
                wattron(win, attr = defattr);
            }
            else
            {
                wattroff(win, attr);
                wattron(win, attr = display_get_colval(colset, tval));
            }

            /* free temporary memory */
            g_free(tval);

            /* advance position over the end of the tag */
            pos = tpos + 1;
        }

        /* the end of the string might be reached in the tag parser */
        if (pos >= strlen(msg))
            break;

        /* print the message character wise */
        waddch(win, msg[pos]);

        /* increment the count of printed chars */
        count++;
    }

    /* reset the default attribute */
    wattroff(win, defattr);

    /* clean assembled string */
    g_free(msg);

    return count;
}

static void display_inventory_help(GPtrArray *callbacks)
{
    size_t maxlen = 0;
    GString *help = g_string_new(NULL);

    if (callbacks == NULL || callbacks->len == 0)
    {
        /* no callbacks available => select item with ENTER */
        g_string_append(help, "Select the desired item with ENTER.\n"
                        "You may abort by pressing the escape key.");
    }
    else
    {
        /* determine the maximum length of the description */
        for (guint i = 0; i < callbacks->len; i++)
        {
            display_inv_callback *cb = g_ptr_array_index(callbacks, i);

            /* skip this callback function if it is not active */
            if (!cb->active) continue;

            size_t desclen = strlen(cb->description);
            if (desclen > maxlen)
                maxlen = desclen;
        }

        if (maxlen == 0)
        {
            /* no active callbacks */
            g_string_append(help, "There are no options available for the selected item.");
        }
        else
        {
            for (guint i = 0; i < callbacks->len; i++)
            {
                display_inv_callback *cb = g_ptr_array_index(callbacks, i);

                if (cb->active && cb->helpmsg != NULL)
                {
                    g_string_append_printf(help, "`lightgreen`%*s:`end` %s\n",
                                           (int)maxlen,
                                           cb->description,
                                           cb->helpmsg);
                }
            }
        }
    }

    display_show_message("Help", help->str, maxlen + 2);
    g_string_free(help, TRUE);
}

static int display_get_colval(const display_colset *colset, const char *name)
{
    int colour = 0;
    int pos = 0;

    while (colset[pos].name != NULL)
    {
        if (strcmp(name, colset[pos].name) == 0)
        {
            /* colour found */
            colour = colset[pos].val;
            break;
        }

        pos++;
    }

    return colour;
}

static display_window *display_window_new(int x1, int y1, int width,
                                          int height, const char *title)
{
    display_window *dwin;
    int attrs; /* curses attributes */

    dwin = g_malloc0(sizeof(display_window));

    dwin->x1 = x1;
    dwin->y1 = y1;
    dwin->width = width;
    dwin->height = height;

    dwin->window = newwin(dwin->height, dwin->width, dwin->y1, dwin->x1);

#ifdef PDCURSES
    /* PDCurses does not inherit keypad setting from stdscr */
    keypad(dwin->window, TRUE);
#endif

    /* fill window background */
    wattron(dwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));

    for (int i = 1; i < height; i++)
        mvwprintw(dwin->window, i, 1, "%*s", width - 2, "");

    wattroff(dwin->window, attrs);

    /* draw borders */
    wattron(dwin->window, (attrs = COLOR_PAIR(DCP_BLUE_RED)));
    box(dwin->window, 0, 0);
    wattroff(dwin->window, attrs);

    /* set the window title */
    display_window_update_title(dwin, title);

    /* create a panel for the window */
    dwin->panel = new_panel(dwin->window);

    /* add window to the list of opened windows */
    windows = g_list_append(windows, dwin);

    if (nlarn != NULL && nlarn->p != NULL)
    {
        /* repaint the screen if the game has been initialized */
        display_paint_screen(nlarn->p);
    }
    else
    {
        /* refresh panels */
        update_panels();
    }

    return dwin;
}

static int display_window_move(display_window *dwin, int key)
{
    gboolean need_refresh = TRUE;

    g_assert (dwin != NULL);

    switch (key)
    {
    case 0:
        /* the windows keys generate two key presses, of which the first
           is a zero. flush the buffer or the second key code will confuse
           everything. This happens here as all dialogs call this function
           after everything else. */
        flushinp();
        break;

        /* ^left */
    case 541: /* NCurses - Linux */
    case 443: /* PDCurses - Windows */
        if (dwin->x1 > 0) dwin->x1--;
        break;

        /* ^right */
    case 556: /* NCurses - Linux */
    case 444: /* PDCurses - Windows */
        if (dwin->x1 < (COLS - dwin->width)) dwin->x1++;
        break;

        /* ^up */
    case 562: /* NCurses - Linux */
    case 480: /* PDCurses - Windows */
        if (dwin->y1 > 0) dwin->y1--;
        break;

        /* ^down */
    case 521: /* NCurses - Linux */
    case 481: /* PDCurses - Windows */
        if (dwin->y1 < (LINES - dwin->height)) dwin->y1++;
        break;

    default:
        need_refresh = FALSE;
    }

    if (need_refresh)
    {
        move_panel(dwin->panel, dwin->y1, dwin->x1);
        (void)display_draw();
    }

    return need_refresh;
}

static void display_window_update_title(display_window *dwin, const char *title)
{
    int attrs; /* curses attributes */

    g_assert (dwin != NULL && dwin->window != NULL);

    if (dwin->title)
    {
        /* free the previous title */
        g_free(dwin->title);

        /* repaint line to overwrite previous title */
        wattron(dwin->window, (attrs = COLOR_PAIR(DCP_BLUE_RED)));
        (void)mvwhline(dwin->window, 0, 2, ACS_HLINE, dwin->width - 7);
        wattroff(dwin->window, attrs);
    }

    /* print the provided title */
    if (title && strlen(title))
    {
        /* copy the new title
         * the maximum length is determined by the window width
         * minus the space required for the left corner (3)
         * minus the space reqired for the right corner
         *       and the scroll marker (7)
         */
        dwin->title = g_strndup(title, dwin->width - 10);

        /* make sure the first letter of the window title is upper case */
        dwin->title[0] = g_ascii_toupper(dwin->title[0]);

        wattron(dwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));
        mvwprintw(dwin->window, 0, 2, " %s ", dwin->title);
        wattroff(dwin->window, attrs);
    }

    wrefresh(dwin->window);
}

static void display_window_update_caption(display_window *dwin, char *caption)
{
    int attrs; /* curses attributes */

    g_assert (dwin != NULL && dwin->window != NULL);

    /* repaint line to overwrite previous captions */
    wattron(dwin->window, (attrs = COLOR_PAIR(DCP_BLUE_RED)));
    (void)mvwhline(dwin->window, dwin->height - 1, 3, ACS_HLINE, dwin->width - 7);
    wattroff(dwin->window, attrs);

    /* print caption if caption is set */
    if (caption && strlen(caption))
    {
        wattron(dwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));
        mvwprintw(dwin->window, dwin->height - 1, 3, " %s ", caption);
        wattroff(dwin->window, attrs);

        /* free the provided caption */
        g_free(caption);
    }

    wrefresh(dwin->window);
}

static void display_window_update_arrow_up(display_window *dwin, gboolean on)
{
    int attrs; /* curses attributes */

    g_assert (dwin != NULL && dwin->window != NULL);

    if (on)
    {
        wattron(dwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));
        mvwprintw(dwin->window, 0, dwin->width - 5, " ^ ");
        wattroff(dwin->window, attrs);
    }
    else
    {
        wattron(dwin->window, (attrs = COLOR_PAIR(DCP_BLUE_RED)));
        (void)mvwhline(dwin->window, 0, dwin->width - 5, ACS_HLINE, 3);
        wattroff(dwin->window, attrs);
    }
}

static void display_window_update_arrow_down(display_window *dwin, gboolean on)
{
    int attrs; /* curses attributes */

    g_assert (dwin != NULL && dwin->window != NULL);

    if (on)
    {
        wattron(dwin->window, (attrs = COLOR_PAIR(DCP_WHITE_RED)));
        mvwprintw(dwin->window, dwin->height - 1, dwin->width - 5, " v ");
        wattroff(dwin->window, attrs);
    }
    else
    {
        wattron(dwin->window, (attrs = COLOR_PAIR(DCP_BLUE_RED)));
        (void)mvwhline(dwin->window, dwin->height - 1, dwin->width - 5, ACS_HLINE, 3);
        wattroff(dwin->window, attrs);
    }
}

static display_window *display_item_details(guint x1, guint y1, guint width,
                                            item *it, player *p, gboolean shop)
{
    /* the popup window created by display_popup */
    display_window *idpop;

    /* string for the content of the item description popup */
    char *msg = NULL;

    /* determine if the item is known or displayed in the shop */
    const gboolean known = shop | player_item_known(p, it);

    /* the detailed item description */
    msg = item_detailed_description(it, known, shop);
    idpop = display_popup(x1, y1, width, "Item details", msg);

    /* tidy up */
    g_free(msg);

    return idpop;
}

static void display_spheres_paint(sphere *s, player *p)
{
    /* check if sphere is on current level */
    if (!(Z(s->pos) == Z(p->pos)))
        return;

    if (game_fullvis(nlarn) || fov_get(p->fv, s->pos))
    {
        attron(DC_MAGENTA);
        (void)mvaddch(Y(s->pos), X(s->pos), '0');
        attroff(DC_MAGENTA);
    }
}
