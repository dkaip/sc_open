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

uint32_t cmd_delete()
{
	uint32_t iret = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_delete()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    get_program_token(&token, &type);    
    DEBUG (3, DEBUG_HDR, "Name is <%s>\n", bufdata(&token));

    if ((type == TT_IDENTIFIER) || (type == TT_VARIABLE) || (type == TT_STRING))
    {
        variable_delete(bufdata(&token));
    }
    else
    {
        ERROR ("Expecting IDENTIFIER, or VARIABLE, found token <%s>, type <%s>\n", 
            bufdata(&token), show_type(type));
        iret = 1;
    }
     
    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/


