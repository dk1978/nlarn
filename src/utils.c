/*
 * utils.c
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

int divert(int value, int percent)
{
    int lower, upper;

    assert(value > 0 && percent > 0);

    lower = value - (value / percent);
    upper = value + (value / percent);
    if (lower == upper)
        return value;

    return rand_m_n(lower, upper);
}

/**
 * Shuffle an array of integers
 *
 * @param pointer to integer array
 * @param length of array
 * @param how many fields should be skipped
 */
void shuffle(int array[], int length, int skip)
{
    int i;
    int npos;
    int temp;

    for (i = 0; i < length; i++)
    {
        /* fill the array in order */
        array[i] = i;
    }

    for (i = skip; i < (length / 2); i++)
    {
        /* randomize positions */
        npos = i + g_random_int_range(0, length - i);
        temp = array[i];
        array[i] = array[npos];
        array[npos] = temp;
    }
}

char *str_replace(char *string, char *orig, char *replace)
{
    static char buffer[1024];
    char *ch;

    if (!(ch = strstr(string, orig)))
        return string;

    strncpy(buffer, string, ch - string);
    buffer[ch - string] = 0;
    sprintf(buffer + (ch - string),
            "%s%s",
            replace,
            ch + strlen(orig));

    return buffer;
}

message_log *log_new()
{
    message_log *log;

    log = g_malloc0(sizeof(message_log));

    /* to force message assemble on turn 0 */
    log->gtime = -1;
    log->active = TRUE;
    log->buffer = g_string_new(NULL);

    return log;
}

/**
 * Update the game time. This function flushes the message buffer and appends
 * the collected messages to the log.
 *
 * @param the log
 * @param the new game time
 *
 */
void log_set_time(message_log *log, int gtime)
{
    message_log_entry *entry;

    assert(log != NULL);

    /* flush pending entry */
    if ((log->buffer)->len)
    {
        entry = g_malloc(sizeof(message_log_entry));
        entry->gtime = log->gtime;
        entry->ltime = time(NULL);
        entry->message = (log->buffer)->str;

        /* allocate storage space */
        log->entries = g_realloc(log->entries,
                                 (log->length + 1) * sizeof(message_log_entry *));

        /* append the entry to the message log */
        log->entries[log->length] = entry;
        log->length++;

        /* destroy buffer and add prepare new one */
        g_string_free(log->buffer, FALSE);
        log->buffer = g_string_new("");
    }

    log->gtime = gtime;
}

int log_add_entry(message_log *log, char *fmt, ...)
{
    va_list argp;

    if (log == NULL || log->active == FALSE)
        return FALSE;

    /* if there is already text in the buffer, append a space first */
    if (log->buffer->len)
        g_string_append_c(log->buffer, ' ');

    /* assemble message and append it to the buffer */
    va_start(argp, fmt);
    g_string_append_vprintf(log->buffer, fmt, argp);
    va_end(argp);

    return TRUE;
}

message_log_entry *log_get_entry(message_log *log, int id)
{
    assert(log != NULL && id > 0 && id <= log->length);
    return(log->entries[id - 1]);
}

void log_delete(message_log *log)
{
    int i;

    assert(log != NULL);
    for (i = log->length; i > 0; i--)
    {
        g_free(log->entries[i - 1]->message);
        g_free(log->entries[i - 1]);
    }

    g_free(log->entries);

    g_string_free(log->buffer, TRUE);
    g_free(log);
}

GPtrArray *text_wrap(char *str, int width, int indent)
{
    GPtrArray *text;
    int pos = 0;        /* position in string */
    int lp;             /* last position of whitespace */
    int len;            /* current line length */
    char *line, *tmp;   /* copy of line */
    char *spaces = NULL;

    text = g_ptr_array_new();

    if (indent)
    {
        spaces = g_malloc((indent + 1) * sizeof(char));
        for (lp = 0; lp < indent; lp++)
            spaces[lp] = ' ';

        spaces[indent] = '\0';
    }

    while (pos < strlen(str))
    {
        len = lp = 0;
        while (len < width)
        {
            if ((str[pos + len] == '\0') || isspace(str[pos + len]))
            {
                lp = len;
                if (str[pos + lp] == '\n')
                {
                    break;
                }
            }
            len++;
        }
        line = g_malloc((lp + 1) * sizeof(char));
        memcpy(line, &(str[pos]), lp);
        line[lp] = '\0';

        /* no indentation on the first line */
        if (indent && text->len == 1)
            width -= indent;

        if (indent && text->len > 0)
        {
            tmp = g_strconcat(spaces, line, NULL);
            g_free(line);
            line = tmp;
        }

        g_ptr_array_add(text, line);

        pos += (lp + 1);
    }

    if (spaces)
        g_free(spaces);

    return text;
}

/**
 * append one array of text to another array of text.
 *
 * @param text to append to
 * @param text to append. this will be freed.
 * @return pointer to combined array.
 *
 */
GPtrArray *text_append(GPtrArray *first, GPtrArray *second)
{
    assert(first != NULL && second != NULL);

    while (second->len > 0)
        g_ptr_array_add(first,
                        g_ptr_array_remove_index(second, 0));

    text_destroy(second);

    return first;
}

void text_destroy(GPtrArray *text)
{
    assert(text != NULL);

    while (text->len > 0)
        g_free(g_ptr_array_remove_index_fast(text, 0));

    g_ptr_array_free(text, TRUE);
}

int str_starts_with_vowel(char *str)
{
    const char vowels[] = "aeioAEIO";

    assert (str != NULL);

    if (strchr(vowels, str[0])) return TRUE;
    else return FALSE;
}


