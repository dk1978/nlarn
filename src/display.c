/*
 * display.c
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

static int display_rows = 0;
static int display_cols = 0;

static WINDOW *display_window_new(int x1, int y1, int width, int height, char *title, char *caption);

int display_init()
{
    /* Start curses mode */
    initscr();

    /* get screen dimensions */
    getmaxyx(stdscr, display_rows, display_cols);
    start_color();

    /* black background */
    init_pair(DC_WHITE,   COLOR_WHITE,   COLOR_BLACK);
    init_pair(DC_RED,     COLOR_RED,     COLOR_BLACK);
    init_pair(DC_GREEN,   COLOR_GREEN,   COLOR_BLACK);
    init_pair(DC_BLUE,    COLOR_BLUE,    COLOR_BLACK);
    init_pair(DC_YELLOW,  COLOR_YELLOW,  COLOR_BLACK);
    init_pair(DC_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(DC_CYAN,    COLOR_CYAN,    COLOR_BLACK);
    init_pair(DC_BLACK,   COLOR_BLACK,   COLOR_BLACK);

    /* colour pairs 9 - 13 are used by dialogs */
    init_pair(9,	COLOR_WHITE,    COLOR_RED);
    init_pair(10,	COLOR_RED,      COLOR_WHITE);
    init_pair(11,	COLOR_BLUE,     COLOR_RED);
    init_pair(12,	COLOR_YELLOW,   COLOR_RED);
    init_pair(13,	COLOR_BLACK,    COLOR_WHITE);
    init_pair(14,	COLOR_CYAN,     COLOR_RED);

    /* green background */
    init_pair(15, 	COLOR_BLACK,    COLOR_GREEN);
    init_pair(16,	COLOR_WHITE,    COLOR_GREEN);
    init_pair(17,	COLOR_RED,      COLOR_GREEN);
    init_pair(18,	COLOR_BLUE,     COLOR_GREEN);
    init_pair(19,	COLOR_YELLOW,   COLOR_GREEN);
    init_pair(20,	COLOR_MAGENTA,  COLOR_GREEN);
    init_pair(21,	COLOR_CYAN,     COLOR_GREEN);
    /* blue background */
    init_pair(22, 	COLOR_WHITE,	COLOR_BLUE);
    init_pair(23,	COLOR_RED,      COLOR_BLUE);
    init_pair(24,	COLOR_GREEN,    COLOR_BLUE);
    init_pair(25,	COLOR_BLUE,     COLOR_BLUE);
    init_pair(26,	COLOR_YELLOW,   COLOR_BLUE);
    init_pair(27,	COLOR_MAGENTA,  COLOR_BLUE);
    init_pair(28,	COLOR_CYAN,     COLOR_BLUE);

    /* control special keys in application */
    raw();

    /* supress input echo */
    noecho();

    /* enable function keys */
    keypad(stdscr, TRUE);

    /* want all 8 bits */
    meta(stdscr, TRUE);

    /* make cursor invisible */
    curs_set(0);

    return TRUE;
}

