/************************************************************************
sc - Station Controller
Copyright (C) 2005 
Written by John F. Poirier DBA Edge Integration

Contact information:
    john@edgeintegration.com

    Edge Integration
    885 North Main Street
    Raynham MA, 02767

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
************************************************************************/

#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include.h"

#define MAX_LIST 500

static const char *ts();
static uint32_t list[MAX_LIST];
static uint32_t num_list = 0;

static uint32_t hash(const char *);

/*************************************************/

void debug(uint32_t level, const char *id, const char *filename, const char *name, int line_number, const char *fmt, ...)
{
	uint32_t i     = 0;
	uint32_t ihash = 0;
    va_list args;

    if (g_debug < level)
    {
        return;
    }

    if (num_list)
    {
        ihash = hash(id);
        for (i=0; i<num_list; i++)
        {
            if (list[i] == ihash)
            {
                break;
            }
        }

        if (i >= num_list)
        {
            return; 
        }
    }

    /* Do the debug */

    fprintf(stdout, "DEBUG-%s,:%10.10s::%10.10s:%3.3d ", ts(), filename, name, line_number);

    for (i=0; i<level; i++)
    {
        fprintf(stdout, "-");
    }

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);

    return;
}

/*************************************************/

void debug_init()
{
	FILE *fd = (FILE *)NULL;
	char tmp[128];

#undef NAME
#define NAME "debug_init()"

    if ((fd = fopen("sc.debug", "r")) == NULL)
    {
        num_list = 0;
        return;
    }

    while (1)
    {
        if (fgets(tmp, sizeof(tmp), fd) == NULL)
        {
            break;

        }
        else
        {
            list[num_list] = hash(tmp);
            num_list++;
            if (num_list >= MAX_LIST)
            {
                FATAL ("Too many items in file \"sc.debug\"\n");
            }
        }

    }
    fclose(fd);

    /* printf("Num num_list <%d>\n", num_list); */
    return;
}

/*************************************************/

void log_debug_hdr(const char *fmt, ...)
{
	va_list args;

    va_start(args, fmt);

    fprintf(stdout, "DEBUG %s ", ts());
    vfprintf(stdout, fmt, args);
    fflush(stdout);

    va_end(args);
    return;
}

/*************************************************/

void log_debug(const char *fmt, ...)
{
	va_list args;

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);
    return;
}

/*************************************************/

static const char *ts()
{
	static char ts[64];
	time_t t;
	struct tm *tm;

    t = time(NULL);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);
    return ts;
}

/**********************************************/

static uint32_t hash(const char *name)
{
	uint32_t i_hash = 0;

#undef NAME
#define NAME "get_hash()"

    /* This one, although not perfect, is very good */
    for (uint32_t i=0; i<strlen(name); i++)
    {
        if (isprint(name[i]))
        {
            i_hash = (uint8_t)name[i] + 31 * i_hash;
        }
    }

    return i_hash;
}

/**********************************************/

