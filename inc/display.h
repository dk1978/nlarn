/*
 * display.h
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

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "game.h"
#include "items.h"
#include "player.h"

enum display_colours {
    DC_NONE,
    DC_WHITE,
    DC_RED,
    DC_GREEN,
    DC_BLUE,
    DC_YELLOW,
    DC_MAGENTA,
    DC_CYAN,
    DC_BLACK,
    DC_MAX
};

typedef struct display_inv_callback {
	char *description;
	char key;
	int (*function)(player *, item *);
	int (*checkfun)(player *, item *);
	gboolean active;
} display_inv_callback;

/* function declarations */

int display_init();
void display_shutdown();
int display_paint_screen(player *p);
inline int display_draw();

void display_inventory(char *title, player *p, inventory *inv, GPtrArray *callbacks);
spell *display_spell_select(char *title, player *p, GPtrArray *callbacks);
void display_player(player *p);

int display_get_count(char *caption, int value);
int display_get_yesno(char *question, char *yes, char *no);
direction display_get_direction(char *title, int *available);
position display_get_position(player *p, char *message);

void display_show_history(message_log *log, char *title);
char display_show_message(char *title, char *message);

#endif