int display_paint_screen(player *p)
{
    int x, y, i;
    position pos;
    item *it;
    monster *monst;
    sphere *sphere;
    level *l;

    /* needed to display messages */
    message_log_entry *le;

    /* storage for formatted messages */
    GPtrArray *text = NULL;

    /* storage for the game time of messages */
    int *ttime = NULL;

    /* refresh screen dimensions */
    getmaxyx(stdscr, display_rows, display_cols);

    /* draw line around map */
    mvhline(LEVEL_MAX_Y, 0, ACS_HLINE, LEVEL_MAX_X);
    mvvline(0, LEVEL_MAX_X, ACS_VLINE, LEVEL_MAX_Y);
    mvaddch(LEVEL_MAX_Y, LEVEL_MAX_X, ACS_LRCORNER);

    /* make shortcut */
    l = p->level;

    /* draw map */
    for (pos.y = 0; pos.y < LEVEL_MAX_Y; pos.y++)
    {
        /* position cursor */
        move(pos.y, 0);

        for (pos.x = 0; pos.x < LEVEL_MAX_X; pos.x++)
        {
            if (player_pos_visible(p, pos)) attron(A_BOLD);

            /* draw the truth */
            if (game_wizardmode(p->game))
            {

                /* draw items */
                if (level_ilist_at(p->level, pos) && (level_ilist_at(p->level, pos)->len > 0))
                {

                    it = (item *) inv_get(level_ilist_at(p->level, pos),
                                          level_ilist_at(p->level, pos)->len - 1);

                    attron(COLOR_PAIR(DC_BLUE));
                    addch(item_get_image(it->type));
                    attroff(COLOR_PAIR(DC_BLUE));
                }
                /* draw stationary stuff */
                else if (level_stationary_at(p->level, pos))
                {
                    attron(COLOR_PAIR(ls_get_colour(level_stationary_at(p->level, pos))));
                    addch(ls_get_image(level_stationary_at(p->level, pos)));
                    attroff(COLOR_PAIR(ls_get_colour(level_stationary_at(p->level, pos))));
                }
                else if (level_trap_at(p->level, pos))
                {
                    attron(COLOR_PAIR(DC_MAGENTA));
                    addch('^');
                    attroff(COLOR_PAIR(DC_MAGENTA));
                }
                /* draw tile */
                else
                {
                    attron(COLOR_PAIR(lt_get_colour(level_tiletype_at(p->level, pos))));
                    addch(lt_get_image(level_tiletype_at(p->level, pos)));
                    attroff(COLOR_PAIR(lt_get_colour(level_tiletype_at(p->level, pos))));
                }

            }
            else /* i.e. !wizardmode */
                /* draw players fov & memory */
            {
                /* draw items */
                if (player_memory_of(p, pos).item)
                {
                    attron(COLOR_PAIR(DC_BLUE));
                    addch(item_get_image(player_memory_of(p, pos).item));
                    attroff(COLOR_PAIR(DC_BLUE));
                }
                /* draw stationary stuff */
                else if (player_memory_of(p, pos).stationary)
                {
                    attron(COLOR_PAIR(ls_get_colour(player_memory_of(p, pos).stationary)));
                    addch(ls_get_image(player_memory_of(p, pos).stationary));
                    attroff(COLOR_PAIR(ls_get_colour(player_memory_of(p, pos).stationary)));
                }
                else if (player_memory_of(p, pos).trap)
                {
                    attron(COLOR_PAIR(DC_MAGENTA));
                    addch('^');
                    attroff(COLOR_PAIR(DC_MAGENTA));
                }
                /* draw tile */
                else
                {
                    attron(COLOR_PAIR(lt_get_colour(player_memory_of(p, pos).type)));
                    addch(lt_get_image(player_memory_of(p, pos).type));
                    attroff(COLOR_PAIR(lt_get_colour(player_memory_of(p, pos).type)));
                }
            }

            if (player_pos_visible(p, pos)) attroff(A_BOLD);
        }
    }

    /* draw monsters */
    for (i = 1; i <= l->mlist->len; i++)
    {
        monst = (monster *) g_ptr_array_index(l->mlist, i - 1);

        if (game_wizardmode(p->game)
                || player_effect(p, ET_DETECT_MONSTER)
                || (player_pos_visible(p, monst->pos)
                && (!monster_is_invisible(monst) || player_effect(p, ET_INFRAVISION))))
        {
            attron(COLOR_PAIR(DC_RED));
            mvaddch(monst->pos.y, monst->pos.x, monster_get_image(monst));
            attroff(COLOR_PAIR(DC_RED));
        }

    }

    /* draw spheres */
    for (i = 1; i <= l->slist->len; i++)
    {
        sphere = g_ptr_array_index(l->slist, i - 1);

        if (game_wizardmode(p->game) || player_pos_visible(p, sphere->pos))
        {
            attron(COLOR_PAIR(DC_MAGENTA));
            mvaddch(sphere->pos.y, sphere->pos.x, '0');
            attroff(COLOR_PAIR(DC_MAGENTA));
        }
    }

    /* *** status line *** */
    move(LEVEL_MAX_Y + 1, 0);
    clrtoeol();

    printw("%s, %s",
           p->name,
           player_get_lvl_desc(p));

    attron(COLOR_PAIR(DC_MAGENTA));
    mvprintw(LEVEL_MAX_Y + 1, LEVEL_MAX_X - 21, "%3d HP %-3d", p->hp, player_get_hp_max(p));
    attroff(COLOR_PAIR(DC_MAGENTA));

    attron(COLOR_PAIR(DC_CYAN));
    mvprintw(LEVEL_MAX_Y + 1, LEVEL_MAX_X - 10, "%3d MP %-3d", p->mp, player_get_mp_max(p));
    attroff(COLOR_PAIR(DC_CYAN));

    /* *** RIGHT STATUS *** */
    mvprintw(1, LEVEL_MAX_X + 3, "STR %2d", player_get_str(p));
    clrtoeol();
    mvprintw(2, LEVEL_MAX_X + 3, "DEX %2d", player_get_dex(p));
    clrtoeol();
    mvprintw(3, LEVEL_MAX_X + 3, "CON %2d", player_get_con(p));
    clrtoeol();
    mvprintw(4, LEVEL_MAX_X + 3, "INT %2d", player_get_int(p));
    clrtoeol();
    mvprintw(5, LEVEL_MAX_X + 3, "WIS %2d", player_get_wis(p));
    clrtoeol();
    mvprintw(6, LEVEL_MAX_X + 3, "CHA %2d", player_get_cha(p));
    clrtoeol();

    mvprintw(8, LEVEL_MAX_X + 3, "AC: %2d", player_get_ac(p));
    clrtoeol();
    mvprintw(9, LEVEL_MAX_X + 3, "WC: %2d", player_get_wc(p));
    clrtoeol();

    mvprintw(11, LEVEL_MAX_X + 3, "XL: %d", p->lvl);
    clrtoeol();
    mvprintw(12, LEVEL_MAX_X + 3, "XP: %d", p->experience);
    clrtoeol();
    mvprintw(13, LEVEL_MAX_X + 3, "$%-7d", player_get_gold(p));
    clrtoeol();

    mvprintw(15, LEVEL_MAX_X + 3, "Levl: %2d", p->level->nlevel);
    clrtoeol();
    mvprintw(16, LEVEL_MAX_X + 3, "t %-7d", p->log->gtime);
    clrtoeol();


    /* *** MESSAGES *** */
    /* number of lines which can be displayed */
    y = display_rows - 20;

    /* storage for game time of message */
    ttime = g_new0(int, y);

    /* hold original length of text */
    x = 1;

    /* line counter */
    i = 0;

    /* if log contains buffered messaged, display them */
    if (log_buffer(p->log))
    {
        text = text_wrap(log_buffer(p->log), display_cols, 2);
        for (x = 1; x <= min(text->len, y); x++)
            ttime[x - 1] = game_turn(p->game);
    }

    /* retrieve game log and reformat messages to window width */
    while (((text == NULL) || (text->len < y)) && (log_length(p->log) > i))
    {
        le = log_get_entry(p->log, log_length(p->log) - i);

        if (text == NULL)
            text = text_wrap(le->message, display_cols, 2);
        else
            text = text_append(text, text_wrap(le->message, display_cols, 2));

        /* store game time for associated text line */
        while ((x <= text->len) && (x <= y))
        {
            ttime[x - 1] = le->gtime;
            x++;
        }

        i++;
    }

    for (y = 20, i = 0; (y < display_rows) && (i < text->len); i++, y++)
    {
        move(y, 0);
        clrtoeol();

        if ((p->log->gtime - 15) < ttime[i])
            attron(A_BOLD);

        printw(g_ptr_array_index(text, i));

        if ((p->log->gtime - 15) < ttime[i])
            attroff(A_BOLD);
    }

    text_destroy(text);
    g_free(ttime);


    /* draw player - has to happen in the end to position cursor */
    if (player_effect(p, ET_INVISIBILITY))
    {
        curs_set(1);
        move(p->pos.y, p->pos.x);
    }
    else
    {
        curs_set(0);
        attron(A_BOLD | COLOR_PAIR(DC_YELLOW));
        mvaddch(p->pos.y, p->pos.x, '@');
        attroff(A_BOLD | COLOR_PAIR(DC_YELLOW));
    }

    return display_draw();
}

