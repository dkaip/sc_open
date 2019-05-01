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

#include <string.h>
#include <stdint.h>

#include "include.h"

static int pragma[PRAGMA_MAX] = {0,0,0,0};

/* The order of these below must match the order as defined in pragma.h */

static const char *str[PRAGMA_MAX] =
{
    "LOG_CPU_USAGE", 
    "SHOW_NON_PRINTING_CHARS_IN_BRACES",
    "NO_BLOCK_ON_SLEEP",
    "NO_SOURCE_CODE_LINE_NUMBERS"
};

/********************************************/

uint32_t cmd_pragma()
{
	int i = 0;
	uint32_t iret = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_pragma()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        get_program_token(&token, &type);    

        if (type == TT_END_OF_LINE)
        {
            break;
        }

        for (i=0; i<PRAGMA_MAX; i++)
        {
            if (strcmp(bufdata(&token), str[i]) == 0)
            {
                pragma[i] = 1;
                break;
            }
        }

        if (i >= PRAGMA_MAX)
        {
            ERROR ("Invalid pragma <%s>\n", bufdata(&token));
            iret = 1;
            break;
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    buffree(&token);
    return iret;

}

/********************************************/

void dump_pragma(struct buffer *buffer)
{
	char buf[512];

#undef NAME
#define NAME "dump_server()"

    bufcat(buffer, "Pragmas (0=Off, 1=On):\n");

    for (uint32_t i=0; i<PRAGMA_MAX; i++)
    {
        sprintf(buf, "  %s=<%d>\n", str[i], pragma[i]); 
        bufcat(buffer, buf);
    }

    return;
}

/********************************************/

int is_pragma(int code)
{
#undef NAME
#define NAME "is_pragma()"

    return pragma[code];
}

