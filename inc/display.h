/*
 * display.h
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

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <curses.h>
#include <panel.h>

#include "game.h"
#include "items.h"
#include "player.h"


/* missing key definitions */
#define KEY_BS   8 /* backspace */
#define KEY_TAB  9 /* tab */
#define KEY_LF  10 /* line feed, i.e. numeric keypad enter */
#define KEY_FF  12 /* form feed */
#define KEY_CR  13 /* carriage return */
#define KEY_ESC 27 /* escape */
#define KEY_SPC 32 /* space */

enum display_colour_pairs
{
    DCP_NONE,
    DCP_WHITE_BLACK,
    DCP_RED_BLACK,
    DCP_GREEN_BLACK,
    DCP_BLUE_BLACK,
    DCP_YELLOW_BLACK,
    DCP_MAGENTA_BLACK,
    DCP_CYAN_BLACK,
    DCP_BLACK_BLACK,
    DCP_WHITE_RED,
    DCP_RED_WHITE,
    DCP_BLUE_RED,
    DCP_YELLOW_RED,
    DCP_BLACK_WHITE,
    DCP_CYAN_RED,
    DCP_MAX
};

enum display_colours
{
    DC_NONE,
    DC_BLACK        = COLOR_PAIR(DCP_BLACK_BLACK),
    DC_RED          = COLOR_PAIR(DCP_RED_BLACK),
    DC_GREEN        = COLOR_PAIR(DCP_GREEN_BLACK),
    DC_BROWN        = COLOR_PAIR(DCP_YELLOW_BLACK),
    DC_BLUE         = COLOR_PAIR(DCP_BLUE_BLACK),
    DC_MAGENTA      = COLOR_PAIR(DCP_MAGENTA_BLACK),
    DC_CYAN         = COLOR_PAIR(DCP_CYAN_BLACK),
    DC_LIGHTGRAY    = COLOR_PAIR(DCP_WHITE_BLACK),
    DC_DARKGRAY     = COLOR_PAIR(DCP_BLACK_BLACK) | A_BOLD,
    DC_LIGHTRED     = COLOR_PAIR(DCP_RED_BLACK) | A_BOLD,
    DC_LIGHTGREEN   = COLOR_PAIR(DCP_GREEN_BLACK) | A_BOLD,
    DC_YELLOW       = COLOR_PAIR(DCP_YELLOW_BLACK) | A_BOLD,
    DC_LIGHTBLUE    = COLOR_PAIR(DCP_BLUE_BLACK) | A_BOLD,
    DC_LIGHTMAGENTA = COLOR_PAIR(DCP_MAGENTA_BLACK) | A_BOLD,
    DC_LIGHTCYAN    = COLOR_PAIR(DCP_CYAN_BLACK) | A_BOLD,
    DC_WHITE        = COLOR_PAIR(DCP_WHITE_BLACK) | A_BOLD,
    DC_MAX
};

typedef struct display_inv_callback
{
    char *description;
    char key;
    inventory **inv;
    void (*function)(player *, inventory **, item *);
    int (*checkfun)(player *, item *);
    gboolean active;
} display_inv_callback;

typedef struct display_window
{
    guint x1;
    guint y1;
    guint width;
    guint height;
    char *title;
    char *caption;
    WINDOW *window;
    PANEL *panel;
} display_window;

/* function declarations */

void display_init();
void display_shutdown();
void display_wrap(lua_State *L);
int display_draw();

int display_paint_screen(player *p);

/**
 * Generic inventory display function
 *
 * @param Window title
 * @param player
 * @param inventory to display
 * @param a GPtrArray of display_inv_callbacks (may be NULL)
 * @param display prices
 * @param a filter function: will be called for every item
 * @return if no callbacks have been supplied, the selected item will be returned on pressing enter
 *
 */
item *display_inventory(const char *title, player *p, inventory **inv,
                        GPtrArray *callbacks, gboolean show_price,
                        gboolean show_weight, gboolean show_account,
                        int (*filter)(item *));

void display_inv_callbacks_clean(GPtrArray *callbacks);

void display_config_autopickup(player *p);

spell *display_spell_select(char *title, player *p);

int display_get_count(char *caption, int value);
char *display_get_string(char *caption, char *value, size_t max_len);
int display_get_yesno(char *question, char *yes, char *no);
direction display_get_direction(char *title, int *available);
position display_get_position(player *p, char *message, gboolean ray,
                              gboolean ball, guint radius,
                              gboolean passable, gboolean visible);

void display_show_history(message_log *log, const char *title);

/**
 * Simple "popup" message window
 *
 * @param window title
 * @param message to be displayed inside window
 * @param the number of chars wrapped lines will be indented
 * @return key pressed to close window
 */
int display_show_message(const char *title, const char *message, int indent);

void display_windows_hide();
void display_windows_show();

#define display_getch getch


#endif
