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
#include <stdbool.h>

#include "include.h"

extern void log_trace(const char *fmt, ...);

static int while_nest = 0;

/********************************************/

uint32_t cmd_while()
{
	uint32_t iret = 0;
	uint32_t nest = 0;
	uint32_t tmp_idx = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	struct buffer name = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;
	char num[64];

#undef NAME
#define NAME "cmd_while()"

	num[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called\n");

    bufcpy(&name, "WHILE.");
    while_nest++;
    sprintf(num, "%d", while_nest);
    bufcat(&name, num);

    put_program_token();  /* Need to backup one to include the "while" */ 
    tmp_idx = get_program_token_idx();
    sprintf(num, "%d", tmp_idx);
    bufcpy(&valu, num);
    variable_put(bufdata(&name), num, 0, VT_SC);

    get_program_token(&token, &type);  /* Now skip over the "while" before calling expression() */
    iret = expression(&token);
    if (iret)
    {
        ERROR ("Error with WHILE expression\n");
    } else if (g_syntax_check)
    {
    	;
    }
    else if (atoi(bufdata(&token)))
    {
        TRACE ("    WHILE statement is TRUE\n");
    }
    else
    {
        TRACE ("    WHILE statement is FALSE\n");
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
            else if (type == TT_WHILE)
            {
                nest++;
            }
            else if (type == TT_END_WHILE)
            {
                if (nest)
                {
                    nest--;
                }
                else
                {
                    while_nest--;
                    break;
                }
            }
        }
    }

    buffree(&token);
    buffree(&name);
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t cmd_end_while()
{
	char num[64];
	int x = 0;
	uint32_t iret = 0;
	struct buffer name = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;

#undef NAME
#define NAME "cmd_end_while()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    if (while_nest < 1)
    {
        ERROR ("END_WHILE not valid here\n");
    }
    else if (g_syntax_check)
    {
    	;
    }
    else
    {
        /* Lookup the WHILE.xx variable to get the program idx */
        bufcpy(&name, "WHILE.");
        sprintf(num, "%d", while_nest);
        bufcat(&name, num);
        iret = variable_get(bufdata(&name), &valu, VT_SC);
        if (iret)
        {
            ERROR ("No END_WHILE information found\n");
        }
        else
        {
            sscanf(bufdata(&valu), "%d", &x);
            set_program_token_idx((uint32_t)x);
            while_nest--; 
        }
    }

    buffree(&name);
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

void while_reset(bool flag)
{
#undef NAME
#define NAME "while_reset()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    if (flag == true)
    {
        if (while_nest)
        {
            ERROR ("Missing END_WHILE for WHILE\n");
        }
    }

    while_nest = 0;
    return;
}

/********************************************/

void while_break()
{
#undef NAME
#define NAME "while_break()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while_nest--; 
    return;
}

/********************************************/

