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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "include.h"

#define FN "sc.tune"

static void process_tune(const char *, const char *);

/*************************************************/

void tune()
{
	uint32_t line_number = 0;
	FILE *fp = (FILE *)NULL;
	char *p = (char *)NULL;
	char buf[128], name[128], value[128];

#undef NAME
#define NAME "tune()"

	buf[0] = 0;
	name[0] = 0;
	value[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called\n");

    fp = fopen(FN, "r");
    if (fp == (FILE *)NULL)
    {
        DEBUG (3, DEBUG_HDR, "Tune file <%s> not defined, returning\n", FN);
        /* fclose(fp); Causes linux to dump */ 
        return;
    }

    while (fgets(buf, sizeof(buf), fp) != '\0')
    {
        /* Convert newline to null */
        if ((p = strchr(buf, '\n')) != '\0')
        {
            *p = '\0';
        }

        line_number++;
        if (buf[0] == '#')
        {
            DEBUG (3, DEBUG_HDR, "Skipping comment line <%s>\n", buf);
            continue;
        }
        else if (strlen(buf) == 0)
        {
            DEBUG (3, DEBUG_HDR, "Skipping blank line\n");
            continue;
        }
        else
        {
            if (sscanf(buf, "%s %s", name, value) == 2)
            {
                process_tune(name, value);
            }
            else
            {
                ERROR ("Invalid \"tune\" format, file <%s>, line <%d>\n", FN, line_number);
            } 
        }
    } 

    fclose(fp);
    DEBUG (3, DEBUG_HDR, "Returning\n");
}

/*************************************************/

static void process_tune(const char *name, const char *value)
{

#undef NAME
#define NAME "process_tune()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, value <%s>\n", name, value);

    if (strcmp(name, "POLL_SECS") == 0)
    {
        t_tv_sec = (uint32_t)atoi(value);
        DEBUG (3, DEBUG_HDR, "Setting t_tv_sec <%d>\n", t_tv_sec);

    }
    else if (strcmp(name, "POLL_USECS") == 0)
    {
        t_tv_usec = (uint32_t)atoi(value);
        DEBUG (3, DEBUG_HDR, "Setting t_tv_usec <%d>\n", t_tv_usec);
    }

    return;
}


