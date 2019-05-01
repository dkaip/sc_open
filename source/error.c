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
#include <errno.h>
#include <string.h>
#include "include.h"

static char *ts();

/*************************************************/

void log_error_hdr(const char *fmt, ...)
{
va_list args;

    va_start(args, fmt);

    if (is_pragma(PRAGMA_NO_SOURCE_CODE_LINE_NUMBERS)) {

    } else {
        fprintf(stderr, "%s %s ERROR Detected by ", ts(), g_name);
        vfprintf(stderr, fmt, args);
        fflush(stderr);
    }

    va_end(args);
    return;
}

/*************************************************/

void log_error(const char *fmt, ...)
{
va_list args;

    va_start(args, fmt);

    fprintf(stderr, "%s %s ERROR on line <%s>, ", ts(), g_name, get_line_number());
    vfprintf(stderr, fmt, args);
    if (errno) {
        fprintf(stderr, "%s %s ERROR errno <%d>, <%s>\n", ts(), g_name, errno, strerror(errno));
        errno = 0;
    }
    fflush(stderr);

    fprintf(stdout, "%s %s ERROR on line <%s>, ", ts(), g_name, get_line_number());
    vfprintf(stdout, fmt, args);
    fflush(stdout);

    va_end(args);
    return;
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