void display_shutdown()
{
    /* End curses mode */
    endwin();
}

inline int display_draw()
{
    return refresh();
}

/**
 * Generic inventory display function
 *
 * @param Window caption
 * @param player
 * @param inventory to display
 * @param a GPtrArray of display_inv_callbacks
 */
void display_inventory(char *title, player *p, inventory *inv, GPtrArray *callbacks)
{
    WINDOW *iwin = NULL;
    int width, height;
    int startx, starty;
    int len;
    gboolean redraw = FALSE;

    /* can display a maximum of 18 rows */
    const int maxvis = 18;

    /* window caption assembled from callback descriptions */
    char *caption, *tmp;

    /* used for looping over callbacks */
    int cb_nr;
    display_inv_callback *cb;
    int key;

    /* time usage returned by callback function */
    int time;

    /* offset to element position (when displaying more than maxvis items) */
    int offset = 0;

    /* currently selected item */
    int curr = 1;

    /* position in inventory (loop var) */
    int pos;
    item *it;
    char item_desc[81];

    assert(p != NULL && callbacks != NULL);

    /* sort inventory by item type */
    g_ptr_array_sort(inv, &item_sort);

    /* store inventory length */
    len = inv_length(inv);

    /* main loop */
    do
    {
        height = min(inv_length(inv) + 2, maxvis + 2);

        width = 50;
        starty = (display_rows - height) / 2; /* calculation for centered */
        startx = (display_cols - width) / 2; /* placement of the window */

        /* fix marked item */
        if (curr > inv_length(inv))
            curr = inv_length(inv);

        /* rebuild image */
        if (iwin != NULL)
        {
            delwin(iwin);

            /* repaint game screen only when needed */
            if (redraw)
            {
                clear();
                display_paint_screen(p);
                redraw = FALSE;

                /* inventory length is smaller than before */
                if (len > inv_length(inv))
                {
                    /* if on the last page, reduce offset */
                    if ((offset > 0) && ((offset + maxvis) > inv_length(inv)))
                        offset--;

                    /* remember current length */
                    len = inv_length(inv);
                }
            }
        }

        it = inv_get(inv, curr + offset - 1);

        /* assemble window caption */
        for (caption = NULL, cb_nr = 1; cb_nr <= callbacks->len; cb_nr++)
        {
            cb = g_ptr_array_index(callbacks, cb_nr - 1);

            /* check if callback is approriate for this item */
            /* if no checkfunktion is set, always display item */
            if ((cb->checkfun == NULL) || cb->checkfun(p, it))
            {
                cb->active = TRUE;

                if (caption)
                {
                    tmp = g_strconcat(caption, " ", cb->description, NULL);
                    g_free(caption);
                    caption = tmp;
                }
                else
                {
                    caption = g_strdup(cb->description);
                }
            }
            else
            {
                /* it isn't */
                cb->active = FALSE;
            }
        }

        if (caption)
        {
            iwin = display_window_new(startx, starty, width, height, title, caption);
            g_free(caption);
        }

        for (pos = 1; pos <= min(inv_length(inv), maxvis); pos++)
        {
            it = inv_get(inv, (pos - 1) + offset);

            if ((curr == pos) && player_item_is_equipped(p, it))
                wattron(iwin, COLOR_PAIR(13));
            else if (curr == pos)
                wattron(iwin, COLOR_PAIR(10));
            else if (player_item_is_equipped(p, it))
                wattron(iwin, COLOR_PAIR(9) | A_BOLD);
            else
                wattron(iwin, COLOR_PAIR(9));

            mvwprintw(iwin, pos, 1, " %2d %41s %c ",
                      pos + offset,
                      item_describe(it,
                                    player_item_identified(p, it),
                                    FALSE, FALSE,
                                    item_desc, 80),
                      player_item_is_equipped(p, it) ? '*' : ' ');

            if ((curr == pos) && player_item_is_equipped(p, it))
                wattroff(iwin, COLOR_PAIR(13));
            else if (curr == pos)
                wattroff(iwin, COLOR_PAIR(10));
            else if (player_item_is_equipped(p, it))
                wattroff(iwin, COLOR_PAIR(9) | A_BOLD);
            else
                wattroff(iwin, COLOR_PAIR(9));

        }

        if (offset > 0)
        {
            wattron(iwin, COLOR_PAIR(9));
            mvwprintw(iwin, 0, width - 5, " ^ ");
            wattroff(iwin, COLOR_PAIR(9));
        }
        else
        {
            wattron(iwin, COLOR_PAIR(11));
            mvwhline(iwin, 0, width - 5, ACS_HLINE, 3);
            wattroff(iwin, COLOR_PAIR(11));
        }

        if ((offset + maxvis) < inv_length(inv))
        {
            wattron(iwin, COLOR_PAIR(9));
            mvwprintw(iwin, height - 1, width - 5, " v ");
            wattroff(iwin, COLOR_PAIR(9));
        }
        else
        {
            wattron(iwin, COLOR_PAIR(11));
            mvwhline(iwin, height - 1, width - 5, ACS_HLINE, 3);
            wattroff(iwin, COLOR_PAIR(11));
        }

        wrefresh(iwin);

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

            if ((curr == maxvis) || offset == 0)
                curr = 1;
            else
                offset = max(offset - maxvis, 0);

            break;

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

        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            if ((curr + offset) < inv_length(inv))
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

            if (curr == 1)
            {
                curr = maxvis;
            }
            else
            {
                offset = offset + maxvis;

                if ((offset + curr) > inv_length(inv))
                {
                    curr = min(len, maxvis);
                    offset = inv_length(inv) - curr;
                }
            }
            break;

        case '1':
        case KEY_END:
        case KEY_C1:

            if (inv_length(inv) > maxvis)
            {
                curr = maxvis;
                offset = inv_length(inv) - maxvis;
            }
            else
            {
                curr = inv_length(inv);
            }

            break;

        default:
            /* check callback function keys */
            for (cb_nr = 1; cb_nr <= callbacks->len; cb_nr++)
            {
                cb = g_ptr_array_index(callbacks, cb_nr - 1);

                if ((cb->key == key) && cb->active)
                {
                    time = 0;

                    /* trigger callback */
                    time = cb->function(p, inv_get(inv, curr + offset - 1));

                    if (time)
                        game_spin_the_wheel(p->game, time);

                    redraw = TRUE;
                }
            }
        };

        /* fake ESC to close window if no items are left */
        if (!inv_length(inv))
        {
            key = 27;
        }
    }
    while (key != 27); /* ESC */

    delwin(iwin);
    clear();
}

