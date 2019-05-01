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

static int for_nest;

/********************************************/

uint32_t cmd_for()
{
	uint32_t iret = 0;
	bool flag = false;
	uint32_t save_program_token_idx = 0;
	int x = 0;
	int y = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	struct buffer name = INIT_BUFFER;
	struct buffer tmp = INIT_BUFFER;
	char num[64], num_nest[64];

#undef NAME
#define NAME "cmd_for()"

	num[0] = 0;
	num_nest[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called\n");

    for_nest++;
    sprintf(num_nest, "%d", for_nest);

    get_program_token(&token, &type);    
    DEBUG (3, DEBUG_HDR, "Got token <%s>\n", bufdata(&token)); 
    bufcpy(&name, bufdata(&token));
    bufcpy(&tmp, bufdata(&token));
    DEBUG (3, DEBUG_HDR, "Name is <%s>\n", bufdata(&name));

    if (type == TT_IDENTIFIER)
    {
        get_program_token(&token, &type);    
        DEBUG (3, DEBUG_HDR, "Got token <%s>\n", bufdata(&token));
 
        if (type == TT_ASSIGN)
        {
            DEBUG (3, DEBUG_HDR, "Calling expression\n");
            iret = expression(&token);
            if (iret)
            {
                ERROR ("Error with expression in FOR <expr> statement");
            }
            else
            {
                sscanf(bufdata(&token), "%d", &x);
                variable_put(bufdata(&name), bufdata(&token), bufsize(&token), VT_USER);

                get_program_token(&token, &type);    
                DEBUG (3, DEBUG_HDR, "Got token <%s>\n", bufdata(&token)); 
                if (type == TT_TO)
                {
                    iret = expression(&token);
                    if (iret)
                    {
                        ERROR ("Error with expression in FOR TO <expr> statement");
                    }
                    else
                    {
                        sscanf(bufdata(&token), "%d", &y);
                        bufcpy(&name, "TO.");
                        bufcat(&name, num_nest);
                        variable_put(bufdata(&name), bufdata(&token), bufsize(&token), VT_SC);

                        get_program_token(&token, &type);    
                        DEBUG (3, DEBUG_HDR, "Got token <%s>\n", bufdata(&token)); 
                        if (type == TT_STEP)
                        {
                            iret = expression(&token);
                            if (iret)
                            {
                                ERROR ("Error with expression in FOR TO STEP <expr> statement");
                            }
                            else
                            {
                            	;
                            }
                        }
                        else
                        {
                            put_program_token();
                            bufcpy(&token, "1");
                        }
                        bufcpy(&name, "STEP.");
                        bufcat(&name, num_nest);
                        variable_put(bufdata(&name), bufdata(&token), bufsize(&token), VT_SC);

                        bufcpy(&name, "FOR.");
                        bufcat(&name, num_nest);
                        sprintf(num, "%d", get_program_token_idx());
                        variable_put(bufdata(&name), num, 0, VT_SC);
                    }
                }
                else
                {
                    ERROR ("Expecting \"TO\", found <%s>\n", bufdata(&token));
                    iret = 1;
                }
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
        ERROR ("Expecting IDENTIFIER, found token <%s>, type <%s>\n", 
            bufdata(&token), show_type(type));
        iret = 1;
    }

    DEBUG (3, DEBUG_HDR, "Check FOR xxx=<%d> TO <%d>\n", x, y);
    if (x > y)
    {
        DEBUG (3, DEBUG_HDR, "Need to skip FOR loop\n");
        flag = false;
        for_nest--;
        save_program_token_idx = get_program_token_idx();
        while (1)
        {
            get_program_token_no_eval(&token, &type);    
            if (type == TT_NULL_TOKEN)
            {
                ERROR ("Can't find end of FOR statement\n");
                iret = 1;
                set_program_token_idx(save_program_token_idx);
                break;
            } 

            if (flag)
            {
                if (strcmp(bufdata(&token), bufdata(&tmp)) == 0)
                {
                    break;
                }
                else
                {
                    flag = false;
                } 
            }
            else if (type == TT_NEXT)
            {
                flag = true;
            }
            else
            {
                flag = false;
            }
        }
    }
    else
    {
        DEBUG (3, DEBUG_HDR, "OK to execute FOR loop atleast once\n");
    }
     
    buffree(&token);
    buffree(&name);
    buffree(&tmp);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t cmd_next()
{
	uint32_t iret = 0;
	int x = 0;
	int y = 0;
	int z = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	struct buffer tmp = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;
	char num[64];
	char num_nest[64];

#undef NAME
#define NAME "cmd_next()"

	num[0] = 0;
	num_nest[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called\n");
    get_program_token(&token, &type);    
    bufcpy(&tmp, bufdata(&token));

    sprintf(num_nest, "%d", for_nest);

    iret = variable_get(bufdata(&token), &valu, VT_USER);
    if (iret)
    {
        ERROR ("Not valid for the FOR NEXT\n", bufdata(&token));
    }
    else if (g_syntax_check)
    {
    	;
    }
    else
    {
        sscanf(bufdata(&valu), "%d", &x);
        bufcpy(&token, "TO.");
        bufcat(&token, num_nest);
        iret = variable_get(bufdata(&token), &valu, VT_SC);
        if (iret)
        {
            FATAL ("Internal error for the FOR NEXT, token <%s>\n", bufdata(&token));
        }
        else
        {
            sscanf(bufdata(&valu), "%d", &y);

            bufcpy(&token, "STEP.");
            bufcat(&token, num_nest);
            iret = variable_get(bufdata(&token), &valu, VT_SC);
            if (iret)
            {
                FATAL ("Internal error for the FOR NEXT, token <%s>\n", bufdata(&token));
            }
            else
            {
                sscanf(bufdata(&valu), "%d", &z);
                x = x + z;
                DEBUG (3, DEBUG_HDR, "Check FOR xxx=<%d> TO <%d>\n", x, y);
                if (x <= y) {
                    sprintf(num, "%d", x);
                    variable_put(bufdata(&tmp), num, 0, VT_USER);

                    bufcpy(&token, "FOR.");
                    bufcat(&token, num_nest);
                    iret = variable_get(bufdata(&token), &valu, VT_SC);
                    if (iret)
                    {
                        FATAL ("Internal error for the FOR NEXT, token <%s>\n", bufdata(&token));
                    }
                    else
                    {
                        sscanf(bufdata(&valu), "%d", &x);
                        set_program_token_idx((uint32_t)x);
                    }
                }
                else
                {
                    for_nest--;
                }
            }
        }
    }

    buffree(&token);
    buffree(&tmp);
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning, error <%d>\n", iret);
    return iret;
}

/********************************************/

void for_reset(bool flag)
{
#undef NAME
#define NAME "for_reset()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    if (flag)
    {
        if (for_nest)
        {
            ERROR ("Missing NEXT for FOR\n");
        }
    }

    for_nest = 0;
    return;
}

/********************************************/

