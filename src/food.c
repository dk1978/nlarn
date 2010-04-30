/*
 * food.c
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

#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "food.h"
#include "game.h"
#include "nlarn.h"
#include "player.h"
#include "utils.h"

const food_data foods[FT_MAX] =
{
    /* id                name              wt pr*/
    { FT_NONE,           "",                0, 0, },
    { FT_FORTUNE_COOKIE, "fortune cookie", 10, 2, },
};

static char *food_get_fortune(char *fortune_file);

item_usage_result food_eat(struct player *p, item *food)
{
    char description[61];
    item_usage_result result;

    result.identified = TRUE;
    result.used_up = TRUE;
    result.time = 2;

    item_describe(food, player_item_known(p, food),
                  TRUE, FALSE, description, 60);

    log_add_entry(nlarn->log, "You eat %s.", description);

    if (food->id == FT_FORTUNE_COOKIE)
    {
        log_add_entry(nlarn->log,
                      "It has a piece of paper inside. It reads: \"%s\"",
                      food_get_fortune(game_fortunes(nlarn)));
    }

    return result;
}

/* function to return a random fortune from the fortune file  */
static char *food_get_fortune(char *fortune_file)
{

    /* array of pointers to fortunes */
    static GPtrArray *fortunes = NULL;

    if (!fortunes) {

    	/* read in the fortunes */

		size_t len = 0;
		char buffer[80];
		char *tmp = 0;
		FILE *fortune_fd;

    	fortunes = g_ptr_array_new();

        /* open the file */
        fortune_fd = fopen(fortune_file, "r");
        if (fortune_fd == NULL) {
        	/* can't find file */
        	tmp = "Help me! I can't find the fortune file!";
            g_ptr_array_add(fortunes, tmp);
        }
        else
        {
            /* read in the entire fortune file */
            while((fgets(buffer, 79, fortune_fd))) {
                /* replace EOL with \0 */
                len = (size_t)(strchr(buffer, '\n') - (char *)&buffer);
                buffer[len] = '\0';

                /* keep the line */
                tmp = g_malloc((len + 1) * sizeof(char));
                memcpy(tmp, &buffer, (len + 1));
                g_ptr_array_add(fortunes, tmp);
            }

            fclose(fortune_fd);
        }
    }

    return g_ptr_array_index(fortunes, rand_0n(fortunes->len));
}