spell *display_spell_select(char *title, player *p, GPtrArray *callbacks)
{
    WINDOW *swin;
    int width, height;
    int startx, starty;
    int maxvis;
    int pos;
    int RUN = TRUE;

    /* currently displayed spell; return value */
    spell *sp;

    /* offset to element position (when displaying more than maxvis items) */
    int offset = 0;

    /* currently selected item */
    int curr = 1;


    assert(p != NULL);


    if (!p->known_spells || !p->known_spells->len)
    {
        log_add_entry(p->log, "You don't know any spells.");
        return NULL;
    }

    /* sort spell list  */
    g_ptr_array_sort(p->known_spells, &spell_sort);

    /* set height according to message line count */
    height = min((display_rows - 3), (p->known_spells->len + 2));
    maxvis = min(p->known_spells->len, height - 2);

    width = 44;
    starty = (display_rows - height) / 2;
    startx = (display_cols - width) / 2;

    swin = display_window_new(startx, starty, width, height, title, "");

    do
    {
        /* display spells */
        for (pos = 1; pos <= maxvis; pos++)
        {
            sp = g_ptr_array_index(p->known_spells, pos + offset - 1);

            if (curr == pos)
                wattron(swin, COLOR_PAIR(10));
            else
                wattron(swin, COLOR_PAIR(9));


            mvwprintw(swin, pos, 1, " %3s - %-23s (Lvl %d) %2d ",
                      spell_code(sp),
                      spell_name(sp),
                      spell_level(sp),
                      sp->knowledge);

            if (curr == pos)
                wattroff(swin, COLOR_PAIR(10));
            else
                wattroff(swin, COLOR_PAIR(9));

        }

        /* display up / down markers */
        if (offset > 0)
        {
            wattron(swin, COLOR_PAIR(9));
            mvwprintw(swin, 0, width - 5, " ^ ");
            wattroff(swin, COLOR_PAIR(9));
        }
        else
        {
            wattron(swin, COLOR_PAIR(11));
            mvwhline(swin, 0, width - 5, ACS_HLINE, 3);
            wattroff(swin, COLOR_PAIR(11));
        }

        if ((offset + maxvis) < p->known_spells->len)
        {
            wattron(swin, COLOR_PAIR(9));
            mvwprintw(swin, height - 1, width - 5, " v ");
            wattroff(swin, COLOR_PAIR(9));
        }
        else
        {
            wattron(swin, COLOR_PAIR(11));
            mvwhline(swin, height - 1, width - 5, ACS_HLINE, 3);
            wattroff(swin, COLOR_PAIR(11));
        }

        wrefresh(swin);

        switch (getch())
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
            if ((curr == maxvis) || offset == 0)
                curr = 1;
            else
                offset = max(offset - maxvis, 0);
            break;

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

        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            if ((curr + offset) < p->known_spells->len)
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
            if (curr == 1)
            {
                curr = maxvis;
            }
            else
            {
                offset = offset + maxvis;

                if ((offset + curr) > p->known_spells->len)
                {
                    curr = min(p->known_spells->len, maxvis);
                    offset = p->known_spells->len - curr;
                }
            }
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
            break;

        case 27:
            RUN = FALSE;
            sp = NULL;

            break;

        case 10:
        case 13:
        case KEY_ENTER:
        case 32: /* space bar */
            RUN = FALSE;
            sp = g_ptr_array_index(p->known_spells, curr + offset - 1);
            break;
        }
    }
    while (RUN);

    delwin(swin);
    clear();

    return sp;
}

