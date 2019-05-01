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
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "include.h"

static char *ts();
static FILE *logfile_fp;

/*************************************************/

void log_set_fp(const char *name)
{
    logfile_fp = get_logfile_fp(name);
    return;
}

/*************************************************/

void log_clear_fp()
{
    logfile_fp = NULL;
    return;
}

/*************************************************/

void log_msg(const char *fmt, ...)
{
va_list args;

    va_start(args, fmt);
    /* fprintf(stdout, "%s ", ts()); */

    vfprintf(stdout, fmt, args);
    fflush(stdout);

    if (logfile_fp != NULL) {
        vfprintf(logfile_fp, fmt, args);
        fflush(logfile_fp);
    }

    va_end(args);
    return;
}

/*************************************************/

void log_msg_hdr(const char *fmt, ...)
{
va_list args;

    va_start(args, fmt);

    fprintf(stdout, "%s %s ", ts(), g_name);
    vfprintf(stdout, fmt, args);
    fflush(stdout);

    if (logfile_fp != NULL) {
        fprintf(logfile_fp, "%s %s ", ts(), g_name);
        vfprintf(logfile_fp, fmt, args);
        fflush(logfile_fp);
    }

    va_end(args);
    return;
}

/*************************************************/

void log_data(const char *msg, const uint8_t *data, uint32_t size)
{
	char pad[128];

    memset(pad, '\0', sizeof(pad));
    memset(pad, ' ', strlen(ts()));

    fprintf(stdout, "%s %s", pad, msg);
    for (uint32_t i=0; i<size; i++)
    {
        if ((i) && ((i % 16) == 0))
        {
            fprintf(stdout, "\n%s %s", pad, msg);
        }
        fprintf(stdout, "%.2X ", (unsigned char) data[i]);
    }
    fprintf(stdout, "\n");

    if (logfile_fp != NULL)
    {
        fprintf(logfile_fp, "%s %s", pad, msg);
        for (uint32_t i=0; i<size; i++)
        {
            if ((i) && ((i % 16) == 0))
            {
                fprintf(logfile_fp, "\n%s %s", pad, msg);
            }
            fprintf(logfile_fp, "%.2X ", (unsigned char) data[i]);
        }
        fprintf(logfile_fp, "\n");
    }

#ifdef oldcode
    fprintf(stdout, "%s %s", ts(), msg);
    for (i=0; i<size; i++) {
        if ((i) && ((i % 16) == 0)) {
            fprintf(stdout, "\n%s %s", ts(), msg);
        }
        fprintf(stdout, "%.2X ", (unsigned char) data[i]);
    }
    fprintf(stdout, "\n");

    if (logfile_fp != NULL) {
        fprintf(logfile_fp, "%s %s", ts(), msg);
        for (i=0; i<size; i++) {
            if ((i) && ((i % 16) == 0)) {
                fprintf(logfile_fp, "\n%s %s", ts(), msg);
            }
            fprintf(logfile_fp, "%.2X ", (unsigned char) data[i]);
        }
        fprintf(logfile_fp, "\n");
    }
#endif

}

/*************************************************/

static char *ts()
{
static char ts[64];
time_t t;
struct tm *tm;

    t = time(NULL);
    tm = localtime(&t);
    strftime(ts, sizeof(ts), "%Y/%m/%d %H:%M:%S", tm);
    return ts;
}

/*************************************************/

