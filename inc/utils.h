/*
 * utils.h
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

#ifndef __UTILS_H_
#define __UTILS_H_

#include <time.h>
#include "cJSON.h"
#include "defines.h"

/* game messaging */
typedef struct message_log_entry
{
    time_t ltime;       /* real time of log entry */
    guint32 gtime;      /* game time of log entry */
    char *message;
} message_log_entry;

typedef struct message_log
{
    guint32 length;     /* number of entries */
    guint32 gtime;      /* current game time */
    gint32 active;      /* flag to disable logging onto this log */
    GString *buffer;    /* space to assemble a turn's messages */
    char *lastmsg;      /* copy of last message */
    message_log_entry **entries;
} message_log;

/* macros */

#define rand_1n(n)      (((n) <= 1) ? 1 : g_random_int_range(1,(n)))
#define rand_0n(n)      (((n) <= 0) ? 0 : g_random_int_range(0,(n)))
#define rand_m_n(m,n)   ((m) == (n) ? (m) : g_random_int_range((m),(n)))
#define chance(percent) ((percent) >= rand_1n(101))

/* windef.h defines these */
#ifdef WIN32
#undef min
#undef max
#endif

#define min(x,y)    (((x) > (y)) ? (y) : (x))
#define max(x,y)    (((x) > (y)) ? (x) : (y))
#define even(x)     (!((x) % 2))

/* function definitions */
int divert(int value, int percent);
void shuffle(int array[], int length, int skip);
char *str_replace(char *string, char *orig, char *replace);

/* message log handling */
message_log *log_new();
int log_add_entry(message_log *log, char *fmt, ...);
void log_set_time(message_log *log, int gtime);
void log_delete(message_log *log);
message_log_entry *log_get_entry(message_log *log, guint id);

cJSON *log_serialize(message_log *log);
message_log *log_deserialize(cJSON *lser);

#define log_length(log) ((log)->length)
#define log_enable(log) ((log)->active = TRUE)
#define log_disable(log) ((log)->active = FALSE)
#define log_buffer(log) ((log)->buffer->len ? (log)->buffer->str : NULL)

/* text array handling */
GPtrArray *text_wrap(char *str, int width, int indent);
GPtrArray *text_append(GPtrArray *first, GPtrArray *second);
void text_destroy(GPtrArray *text);

/* misc. text functions */
int str_starts_with_vowel(char *str);
const char *int2str(int val);
#define a_an(str) (str_starts_with_vowel((str)) ? "an" : "a")
#define plural(i) (((i) > 1) ? "s" : "")


/* regarding stuff defined in defines.h */
damage *damage_new(damage_t type, attack_t attack, int amount, gpointer originator);
#define damage_free(dam)    g_free((dam))

#endif