void display_show_player(player *p)
{

}

int display_get_count(char *caption, int value)
{
    WINDOW *mwin;
    int height, width;
    int startx, starty;

    int tmp;

    /* caption length */
    int len;

    /* user input */
    int key;

    /* input length */
    int ilen = 0;

    /* cursor position */
    int ipos = 0;

    /* input as char */
    char ivalue[7];

    /* continue editing the number */
    int cont = TRUE;

    /* 6: input field width; 5: 3 spaces between border, caption + input field, 2 border */
    len = strlen(caption);
    width = len + 6 + 5;
    height = 3;

    starty = (display_rows - height) / 2;
    startx = (display_cols - width) / 2;

    mwin = display_window_new(startx, starty, width, height, "", "");

    /* fill the box background */
    wattron(mwin, COLOR_PAIR(9));
    mvwprintw(mwin, 1, 1, "%-*s", width - 2, caption);
    wattroff(mwin, COLOR_PAIR(9));

    /* make cursor visible */
    curs_set(1);

    /* prepare string to edit */
    snprintf(ivalue, 7, "%d", value);
    /* set boundary at end of array */
    ivalue[6] = '\0';

    ilen = strlen(ivalue);

    wattron(mwin, COLOR_PAIR(13));

    do
    {

        mvwprintw(mwin, 1, len + 3, "%-6s", ivalue);
        wmove(mwin, 1, len + 3 + ipos);

        wrefresh(mwin);

        key = wgetch(mwin);

        switch (key)
        {
        case KEY_LEFT:
            if (ipos > 0)
                ipos--;
            break;

        case KEY_RIGHT:
            if ((ipos < 5) && (ipos < ilen))
                ipos++;
            break;

        case KEY_BACKSPACE:
            if ((ipos == ilen) && (ipos > 0))
            {
                ivalue[ipos - 1] = '\0';
                ipos--;
            }
            else if (ipos > 0)
            {
                for (tmp = ipos - 1; tmp < ilen; tmp++)
                    ivalue[tmp] = ivalue[tmp + 1];

                ipos--;
            }

            break;

        case KEY_DC:
            if (ipos < ilen)
            {
                for (tmp = ipos; tmp < ilen; tmp++)
                    ivalue[tmp] = ivalue[tmp + 1];
            }
            break;

        case KEY_END:
            ipos = min(5, ilen);
            break;

        case KEY_HOME:
            ipos = 0;
            break;

        case 10: /* LF */
        case 13: /* CR */
        case 27: /* ESC */
        case KEY_ENTER: /* keypad enter */
            cont = FALSE;
            break;

        default:
            if (key == ' ')
            {
                if ((ipos == 5)
                        || (ivalue[ipos + 1] == ' ')
                        || (ivalue[ipos + 1] == '\0'))
                {
                    /* clear current number */
                    ivalue[ipos] = '\0';
                }
            }
            else if ((key >= '0') && (key <= '9'))
            {
                if ((ipos == 5) || (ivalue[ipos - 1] != ' '))
                {
                    ivalue[ipos] = key;

                    /* set terminator if needed */
                    if (ipos == ilen)
                        ivalue[ipos + 1] = '\0';

                    /* move position */
                    if (ipos < 5) ipos++;
                }
                else
                {

                }

            }
            else
            {
                if (!beep())
                    flash();
            }

            break;
        }

        ilen = strlen(ivalue);
    }
    while (cont);

    wattroff(mwin, COLOR_PAIR(13));

    /* hide cursor */
    curs_set(0);

    delwin(mwin);
    clear();

    if (key == 27)
        return value;

    return atoi(ivalue);
}

