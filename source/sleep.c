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

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

/********************************************/
// FIXME Is this a bug since wait_flag is only set in a specific condition otherwise it is left unchanged?
uint32_t cmd_sleep(bool *wait_flag)
{
	uint32_t iret = 1;
	uint32_t x = 0;
	struct buffer token = INIT_BUFFER;
	struct buffer name = INIT_BUFFER;

#undef NAME
#define NAME "cmd_sleep()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    DEBUG (3, DEBUG_HDR, "Calling expression\n");
    iret = expression(&token);
    if (g_syntax_check)
    {
    	;
    }
    else if (iret)
    {
        ERROR ("Error with expression in SLEEP statement\n");
    }
    else
    {
        if (sscanf(bufdata(&token), "%u", &x) == 1)
        {
            if (is_pragma(PRAGMA_NO_BLOCK_ON_SLEEP))
            {
// For some reason we are not using timer_idx
//                timer_idx = start_timer(-1, x, get_program_token_idx(), "Resuming", "Sleep is over");
                (void)start_timer(-1, x, get_program_token_idx(), "Resuming", "Sleep is over");
                *wait_flag = true;
            }
            else
            {
                sleep(x);
            }
        }
        else
        {
            iret = 1;
        }
    }

    buffree(&token);
    buffree(&name);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/


