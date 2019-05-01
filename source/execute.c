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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

extern void log_trace(const char *fmt, ...);
extern uint32_t cmd_continue();
extern uint32_t cmd_define();
extern uint32_t cmd_define_array();
extern uint32_t cmd_define_xref();
extern uint32_t cmd_local_var();
extern uint32_t cmd_delete();

static uint32_t get_error_token(int *error_idx);

static char line_number[32];

/********************************************/

uint32_t execute(struct buffer *token)
{
	int error_idx; /*num,*/
	uint32_t iret = 0;
	bool wait_flag = false;
	enum token_types type = TT_NOT_ASSIGNED_YET;
//int idx_save, idx_tmp, first_time;
	uint32_t idx_tmp = 0;
	bool first_time = true;
	struct buffer valu = INIT_BUFFER;

#undef NAME
#define NAME "execute()"

    DEBUG (3, DEBUG_HDR, "Called\n");
 
//    idx_save = get_program_token_idx();
    iret = 0;
    errno = 0;
    while (1)
    {
        wait_flag = 0;
        errno = 0;
        iret = 0;
        get_program_token(token, &type);
        DEBUG (3, DEBUG_HDR, "Processing token <%s>, type <%s>\n", bufdata(token), show_type(type));

/*
        switch (type) {

        case TT_NULL_TOKEN:
            break;
        }
*/
             
        if (type == TT_NULL_TOKEN)
        {
            break;
        }
        else if (type == TT_LINE_NUMBER)
        {
            strcpy(line_number, bufdata(token));
            /* sscanf(bufdata(token), "%d", &line_number); */
            DEBUG (2, DEBUG_HDR, "Line number <%s>\n", line_number);
            
            if (g_syntax_check && g_debug)
            {
                printf("Line<%s>\n", line_number);
            }

            if (first_time)
            {
                LOG ("Resuming at line number <%s>\n", line_number);
                first_time=0;
            }

            if (g_trace)
            {
                TRACE ("");
                idx_tmp = get_program_token_idx();
                while (1) {
                    if ((type == TT_END_OF_LINE) || (type == TT_NULL_TOKEN))
                    {
                        break;
                    }

                    if (bufsize(token))
                    {
                        fprintf (stdout, "%s ", bufmkprint(token));

                    }
                    else
                    {
                        fprintf (stdout, "%s ", show_type(type));
                    }

                    get_program_token_no_eval(token, &type);
                }
                fprintf (stdout, "\n");
                set_program_token_idx(idx_tmp);
            }
            continue;

        }
        else if (type == TT_END_OF_LINE)
        {
//            idx_save = get_program_token_idx();
            continue;
        }
        else if ((type == TT_AFTER) ||
            (type == TT_BEGIN) ||
            (type == TT_END) ||
            (type == TT_BREAK) ||
            (type == TT_LABEL))
        {
            if (g_syntax_check)
            {
                while (type != TT_END_OF_LINE)
                {
                    get_program_token(token, &type);
                }
                continue;
            }
            else
            {
                break;
            }

        }
        else if (type == TT_BREAK_LOOP)
        {
//            iret = cmd_break_loop();
            cmd_break_loop();
            iret = 0;
        }
        else if (type == TT_CLOSE)
        {
            iret = cmd_close();
        }
        else if (type == TT_CONTINUE)
        {
            iret = cmd_continue();
        }
        else if (type == TT_DEFINE)
        {
            iret = cmd_define();
        }
        else if (type == TT_DEFINE_ARRAY)
        {
            iret = cmd_define_array();
        }
        else if (type == TT_DEFINE_XREF)
        {
            iret = cmd_define_xref();
        }
        else if (type == TT_LOCAL_VAR)
        {
            iret = cmd_local_var();
        }
        else if (type == TT_DELETE)
        {
            iret = cmd_delete();
        }
        else if (type == TT_LET)
        {
            iret = cmd_let();
        }
        else if (type == TT_GLET)
        {
            iret = cmd_glet();
        }
        else if (type == TT_ELSE)
        {
            iret = cmd_else();
        }
        else if (type == TT_ELSE_IF)
        {
            iret = cmd_else_if();
        }
        else if (type == TT_END_IF)
        {
            continue;
        }
        else if (type == TT_END_WHILE)
        {
            iret = cmd_end_while();
        }
        else if (type == TT_ERROR)
        {
            /* Just consume the '=' and the value */
            get_program_token(token, &type);
            get_program_token(token, &type);
            continue;
        }
        else if (type == TT_EXIT)
        {
            exit(0);
        }
        else if (type == TT_FOR)
        {
            iret = cmd_for();
        }
        else if (type == TT_GOSUB)
        {
            iret = cmd_gosub();
        }
        else if (type == TT_GOTO)
        {
            iret = cmd_goto();
        }
        else if (type == TT_IF)
        {
            iret = cmd_if();
        }
        else if (type == TT_NO_ERROR)
        {
            continue;
        }
        else if (type == TT_NEXT)
        {
            iret = cmd_next();
        }
        else if (type == TT_OPEN)
        {
            iret = cmd_open();
        }
        else if (type == TT_PRINT)
        {
            iret = cmd_print();
        }
        else if (type == TT_RETURN)
        {
            iret = cmd_return();
        }
        else if (type == TT_SEND)
        {
            iret = cmd_send(&wait_flag);
        }
        else if (type == TT_SEND_REPLY)
        {
            iret = cmd_send_reply();
        }
        else if (type == TT_SLEEP)
        {
            iret = cmd_sleep(&wait_flag);
        }
        else if (type == TT_START_TIMER)
        {
            iret = cmd_start_timer();
        }
        else if (type == TT_STOP_TIMER)
        {
            iret = cmd_stop_timer();
        }
        else if (type == TT_WHILE)
        {
            iret = cmd_while();
        }
        else if (type == TT_THEN)
        {
            continue;
        }
        else if (type == TT_SET_PROGRAM)
        {
            iret = cmd_set_program();
        }
        else if (type == TT_PRAGMA)
        {
            iret = cmd_pragma();
        }
        else if (is_function(type))
        {
            put_program_token();
            iret = expression(token);
        }
        else
        {
            put_program_token();
            get_program_token(token, &type);
            ERROR ("Invalid command, type <%s>, value <%s>\n", 
                show_type(type), bufdata(token));
            iret = 1;
        }
 
        DEBUG (3, DEBUG_HDR, "Command done, iret <%d>\n", iret);

        if (iret)
        {
            /* gosub_reset(0); */  /* Don't reset here, may be a no_error */
            ERROR ("Error on line <%s>, near >> %s\n", line_number, bufdata(token));

            if (g_syntax_check)
            {
                continue;
            }

            iret = get_error_token(&error_idx);
            if (iret)
            {
                ERROR ("Can't process error\n");
                gosub_reset(0); 
                while_reset(0); 
                for_reset(0); 
                break;
            }
            else
            {
//                if (error_idx == -1)
                if (error_idx < 0)
                {
                    /* There is just no where to go */
                    gosub_reset(0); 
                    while_reset(0); 
                    for_reset(0); 
                    break;
                }
                else
                {
                    /* Continue, probably has a no_error */
                    set_program_token_idx((uint32_t)error_idx);
                }
            }
        }
        DEBUG (3, DEBUG_HDR, "Continuing\n");

        if (wait_flag)
        {
            break;
        }
    }
 
    /* gosub_reset(1); */
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning\n");
    return iret;
}