int display_get_yesno(char *question, char *yes, char *no)
{
    WINDOW *ywin;

    int startx, starty;
    int width, text_width;

    int RUN = TRUE;
    int selection = FALSE;
    int line;

    GPtrArray *text;

    const int padding = 1;
    const int margin = 2;

    /* default values */
    if (!yes)
        yes = "Yes";

    if (!no)
        no = "No";

    /* determine text width, either defined by space available  for the window
     * or the length of question */
    text_width = min(display_cols - 2 /* borders */
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
    startx = (display_cols / 2) - (width / 2);
    starty = (display_rows / 2) - 4;

    ywin = display_window_new(startx, starty, width, text->len + 4, NULL, NULL);

    wattron(ywin, COLOR_PAIR(9));

    for (line = 0; line < text->len; line++)
        mvwprintw(ywin,
                  line + 1,
                  1 + padding,
                  g_ptr_array_index(text, line));

    wattroff(ywin, COLOR_PAIR(9));

    text_destroy(text);

    do
    {
        /* paint */
        if (selection) wattron(ywin, COLOR_PAIR(13) | A_BOLD);
        else           wattron(ywin, COLOR_PAIR(10));

        mvwprintw(ywin, line + 2, margin, "%*s%s%*s", padding, " ",
                  yes, padding, " ");

        if (selection)  wattroff(ywin, COLOR_PAIR(13) | A_BOLD);
        else            wattroff(ywin, COLOR_PAIR(10));

        if (!selection) wattron(ywin, COLOR_PAIR(13) | A_BOLD);
        else            wattron(ywin, COLOR_PAIR(10));

        mvwprintw(ywin, line + 2,
                  width - margin - strlen(no) - (2 * padding),
                  "%*s%s%*s", padding, " ", no, padding, " ");

        if (!selection) wattroff(ywin, COLOR_PAIR(13) | A_BOLD);
        else            wattroff(ywin, COLOR_PAIR(10));

        wrefresh(ywin);

        /* wait for input */
        switch (getch())
        {
        case 27: /* ESC */
            selection = FALSE;
            /* fall through desired */

        case 10: /* LF */
        case 13: /* CR */
        case KEY_ENTER: /* keypad enter */
        case 32: /* space bar */
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
        case 'Y':
            selection = TRUE;
            RUN = FALSE;
            break;

        case 'n':
        case 'N':
            selection = FALSE;
            RUN = FALSE;
            break;
        }
    }
    while (RUN);

    delwin(ywin);
    clear();

    return selection;
}

direction display_get_direction(char *title, int *available)
{
    WINDOW *dwin;

    int *dirs;
    int startx, starty;
    int width;
    int x, y;
    int RUN = TRUE;

    /* direction to return */
    direction dir = GD_NONE;

    if (!available)
    {
        dirs = g_malloc0(sizeof(int) * GD_MAX);
        for (x = 0; x < GD_MAX; x++)
            dirs[x] = TRUE;

        dirs[GD_CURR] = FALSE;
    }
    else
    {
        dirs = available;
    }


    width = max(9, strlen(title) + 4);

    /* set startx and starty to something that makes sense */
    startx = (display_cols / 2) - (width / 2);
    starty = (display_rows / 2) - 4;

    dwin = display_window_new(startx, starty, width, 9, title, NULL);


    wattron(dwin, COLOR_PAIR(9));

    mvwprintw(dwin, 3, 3, "\\|/");
    mvwprintw(dwin, 4, 3, "- -");
    mvwprintw(dwin, 5, 3, "/|\\");

    wattroff(dwin, COLOR_PAIR(9));


    wattron(dwin, COLOR_PAIR(12));

    for (x = 0; x < 3; x++)
        for (y = 0; y < 3; y++)
        {
            if (dirs[(x + 1) + (y * 3)])
                mvwprintw(dwin,
                          6 - (y * 2), /* start in the last row, move up, skip one */
                          (x * 2) + 2, /* start in the second col, skip one */
                          "%d",
                          (x + 1) + (y * 3));

        }

    wattroff(dwin, COLOR_PAIR(12));

    if (!available)
        g_free(dirs);

    wrefresh(dwin);

    do
    {
        switch (getch())
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

        case 27:
            RUN = FALSE;
            break;
        }
    }
    while ((dir == GD_NONE) && RUN);

    delwin(dwin);
    clear();

    return dir;
}

