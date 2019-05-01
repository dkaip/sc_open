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
#include <stdbool.h>

#include "include.h"

extern void log_trace(const char *fmt, ...);

static int nest = 0;

/********************************************/

uint32_t cmd_gosub()
{
	uint32_t iret = 0;
	uint32_t length = 0;
	uint32_t arg_cnt = 0;
	uint8_t *ptr = (uint8_t *)NULL;
	uint32_t gosub_idx = 0;
	uint32_t tmp_idx = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	enum token_types qtype = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	struct buffer name = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;
	struct binary_buffer stack = INIT_BINARY_BUFFER;
	char num[64];

#undef NAME
#define NAME "cmd_gosub()"

	num[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called\n");

    get_program_token(&token, &type);    

    iret = variable_get(bufdata(&token), &valu, VT_LABEL);
    if (iret)
    {
        ERROR ("No label <%s> found in program <%s> for GOSUB statement\n",
            bufdata(&token), get_program_name());
    }
    else
    {
        sscanf(bufdata(&valu), "%u", &gosub_idx);
    }
        
    TRACE ("Gosub to label <%s>\n", bufdata(&token));

#ifdef oldcode
    if (g_syntax_check) {
        arg_cnt = 0;

    } else {
#endif
       
    /* Queue up all the arguments */
    /* The queue is organized as: <type><length><data> */

    arg_cnt = 0;
    while (1)
    {
        get_program_token(&token, &type);    
        if (type == TT_END_OF_LINE)
        {
            break;
        }
        else if (type == TT_COMMA)
        {
            /* Ignore these */
        	;
        }
        else if (type == TT_IDENTIFIER)
        {
            iret = variable_get(bufdata(&token), &valu, VT_USER_REF);  /* Get by reference */
            if (iret)
            {
                ERROR ("Identifier <%s> needs to be defined before being used in GOSUB\n", 
                    bufdata(&token));
                break;
            }

            type = TT_IDENTIFIER;
        	/*
        	 * This is what a string looks like in a program...must be a string
        	 */
            binary_bufncat(&stack, ubyte_type(type), sizeof(type));
            length = bufsize(&valu);
            /* printf("??? Queing type <%d>, length <%d>, data <%s>\n", 
                type, length, bufdata(&valu)); */
            binary_bufncat(&stack, (uint8_t *) &length, sizeof(length));
            binary_bufncat(&stack, (uint8_t *)bufdata(&valu), length);
            arg_cnt++;
   
        }
        else
        {
        	/*
        	 * This is what a string looks like in a program...must be a string
        	 */
        	binary_bufncat(&stack, ubyte_type(type), sizeof(type));
            length = bufsize(&token);
            binary_bufncat(&stack, (uint8_t *)&length, sizeof(length));
            binary_bufncat(&stack, (uint8_t *)bufdata(&token), length);
            /* printf("??? Queing type <%d>, length <%d>, data <%s>\n", 
                type, length, bufdata(&token)); */
            arg_cnt++;
        }
    }

    bufcpy(&name, "RETURN."); 
    nest++;
    sprintf(num, "%d", nest);
    bufcat(&name, num);

    tmp_idx = get_program_token_idx();  /* I use tmp_idx later too */
    sprintf(num, "%d", tmp_idx);
    bufcpy(&valu, num);
    variable_put(bufdata(&name), num, 0, VT_SC);

    DEBUG (3, DEBUG_HDR, "Setting token pointer\n");
    set_program_token_idx(gosub_idx);

    /* Now dequeue the arguments */

    ptr = binary_buffer_data(&stack);
    while (1)
    {
        get_program_token(&name, &type);    

        /* printf("??? type <%s>\n", show_type(type)); */

        if (type == TT_END_OF_LINE)
        {
            break;
        }
        else if (type == TT_COMMA)
        {
            /* Ignore these */
        	;
        }
        else if (type == TT_IDENTIFIER)
        {
            if (arg_cnt == 0)
            {
                ERROR ("Too few arguments for GOSUB\n");
                iret = 1;
                break;
            }

            arg_cnt--;
            qtype = (enum token_types)*ptr;
            ptr++;
//            memcpy(&qtype, ptr, sizeof(qtype));
//            ptr+=sizeof(qtype);
            memcpy(&length, ptr, sizeof(length)); 
            ptr+=sizeof(length);
            bufncpy(&valu, (char *)ptr, length);
            ptr+=length;
            /* printf("??? DeQueing qtype <%d>, length <%d>, data <%s>\n", 
                qtype, length, bufdata(&valu)); */
        
            if (qtype == TT_IDENTIFIER)
            {
                variable_put(bufdata(&name), bufdata(&valu), length, VT_USER_REF);

            }
            else
            {
                variable_put(bufdata(&name), bufdata(&valu), length, VT_USER_LOCAL);
            }

        }
        else
        {
            ERROR ("Expecting IDENTIFIER, found <%s> in target for GOSUB\n", 
                show_type(type));
            iret = 1;
            arg_cnt = 0;  /* Set so won't generate ERROR below */
            break;
        }

    }

    if (arg_cnt > 0)
    {
        ERROR ("Too many arguments for GOSUB\n");
        iret = 1;
    }

    if (g_syntax_check)
    {
        set_program_token_idx(tmp_idx);
    }

    buffree(&token);
    buffree(&name);
    buffree(&valu);
    binary_buffree(&stack);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t cmd_return()
{
	uint32_t iret = 0;
	int x = 0;;
	struct buffer name = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;
	char num[64];

#undef NAME
#define NAME "cmd_return()"

	num[0] = 0;
	DEBUG (3, DEBUG_HDR, "Called\n");

    if (g_syntax_check)
    {
    	;
    }
    else if (nest < 1)
    {
        ERROR ("RETURN not valid here\n");
    }
    else
    {
        /* Lookup the RETURN.xx variable to get the program idx */
        bufcpy(&name, "RETURN."); 
        sprintf(num, "%d", nest);
        bufcat(&name, num);
        iret = variable_get(bufdata(&name), &valu, VT_SC);
        if (iret)
        {
            ERROR ("No RETURN information found\n");
        }
        else
        {
            sscanf(bufdata(&valu), "%d", &x);
            set_program_token_idx((uint32_t)x);
            nest--;
        }
    }

    buffree(&name);
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

void gosub_reset(bool flag)
{
#undef NAME
#define NAME "gosub_reset()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    if (flag)
    {
        if (nest)
        {
            ERROR ("Missing RETURN for GOSUB\n");
        }
    }

    nest = 0;
    return;
}

/********************************************/

