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

uint32_t cmd_print()
{
	uint32_t iret = 0;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_print()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    iret = expression(&token);
    if (iret) {
        ERROR ("Error with expression in PRINT statement\n");

    } else if (g_syntax_check) {

    } else { 
        LOG ("PRINT %s\n", bufmkprint(&token));
        iret = 0;
    }

    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/