position display_get_position(player *p, char *message, int draw_line, int passable)
{
    int RUN  = TRUE;
    int direction = GD_NONE;
    position pos, npos;

    /* variables for ray painting */
    area *ray;
    int distance = 0;
    int x, y;
    monster *target;

    /* start at player's position */
    pos = p->pos;

    /* display message */
    log_add_entry(p->log, message);
    display_paint_screen(p);

    /* show cursor */
    curs_set(1);

    do
    {
        distance = max(abs(pos.x - p->pos.x),
                       abs(pos.y - p->pos.y));

        /* redraw screen to erase old rays */
        if (draw_line) display_paint_screen(p);

        /* draw a line between source and target if told to */
        if (draw_line && distance)
        {
            target = level_get_monster_at(p->level, pos);

            display_paint_screen(p);

            if (target)
            {
                attron(COLOR_PAIR(DC_RED) | A_BOLD);
            }
            else
            {
                attron(COLOR_PAIR(DC_CYAN) | A_BOLD);
            }

            ray = area_new_ray(p->pos,
                               pos,
                               level_get_obstacles(p->level,
                                                   p->pos,
                                                   distance));

            for (y = 0; y < ray->size_y; y++)
            {
                for (x = 0; x < ray->size_x; x++)
                {
                    if (area_point_get(ray, x, y))
                    {
                        if (target
                                && (target->pos.x == ray->start_x + x)
                                && (target->pos.y == ray->start_y + y))
                        {
                            mvaddch(ray->start_y + y, ray->start_x + x, monster_get_image(target));
                        }
                        else
                        {
                            mvaddch(ray->start_y + y, ray->start_x + x, '*');
                        }

                    }
                }
            }

            if (target)
            {
                attroff(COLOR_PAIR(DC_RED) | A_BOLD);
            }
            else
            {
                attroff(COLOR_PAIR(DC_CYAN) | A_BOLD);
            }

            area_destroy(ray);
        }

        /* position cursor */
        move(pos.y, pos.x);

        /* wait for input */
        switch (getch())
        {
        case 27: /* ESC */
            pos.x = G_MAXINT16;
            pos.y = G_MAXINT16;
            /* fall through desired */

        case 10: /* LF */
        case 13: /* CR */
        case KEY_ENTER: /* keypad enter */
        case 32: /* space bar */
            RUN = FALSE;
            break;

            /* move cursor */
        case 'h':
        case '4':
        case KEY_LEFT:
#ifdef KEY_B1
        case KEY_B1:
#endif
            direction = GD_WEST;
            break;

        case 'y':
        case '7':
        case KEY_HOME:
        case KEY_A1:
            direction = GD_NW;
            break;

        case 'l':
        case '6':
        case KEY_RIGHT:
#ifdef KEY_B3
        case KEY_B3:
#endif
            direction = GD_EAST;
            break;

        case 'n':
        case '3':
        case KEY_NPAGE:
        case KEY_C3:
            direction = GD_SE;
            break;

        case 'k':
        case '8':
        case KEY_UP:
#ifdef KEY_A2
        case KEY_A2:
#endif
            direction = GD_NORTH;
            break;

        case 'u':
        case '9':
        case KEY_PPAGE:
        case KEY_A3:
            direction = GD_NE;
            break;

        case 'j':
        case '2':
        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            direction = GD_SOUTH;
            break;

        case 'b':
        case '1':
        case KEY_END:
        case KEY_C1:
            direction = GD_SW;
            break;
        }

        if (direction)
        {
            npos = pos_move(pos, direction);
            direction = GD_NONE;
        }

        if (pos_valid(npos) && player_pos_visible(p, npos))
        {
            if (passable && !level_pos_passable(p->level, npos))
                /* a passable position has been requested and this one isn't */
                continue;

            /* new position is within bounds and visible */
            pos = npos;

            /* make npos invalid */
            npos.x = G_MAXINT16;
            npos.y = G_MAXINT16;
        }

    }
    while (RUN);

    /* hide cursor */
    curs_set(0);

    return pos;
}

