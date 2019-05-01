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
#include <stdint.h>

#include "include.h"

/********************************************/

void cmd_break_loop()
{
//int /*x,*/ iret = 0;
	uint32_t for_nest = 0;
	uint32_t while_nest = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_break_loop()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        if (g_syntax_check)
        {
            break;
        }

        get_program_token_no_eval(&token, &type);
        DEBUG (3, DEBUG_HDR, "Got token, <%s>, type <%s>\n", bufdata(&token), show_type(type));
        if ((type == TT_NULL_TOKEN) ||
            (type == TT_AFTER) ||
            (type == TT_BEGIN) ||
            (type == TT_END) ||
            (type == TT_LABEL))
        {
            put_program_token();
            break;
        }
        else if (type == TT_FOR)
        {
            for_nest++;
        }
        else if (type == TT_WHILE)
        {
            while_nest++;
        }
        else if (type == TT_NEXT)
        {
            if (for_nest)
            {
                for_nest--;
            }
            else
            {
                /* Skip over the x value as in "next x" */
                get_program_token_no_eval(&token, &type);
                break;
            }
        }
        else if (type == TT_END_WHILE)
        {
            if (while_nest)
            {
                while_nest--;
            }
            else
            {
                while_break();
                break;
            }
        }
        else
        {
        	;
        }
    }

    buffree(&token);
//    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
//    return iret;
    return;
}

/********************************************/