/********************************************/

static uint32_t get_error_token(int *error_idx)
{
	int num = 0;
	uint32_t iret = 0;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer valu = INIT_BUFFER;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "get_error_token()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    iret = 0;

    get_program_token(&token, &type);
    if (type == TT_ERROR)
    {
        get_program_token(&token, &type);
        if (type == TT_ASSIGN)
        {
            get_program_token(&token, &type);
            if (variable_get(bufdata(&token), &valu, VT_LABEL))
            {
                ERROR ("Can't find error label <%s>, label not found in program <%s>\n",
                    bufdata(&token), get_program_name());
                iret = 1;
            }
            else
            {
                sscanf(bufdata(&valu), "%d", &num);
                DEBUG (3, DEBUG_HDR, "Setting token pointer to <%d>\n", num);
                *error_idx = num;
            }
        }
    } else if (type == TT_NO_ERROR)
    {
        ERROR ("Ignoring error on line <%s>\n", line_number);
        *error_idx = (int)get_program_token_idx();  /* Just continue on */

    }
    else
    {
        /* error=xx not defined for statement */
        put_program_token();

        /* See if the default error is defined */
        if (variable_get("default_error", &valu, VT_LABEL))
        {
            DEBUG (3, DEBUG_HDR, "Default error NOT defined\n");
            *error_idx = -1;  /* No where to go if error happens */

        }
        else
        {
            DEBUG (3, DEBUG_HDR, "Default error defined\n");
            sscanf(bufdata(&valu), "%d", &num);
            DEBUG (3, DEBUG_HDR, "Setting token pointer to <%d>\n", num);
            *error_idx = num;
        }
    }

    buffree(&token);
    buffree(&valu);
    DEBUG (3, DEBUG_HDR, "Returning ret <%d>, error_idx\n", iret, error_idx);
    return iret;
}

/********************************************/

const char *get_line_number()
{
#undef NAME
#define NAME "get_line_number()"

    return line_number;

}

/********************************************/

void set_line_number(const char *ln)
{
#undef NAME
#define NAME "set_line_number()"

    strcpy(line_number, ln);

}