void display_show_history(message_log *log, char *title)
{
    int i;
    char *text = NULL;
    char *tmp = NULL;
    message_log_entry *le;

    for (i = 1; i <= log_length(log); i++)
    {
        le = log_get_entry(log, i);

        if (!text)
        {
            text = g_strconcat(le->message, "\n", NULL);
        }
        else
        {
            tmp = g_strconcat(text, le->message, "\n", NULL);
            g_free(text);
            text = tmp;
        }
    }

    display_show_message(title, text);
    g_free(text);
}

/**
 * Simple "popup" message window
 * @param window title
 * @param message to be displayed inside window
 * @return key pressed to close window
 */
char display_show_message(char *title, char *message)
{
    int height, width;
    int startx, starty;
    WINDOW *mwin;
    int key;

    GPtrArray *text;
    int pos = 0;
    int maxvis = 0;
    int offset = 0;

    gboolean RUN = TRUE;

    width = display_cols - 10;

    /* wrap message according to width */
    text = text_wrap(message, width - 4, 0);

    /* set height according to message line count */
    height = min((display_rows - 3), (text->len + 2));

    starty = (display_rows - height) / 2;
    startx = (display_cols - width) / 2;

    mwin = display_window_new(startx, starty, width, height, title, "");
    maxvis = min(text->len, height - 2);

    do
    {
        for (pos = 1; pos <= maxvis; pos++)
        {
            wattron(mwin, COLOR_PAIR(9));
            mvwprintw(mwin, pos, 1, " %-*s ",
                      width - 4,
                      g_ptr_array_index(text, pos - 1 + offset));

            wattroff(mwin, COLOR_PAIR(9));
        }

        if (offset > 0)
        {
            wattron(mwin, COLOR_PAIR(9));
            mvwprintw(mwin, 0, width - 5, " ^ ");
            wattroff(mwin, COLOR_PAIR(9));
        }
        else
        {
            wattron(mwin, COLOR_PAIR(11));
            mvwhline(mwin, 0, width - 5, ACS_HLINE, 3);
            wattroff(mwin, COLOR_PAIR(11));
        }

        if ((offset + maxvis) < text->len)
        {
            wattron(mwin, COLOR_PAIR(9));
            mvwprintw(mwin, height - 1, width - 5, " v ");
            wattroff(mwin, COLOR_PAIR(9));
        }
        else
        {
            wattron(mwin, COLOR_PAIR(11));
            mvwhline(mwin, height - 1, width - 5, ACS_HLINE, 3);
            wattroff(mwin, COLOR_PAIR(11));
        }

        wrefresh(mwin);
        key = getch();

        switch (key)
        {
        case KEY_UP:
#ifdef KEY_A2
        case KEY_A2:
#endif
            if (offset > 0)
                offset--;
            break;

        case KEY_PPAGE:
        case KEY_A3:
            offset = max(offset - maxvis + 1, 0);
            break;

        case KEY_HOME:
        case KEY_A1:
            offset = 0;
            break;

        case KEY_DOWN:
#ifdef KEY_C2
        case KEY_C2:
#endif
            if (text->len > (maxvis + offset))
                offset++;
            break;

        case KEY_NPAGE:
        case KEY_C3:
            offset = min((offset + maxvis - 1), text->len - maxvis);
            break;

        case KEY_END:
        case KEY_C1:
            offset = text->len - maxvis;
            break;

        default:
            RUN = FALSE;
        }
    }
    while (RUN);

    wattroff(mwin, COLOR_PAIR(9));

    delwin(mwin);
    clear();
    text_destroy(text);

    return key;
}

static WINDOW *display_window_new(int x1, int y1, int width, int height, char *title, char *caption)
{
    WINDOW *nwin;
    int i;

    nwin = newwin(height, width, y1, x1);

    /* fill window background */
    wattron(nwin, COLOR_PAIR(9));

    for (i = 1; i < height; i++)
        mvwprintw(nwin, i, 1, "%*s", width - 2, "");

    wattroff(nwin, COLOR_PAIR(9));


    /* draw borders */
    wattron(nwin, COLOR_PAIR(11));

    box(nwin, 0, 0);

    wattroff(nwin, COLOR_PAIR(11));

    wattron(nwin, COLOR_PAIR(9) | A_BOLD);

    if (title && strlen(title))
    {
        mvwprintw(nwin, 0, 1, " %s ", title);
    }

    if (caption && strlen(caption))
    {
        mvwprintw(nwin, height - 1, 1, " %s ", caption);
    }

    wattroff(nwin, COLOR_PAIR(9) | A_BOLD);

    return nwin;
}
