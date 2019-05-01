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

#include <stdlib.h>
#include <stdint.h>

#include "include.h"

extern void log_trace(const char *fmt, ...);

/********************************************/

uint32_t cmd_if()
{
	uint32_t iret = 0;
	uint32_t nest = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_if()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    iret = expression(&token);
    if (iret)
    {
        ERROR ("Error with IF expression\n");
    }
    else if (g_syntax_check)
    {
    	;
    }
    else if (atoi(bufdata(&token)))
    {
        TRACE ("    IF statement is TRUE\n");
    }
    else
    {
        TRACE ("    IF statement is FALSE\n");
        nest = 0;
        while (1)
        {
            get_program_token_no_eval(&token, &type);
            DEBUG (3, DEBUG_HDR, "Got token, <%s>, type <%s>\n", bufdata(&token), show_type(type));
            if (type == TT_NULL_TOKEN)
            {
                ERROR ("Found NULL token while processing ELSE\n");
                iret = 1;
                break;
            }
            else if (type == TT_IF)
            {
                nest++;
            }
            else if (type == TT_ELSE)
            {
                if (nest)
                {
                    ;/* nest--; */
                }
                else
                {
                    break;
                }
            }
            else if (type == TT_ELSE_IF)
            {
                if (nest)
                {
                    ;/* nest--; */
                }
                else
                {
                    cmd_if();
                    break;
                }
            }
            else if (type == TT_END_IF)
            {
                if (nest)
                {
                    nest--;
                }
                else
                {
                    break;
                }
            }
        }
    }

    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t cmd_else()
{
	uint32_t iret = 0;
	uint32_t nest = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_else()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        get_program_token_no_eval(&token, &type);
        if (type == TT_NULL_TOKEN)
        {
            ERROR ("Found NULL token while processing ELSE\n");
            iret = 1;
            break;
        }
        else if (type == TT_IF)
        {
            nest++;
        }
        else if (type == TT_END_IF)
        {
            if (nest)
            {
                nest--;
            }
            else
            {
                break;
            }
        }
    }

    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return 0;
}

/********************************************/

uint32_t cmd_else_if()
{
	uint32_t iret = 0;
	uint32_t nest = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_else_if()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        get_program_token_no_eval(&token, &type);
        if (type == TT_NULL_TOKEN)
        {
            ERROR ("Found NULL token while processing ELSE\n");
            iret = 1;
            break;
        }
        else if (type == TT_IF)
        {
            nest++;
        }
        else if (type == TT_END_IF)
        {
            if (nest)
            {
                nest--;
            }
            else
            {
                break;
            }
        }
    }

    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return 0;
}


