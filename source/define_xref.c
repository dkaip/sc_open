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

uint32_t cmd_define_xref()
{
	uint32_t iret = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	struct buffer name = INIT_BUFFER;
	struct buffer value1 = INIT_BUFFER;
	struct buffer value2 = INIT_BUFFER;

#undef NAME
#define NAME "cmd_define_xref()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    get_program_token(&name, &type);    
    DEBUG (3, DEBUG_HDR, "Name is <%s>\n", bufdata(&name));

    if ((type == TT_IDENTIFIER) || (type == TT_VARIABLE))
    {
        get_program_token(&token, &type);    
        DEBUG (3, DEBUG_HDR, "Got token <%s>\n", bufdata(&token));
 
        if (type == TT_ASSIGN)
        {
            while (1)
            {
                /* Get the first value */
                iret = expression(&value1);
                if (iret)
                {
                    ERROR ("Error while solving expression in DEFINE_XREF statement\n");
                    break;
                }

                /* Get the comma */
                get_program_token(&token, &type);
                if (type == TT_COMMA)
                {

                }
                else
                {
                    ERROR ("Expecting \",\", found <%s>\n", bufdata(&token));
                    break;
                }

                /* Get the second value */
                iret = expression(&value2);
                if (iret)
                {
                    ERROR ("Error while solving expression in DEFINE_XREF statement\n");
                    break;
                }

                bufcpy(&token, bufdata(&name)); 
                bufcat(&token, "["); 
                bufcat(&token, bufdata(&value1)); 
                bufcat(&token, "]"); 
                variable_put(bufdata(&token), bufdata(&value2), bufsize(&value2), VT_USER_DEFINE);
                
                bufcpy(&token, bufdata(&name)); 
                bufcat(&token, "["); 
                bufcat(&token, bufdata(&value2)); 
                bufcat(&token, "]"); 
                variable_put(bufdata(&token), bufdata(&value1), bufsize(&value1), VT_USER_DEFINE);
                break;
            }

        }
        else
        {
            ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
            iret = 1;
        }
    }
    else
    {
        ERROR ("Expecting IDENTIFIER, or VARIABLE, found token <%s>, type <%s>\n", 
            bufdata(&token), show_type(type));
        iret = 1;
    }
     
    buffree(&token);
    buffree(&name);
    buffree(&value1);
    buffree(&value2);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/


