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

uint32_t cmd_goto()
{
	uint32_t iret = 0;
	uint32_t num = 0;
	struct buffer token = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;

#undef NAME
#define NAME "cmd_goto()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        iret = expression(&token);
        if (iret)
        {
            ERROR ("Error with GOTO expression\n");
            break;
        }

        DEBUG (3, DEBUG_HDR, "Expression returns <%s>\n", bufdata(&token));

        if (variable_get(bufdata(&token), &valu, VT_LABEL))
        {
            ERROR ("Can't find label <%s> in program <%s>\n", 
                bufdata(&token), get_program_name());
            iret = 1;
        }
        else
        {
            if (g_syntax_check)
            {
            	;
            }
            else
            {
                sscanf(bufdata(&valu), "%u", &num);
                set_program_token_idx(num);
            }
        }
        break;
    }

    buffree(&token);
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning\n");
    return iret;
}

