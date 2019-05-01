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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

/*
 * The program is stored in binary format, but, tokens are always null
 * terminated strings.
 *
 *
 * 		case TT_NUMBER:
 *		case TT_CONSTANT:
 *		case TT_IDENTIFIER:
 *		case TT_VARIABLE:
 *		case TT_LINE_NUMBER:
 *
 * The above "tokens" when stored in the program are in the form of:
 *     The first byte is the token type value (see the token_type enum).
 *     The second byte is the beginning of a null terminated string with the information in it
 *
 *     case TT_STRING:
 *
 * The above "token" when stored in the program are in the form of:
 *     The first byte is the token type value (see the token_type enum).
 *     The next 4 bytes contain the number of characters in the string
 *     The subsequent bytes are the characters in the string...it is NOT null terminated
 */

#define UNDEFINED_PROGRAM "undefined"  /* "begin" with no name */ 
#define DEFAULT_PROGRAM   ""           /* Before any "begin" */

extern uint32_t label_build_idx_table();

static bool skip_token(char);
static uint32_t make_labels();
static uint32_t decode_program_token(const uint8_t *ptr, struct buffer *token, enum token_types *type);
/*static char *trim(char *);*/

static struct buffer current_program_name = INIT_BUFFER;  /* The name of the current program running */
static struct binary_buffer program = INIT_BINARY_BUFFER;
static uint32_t program_idx = 0;
static uint32_t saved_program_idx = 0;
static void read_it(char *, char *, char *, bool, bool);
static bool is_file(char *);
/* static FILE *pp_fd; */

/********************************************/

uint32_t read_program(const char *buf)
{
	enum parse_types parse_type = PT_NOT_ASSIGNED_YET;
	uint32_t iret = 0;
	uint32_t len = 0;
	bool pp_flag = 0;
	int num_include = 0;
	char include_str[32];
	char *file = (char *)NULL;
	struct buffer token = INIT_BUFFER;
	struct buffer tmp = INIT_BUFFER;

#undef NAME
#define NAME "read_program()"

	include_str[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called, buf <%s>\n", buf);

//    pp_flag = esc = iret = 0;

    len = parse(&token, buf, &parse_type, "-", false, false);
    buf += len;
    file = "program";  /* Default program name */
//    num_include = 0;

    /* Set the current program name */
    bufcpy(&current_program_name, DEFAULT_PROGRAM);

    /* Delete all the variables */
    variable_delete_all();

    /* Handle all the switches here */

    while (parse_type != PT_END_OF_LINE)
    {
        DEBUG (3, DEBUG_HDR, "Arg <%s>\n", bufdata(&token));

        if (bufcmp(&token, "-I") == 0)
        {
            sprintf(include_str, "#include[%d]", num_include);
            num_include++;
            len = parse(&token, buf, &parse_type, "/", false, false);
            buf+=len;
            if (bufsize(&token) == 0)
            {
                PP_ERROR ("Missing -I argument\n");
            }

            variable_put(include_str, bufdata(&token), 0, VT_SC);
            DEBUG (3, DEBUG_HDR, "-I arg <%s>, <%s>\n", bufdata(&token), bufdata(&tmp));
            LOG ("-I include path <%s>\n", bufdata(&token));
            pp_flag = true;
        }
        else if (bufcmp(&token, "-D") == 0)
        {
            /* Can be -D XXX or -D XXX=<value> */
            len = parse(&token, buf, &parse_type, "", false, false);
            buf+=len;
            bufcpy(&tmp, bufdata(&token));  /* tmp will be the name */
            DEBUG (3, DEBUG_HDR, "-D arg <%s>\n", bufdata(&tmp));
            if (bufsize(&tmp) == 0)
            {
                PP_ERROR ("Missing -D argument\n");
            }

            len = parse(&token, buf, &parse_type, "", false, false);
            if (*bufdata(&token) == '=')
            {
                /* Format is -D XXX=<value> */
                buf+=len;
                len = parse(&token, buf, &parse_type, "", false, false);
                buf+=len;
                variable_put(bufdata(&tmp), bufdata(&token), 0, VT_SC);
                LOG ("-D defined <%s>=<%s>\n", bufdata(&tmp), bufdata(&token));
            }
            else
            {
                /* Format is -D XXX */
                LOG ("-D defined <%s>\n", bufdata(&tmp));
                variable_put(bufdata(&tmp), "", 0, VT_SC);
            }
            pp_flag = true;

        }
        else if (bufcmp(&token, "-P") == 0)
        {
            DEBUG (3, DEBUG_HDR, "-P arg, pp_flag set\n");
            pp_flag = true;
        }
        else
        {
            DEBUG (3, DEBUG_HDR, "Program file <%s>\n", bufdata(&token));
            file = bufdata(&token);
            break;
        }

        len = parse(&token, buf, &parse_type, "-", false, false);
        buf+=len;
    }

    if (pp_flag)
    {
        LOG ("-P preprocessor turned on\n");
    }
    else
    {
        LOG ("-P preprocessor turned off\n");
    }

    DEBUG (3, DEBUG_HDR, "Program file <%s>\n", file);

    /* Init the program */
//    bufcpy(&program, "");
    binary_buffree(&program);

    read_it(file, "", "", pp_flag, true);

    iret = make_labels();
    if (iret)
    {
        ERROR ("Can't create all of the labels for program\n");
    }

    label_build_idx_table();

    if (g_syntax_check)
    {
    	;
    }
    else
    {
        program_idx = 0;
        iret = execute(&token);
    }

    buffree(&token);
    buffree(&tmp);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

static uint32_t make_labels()
{
	struct buffer token = INIT_BUFFER;
	struct buffer x = INIT_BUFFER;
	struct buffer y = INIT_BUFFER;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	uint32_t iret = 0;
	char num[64];

#undef NAME
#define NAME "make_labels()"

	num[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called\n");
    bufcpy(&current_program_name , DEFAULT_PROGRAM);
    program_idx = 0;

    while (1)
    {
        get_program_token_no_eval(&token, &type);
        /* DEBUG (3, DEBUG_HDR, "Token <%s>, <%s>\n", bufdata(&token), show_type(type)); */
        if (type == TT_NULL_TOKEN)
        {
            DEBUG (3, DEBUG_HDR, "Breaking\n");
            break;
        }
        else if (type == TT_LINE_NUMBER)
        {
            set_line_number(bufdata(&token));
        }
        else if (type == TT_BEGIN)
        {
            get_program_token(&token, &type);
            if (type == TT_END_OF_LINE)
            {
                bufcpy(&current_program_name, UNDEFINED_PROGRAM);
            }
            else if (strcmp(bufdata(&current_program_name), DEFAULT_PROGRAM) == 0)
            {
                bufcpy(&current_program_name, bufdata(&token));
            }
            else
            {
                ERROR ("Found BEGIN before END of <%s> at line <%d>\n", 
                    bufdata(&current_program_name), get_line_number());
                iret = 1;
                break;
            }

            variable_put("", "DEFINED", 0, VT_SC);

            sprintf(num, "%d", get_program_token_idx());
            variable_put("BEGIN", num, 0, VT_SC);
        }
        else if (type == TT_END)
        {
            bufcpy(&current_program_name, DEFAULT_PROGRAM);
        }
        else if ((type == TT_AFTER) || (type == TT_LABEL))
        {
        	// TODO this test is invalid since it can never happen because of the previous test
            if (type == TT_END_OF_LINE)
            {
                ERROR ("Missing \"value\" following AFTER or LABEL at line <%s>\n", get_line_number());
                iret = 1;
                break;
            }
            else
            {
                get_program_token(&token, &type);
                if (type == TT_IDENTIFIER)
                {
                    sprintf(num, "%d", get_program_token_idx());
                    bufcpy(&x, bufdata(&token));
                    variable_put(bufdata(&x), num, 0, VT_LABEL);
                }
                else
                {
                    ERROR ("Expecting IDENTIFIER, found <%s>, type <%s> at line <%s>\n", 
                        bufdata(&token), show_type(type), get_line_number());
                    iret = 1;
                    break;
                }
            }
        }
    }

    bufcpy(&current_program_name, DEFAULT_PROGRAM);
    buffree(&token);
    buffree(&x);
    buffree(&y);
    return iret;
}

/********************************************/

char *get_program_name()
{
#undef NAME
#define NAME "get_program_name()"

    return bufdata(&current_program_name);
}

/********************************************/

char *get_default_program_name()
{
#undef NAME
#define NAME "get_default_program_name()"

    return DEFAULT_PROGRAM;
}

/********************************************/

uint32_t cmd_set_program()
{
	enum token_types type = TT_NOT_ASSIGNED_YET;
	uint32_t num = 0;
	uint32_t iret = 0;
	struct buffer token = INIT_BUFFER;
	struct buffer tmp = INIT_BUFFER;
	struct buffer valu = INIT_BUFFER;

#undef NAME
#define NAME "cmd_set_program()"

    DEBUG (3, DEBUG_HDR, "Called, <%s>\n", bufdata(&token));

    iret = 0;
    get_program_token(&token, &type);
    if (type == TT_END_OF_LINE)
    {
        bufcpy(&token, UNDEFINED_PROGRAM);
    }

    bufcpy(&tmp, bufdata(&current_program_name));
    variable_put("", "INACTIVE", 0, VT_SC);

    bufcpy(&current_program_name, bufdata(&token));

    iret = variable_get("BEGIN", &valu, VT_SC);
    if (iret)
    {
        ERROR ("Not a valid program <%s>\n", bufdata(&token)); 
        bufcpy(&current_program_name, bufdata(&tmp));
        variable_put("", "ACTIVE", 0, VT_SC);
        iret = 1;
    }
    else
    {
        sscanf(bufdata(&valu), "%u", &num);
        DEBUG (3, DEBUG_HDR, "Setting token pointer\n");
        set_program_token_idx(num);

        variable_put("", "ACTIVE", 0, VT_SC);
        LOG ("Current program set to <%s>\n", get_program_name());
    }

    buffree(&token);
    buffree(&tmp);
    buffree(&valu);
    return iret;
}

/********************************************/

void list_program(struct buffer *buffer)
{
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	uint32_t num = 0;
	uint8_t *ptr = (uint8_t *)NULL;
	uint8_t *end = (uint8_t *)NULL;

#undef NAME
#define NAME "list_program()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    bufcpy(buffer, "");
    ptr = binary_buffer_data(&program);
    end = ptr + binary_buffer_num_data(&program);

    while (ptr < end)
    {
        num = decode_program_token(ptr, &token, &type);
        ptr += num;

        if (type == TT_LINE_NUMBER)
        {
            bufncat(buffer, bufdata(&token), bufsize(&token)); 

        }
        else if (type == TT_END_OF_LINE)
        {
            bufcat(buffer, "\n");
        }
        else if (type == TT_STRING)
        {
            bufcat(buffer, "\""); 
            bufncat(buffer, bufdata(&token), bufsize(&token)); 
            bufcat(buffer, "\""); 

        }
        else if (type == TT_CONSTANT)
        {
            bufcat(buffer, "'"); 
            bufncat(buffer, bufdata(&token), bufsize(&token)); 
            bufcat(buffer, "'"); 
        }
        else if (bufsize(&token) > 0)
        {
            bufncat(buffer, bufdata(&token), bufsize(&token)); 
        }
        else
        {
            bufcat(buffer, show_type(type)); 
        }
        bufcat(buffer, " "); 
    }

    buffree(&token);
    return;
}

/********************************************/

void dump_program_tokens(struct buffer *buffer)
{
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	uint32_t num = 0;
	uint32_t token_location = 0;
	uint8_t *ptr = (uint8_t *)NULL;
	uint8_t *end = (uint8_t *)NULL;

#undef NAME
#define NAME "dump_program_tokens()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    bufcpy(buffer, "");
    ptr = binary_buffer_data(&program);
    end = ptr + binary_buffer_num_data(&program);

    while (ptr < end)
    {
        num = decode_program_token(ptr, &token, &type);
        ptr += num;
        bufcat(buffer, "<Loc ");
        bufcat(buffer, my_itoa(token_location));
        token_location += num;
        bufcat(buffer, ">");
        bufcat(buffer, "<"); 
        bufcat(buffer, show_type(type)); 
        bufcat(buffer, ">"); 
        bufncat(buffer, bufdata(&token), bufsize(&token)); 
        if (type == TT_END_OF_LINE)
        {
            bufcat(buffer, "\n");
        }
    }
    buffree(&token);
    return;
}

/*************************************************/

void get_program_token(struct buffer *token, enum token_types *type)
{
	uint32_t num = 0;
	uint8_t *ptr = (uint8_t *)NULL;

#undef NAME
#define NAME "get_program_token()"

    saved_program_idx = program_idx;
    ptr = &binary_buffer_data(&program)[program_idx];

    if (program_idx >= binary_buffer_num_data(&program))
    {
        *type = TT_NULL_TOKEN;
        bufcpy(token, "");
    }
    else
    {
        num = decode_program_token(ptr, token, type);
        program_idx = program_idx + num;
    }

    DEBUG (3, DEBUG_HDR, "Before evaluate token, token <%s>, type <%s>\n", bufdata(token), show_type(*type));
    if (*type == TT_VARIABLE)
    {
        evaluate_token(token);
    }

    DEBUG (3, DEBUG_HDR, "Returning token <%s>, type <%s>\n", bufdata(token), show_type(*type));
    return;
}

/*************************************************/

void get_program_token_no_eval(struct buffer *token, enum token_types *type)
{
	uint32_t num = 0;
	uint8_t *ptr = (uint8_t *)NULL;

#undef NAME
#define NAME "get_program_token_no_eval()"

    saved_program_idx = program_idx;
    ptr = &binary_buffer_data(&program)[program_idx];

    if (program_idx >= binary_buffer_num_data(&program))
    {
        *type = TT_NULL_TOKEN;
        bufcpy(token, "");
    }
    else
    {
        num = decode_program_token(ptr, token, type);
        program_idx = program_idx + num;
    }

    DEBUG (3, DEBUG_HDR, "Returning token <%s>, type <%s>\n", bufdata(token), show_type(*type));
    return;
}

/*************************************************/

void put_program_token()
{
#undef NAME
#define NAME "put_program_token()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    program_idx = saved_program_idx;
    return;

}

/*************************************************/

uint32_t get_program_token_idx()
{
#undef NAME
#define NAME "get_program_token_idx()"

    DEBUG (3, DEBUG_HDR, "Called, Returning <%d>\n", program_idx);

    return program_idx;
}

/********************************************/

void set_program_token_idx(uint32_t idx)
{
#undef NAME
#define NAME "set_program_token_idx()"

// This should not be an issue anymore since idx is now a uint32_t.
//    if (idx == -1)
//    {
//        FATAL ("Received negative index\n");
//    }

    program_idx = idx;
    DEBUG (3, DEBUG_HDR, "Set idx to <%d>\n", program_idx);
    return;
}

/*************************************************/

/*
 * Returns, modifies token, and type
 */
static uint32_t decode_program_token(const uint8_t *ptr, struct buffer *token, enum token_types *type)
{
	uint32_t length = 0;
	uint32_t num    = 0;

#undef NAME
#define NAME "decode_program_token()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    num = 0;  /* Returns number of bytes consumed */
    bufcpy(token, "");
    *type = (uint8_t)*ptr;
    ptr++;
    num++;

    /* Work with the token value */
    switch (*type)
    {
		case TT_NUMBER:
		case TT_CONSTANT:
		case TT_IDENTIFIER:
		case TT_VARIABLE:
		case TT_LINE_NUMBER:
			/* These tokens are null terminated strings */
			bufcat(token, (char *)ptr);
			num += (uint32_t)strlen((char *)ptr) + 1;
			break;

		case TT_STRING:
			// TODO check to see if int is appropos
			/* These tokens use an int to define the length */
			memcpy(&length, ptr, sizeof(length));
			ptr += sizeof(length);
			bufncat(token, (char *)ptr, length);
			num = num + (uint32_t)sizeof(length) + length;
			break;

		default:
			/* These tokens have no length */
			break;
    }

    return num;
}

/*************************************************/

static void read_it(char *file_in, char *buf, char *macro_name, bool pp_flag, bool first_time_flag)
{
	enum parse_types parse_type = PT_NOT_ASSIGNED_YET;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	int esc;
	uint32_t len = 0;
	uint32_t iret = 0;
	uint32_t line_number = 0;
	uint32_t num_args = 0;
	uint32_t i = 0;
	char *p, *save_token_ptr, include_str[32], num_str[32], num[128+16], file[128], macro_arg[32];
	struct buffer token= INIT_BUFFER;
	struct buffer source = INIT_BUFFER;
	struct buffer tmp = INIT_BUFFER;
	struct buffer tmp2 = INIT_BUFFER;
	struct buffer name = INIT_BUFFER;
	struct buffer value = INIT_BUFFER;

#undef NAME
#define NAME "read_it()"

    DEBUG (3, DEBUG_HDR, "Called, file <%s>, buf <%s>, macro name <%s>, pp_flag <%d>, first_time <%d>\n", 
        file_in, buf, macro_name, pp_flag, first_time_flag);

    /* Figure out what file to read, use the -I <include> too */

    if (strlen(file_in))
    {
        strncpy(file, file_in, sizeof(file) - 1);
        if (first_time_flag)
        {
            /* Program file, not an include file */
            if(buffilecpyread(&source, file))
            {
                PP_ERROR ("Can't open program file <%s>\n", file); 
            }
        }
        else
        {
            /* Include file, Look here first */
            if (is_file(file))
            {
                if(buffilecpyread(&source, file))
                {
                   PP_ERROR ("Can't open program file <%s>\n", file); 
                }
            }
            else
            {
                /* Look elsewheres, use the include paths */
                /* Include paths are stored like "include.<path>" */
                i = 0;  
                while (1)
                {
                    sprintf(include_str, "#include[%d]", i);
                    i++;
                    iret = variable_get(include_str, &token, VT_SC);
                    if (iret)
                    {
                        PP_ERROR ("Can't open include file <%s>\n", file); 

                    }
                    else
                    {
                        bufcat(&token, "/");
                        bufcat(&token, file);
                        if (is_file(bufdata(&token))) {
                            if(buffilecpyread(&source, bufdata(&token)))
                            {
                                PP_ERROR ("Can't open include file <%s>\n", bufdata(&token)); 
                            }
                            else
                            {
                                strncpy(file, bufdata(&token), sizeof(file) - 1);
                                break;  /* Done, so quit */
                            }
                        }
                    }
                    i++;  /* Inc for the the next include path */
                }
            }
        }
        DEBUG (4, DEBUG_HDR, "Actual file used <%s>\n", file);

        DEBUG (4, DEBUG_HDR, "Saving first line number\n");
        line_number = 1;
        type = TT_LINE_NUMBER;
        DEBUG (4, DEBUG_HDR, "Saving token type <%s> \n", show_type(type));
        binary_bufncat(&program, ubyte_type(type), 1);  /* bufncat(&program, (char *) &type, 1); */
        sprintf(num, "%s:%d", file, line_number);
        DEBUG (4, DEBUG_HDR, "First time, writing line_number <%d>, <%s>\n", line_number, num);
        DEBUG (4, DEBUG_HDR, "Saving token <%s> \n", num);
        binary_bufncat(&program, (uint8_t *)num, (uint32_t)strlen(num) + 1);  // Make sure to get the null
//        binary_bufncat(&program, "", 1);

        if (!first_time_flag)
        {
            LOG ("Line <%s>, Including file <%s>\n", get_line_number(), file);
        } 

    }
    else if (strlen(buf))
    {
        /* Used what was passed in the buffer */
        bufcpy(&source, buf);  
    }
    else
    {
        PP_ERROR ("Missing source filename, or defined value\n");
    }

    set_token(&source);

    esc = 0;

    /* Start creating and saving tokens from sc source code... stick them in the program buffer */

    while (1)
    {
        get_token(&token, &type, pp_flag);
        DEBUG (5, DEBUG_HDR, "Next token <%s>, type <%s>\n", 
            bufmkprint(&token), show_type(type));
 
        if (type == TT_NULL_TOKEN)
        {
            DEBUG (5, DEBUG_HDR, "Null token, breaking...\n");
            break;
        }

        if (esc)
        {
            DEBUG (5, DEBUG_HDR, "Handling esc flag\n");
            esc = 0;
            if (type == TT_END_OF_LINE)
            {
                line_number++;
                DEBUG (5, DEBUG_HDR, "TT_END_OF_LINE, inc line_number <%d>\n", line_number);
                continue;
            }
        }

        /* Check for the esc char */
        if (type == TT_ESC)
        {
            DEBUG (5, DEBUG_HDR, "Setting esc flag\n");
            esc = 1;
            continue;
        }

        /* Possibility of new-lines within data... need to adjust the line count */
        if ((type == TT_STRING) || (type == TT_POUND_DEFINE) || (type == TT_COMMENT))
        {
            p = bufdata(&token);
            while (*p != '\0')
            {
                if (*p == '\n')
                {
                    line_number++;
                    DEBUG (5, DEBUG_HDR, "TT_STRING or TT_POUND_DEFINE, inc line_number <%d>\n", 
                        line_number);
                }    
                p++;
            }  
        }

        /* Process each token type */
        DEBUG (5, DEBUG_HDR, "Switch on token type <%s>\n", show_type(type));

        switch (type)
        {
        case TT_NUMBER:
        case TT_CONSTANT: 
        case TT_VARIABLE:
            DEBUG (5, DEBUG_HDR, "Handling TT_NUMBER, TT_CONSTANT, TT_VARIABLE\n");

            if (skip_token('G'))
            {
            	break;
            }

            /* These tokens have null terminated length */
            DEBUG (5, DEBUG_HDR, "Saving token type <%s>\n", show_type(type));
            binary_bufncat(&program, ubyte_type(type), 1);  /* Save the token type */
            DEBUG (5, DEBUG_HDR, "Saving token <%s>\n", bufdata(&token));
            binary_bufncat(&program, (uint8_t *)bufdata(&token), bufsize(&token) + 1);
//            bufncat(&program, "", 1);
            break;

        case TT_IDENTIFIER:
            DEBUG (5, DEBUG_HDR, "Handling TT_INDENTIFIER\n");

            if (skip_token('G'))
            {
            	break;
            }

            if (pp_flag)
            {
            	;
            }
            else
            {  /* pp_flag not set */
                /* These tokens have null terminated length */

                DEBUG (5, DEBUG_HDR, "Saving token <%s> type <%s> \n", bufdata(&token), show_type(type));
                binary_bufncat(&program, ubyte_type(type), 1);  /* Save the token type */
                DEBUG (5, DEBUG_HDR, "Saving token <%s>\n", bufdata(&token));
                binary_bufncat(&program, (uint8_t *)bufdata(&token), bufsize(&token) + 1);
//                bufncat(&program, "", 1);
                break;  /* Done with this IDENTIFIER */
            }

            /* pp_flag is set, check for possible #define substitution */

            bufcpy(&name, bufdata(&token));
            bufprefix(&name, "-");
            bufprefix(&name, macro_name);
            if (variable_get(bufdata(&name), &value, VT_SC) == 1)
            {
                /* Not found yet, try again */
                bufcpy(&name, bufdata(&token));
                if (variable_get(bufdata(&name), &value, VT_SC) == 1)
                {
                    /* Does not exist as an sc variable */
                    /* Regular idendifier, these tokens have null terminated length...
                       All macro values are substituted here...  both #define's and macros */

                    DEBUG (5, DEBUG_HDR, "Saving token type <%s>\n", show_type(type));
                    binary_bufncat(&program, ubyte_type(type), 1);  /* Save the token type */
                    DEBUG (5, DEBUG_HDR, "Saving token <%s>\n", bufdata(&token));
                    binary_bufncat(&program, (uint8_t *)bufdata(&token), bufsize(&token) + 1);
//                    bufncat(&program, "", 1);
                    break;  /* Done with this IDENTIFIER */
                }
            }

            /* This IDENTIFIER needs to be substituted */

            if (bufsize(&value) == 0)
            {
                PP_ERROR ("Macro <%s> not defined\n", bufdata(&name));
            }

            LOG ("Line <%s>, Substituting <%s> with <%s>, in macro <%s>\n", 
                get_line_number(), bufdata(&name), bufdata(&value), macro_name);

            /* &name has the macro name, &value has the macro */

            /* Need to know if this is a simple #define X .... or macro #define X(.... */

            if (*bufdata(&value) == '(')
            {

                /* Step 1: This is a #define() macro, get all the passing arguments
                   and assign them to sc variables */
                /* In macro, "ADD (A, B) A + B", #macro[1]=A, #macro[2]=B, #macro[0]=2 */

                get_token(&token, &type, pp_flag);
                if (type == TT_OPEN_PAREN)
                {
                    i = 0;
                    while (1)
                    {
                        get_token(&token, &type, pp_flag);
                        DEBUG (5, DEBUG_HDR, "Parsing macro args, type <%s>\n", 
                            show_type(type));

                        if (type == TT_NULL_TOKEN)
                        {
                            PP_ERROR ("Expecting \")\"\n");
                 
                        }
                        else if (type == TT_CLOSE_PAREN)
                        {
                            sprintf(macro_arg, "#macro_arg[0]");
                            sprintf(num_str, "%d", i);
                            variable_put(macro_arg, num_str, 0, VT_SC); 
                            break;

                        }
                        else if (type == TT_COMMA)
                        {
                        	;
                        }
                        else
                        {
                            i++;
                            sprintf(macro_arg, "#macro_arg[%d]", i);
                            variable_put(macro_arg, bufdata(&token), 0, VT_SC); 
                        }
                    }
                    num_args = i;

                }
                else
                {
                    PP_ERROR ("Expecting \"(\"\n");
                }

                /* Step 2: Now go through the defined macro, and replace its arguments with 
                   those from the ones above */
                /* In macro "ADD (A, B) A + B",  ADD-A=#macro_arg[1], ADD-B=#macro_arg[2] */

                save_token_ptr = get_token_ptr();
                set_token(&value);
                get_token(&token, &type, pp_flag);
                if (type == TT_OPEN_PAREN)
                {
                    i = 0;
                    while (1)
                    {
                        get_token(&token, &type, pp_flag);
                        DEBUG (5, DEBUG_HDR, "Parsing macro args, type <%s>\n", 
                            show_type(type));

                        if (type == TT_NULL_TOKEN)
                        {
                            PP_ERROR ("Expecting \")\"\n");
                 
                        }
                        else if (type == TT_CLOSE_PAREN)
                        {
                            break;
                        }
                        else if (type == TT_COMMA)
                        {
                        	;
                        }
                        else
                        {
                            i++; 
                            sprintf(macro_arg, "#macro_arg[%d]", i);
                            variable_get(macro_arg, &tmp, VT_SC);

                            /* This is the code i was having trouble getting to work */ 
                            /* See if this value is yet a #define */
                            if (variable_get(bufdata(&tmp), &tmp2, VT_SC) == 0)
                            {
                                /* Exists as an sc variable */
                                bufcpy(&tmp, bufdata(&tmp2));
                            }

                            /* Create a name like ex: "ADD-A" */
                            bufprefix(&token, "-");
                            bufprefix(&token, bufdata(&name));
                            variable_put(bufdata(&token), bufdata(&tmp), 0, VT_SC); 

                            /* variable_put(bufdata(&token), bufdata(&tmp), 0, VT_SC);  */
                        } 
                    }

                    if (i != num_args)
                    {
                        PP_ERROR ("Num of macro args mismatch, found <%d>, expecting <%d>\n",
                            num_args, i);
                    }
                    read_it("", get_token_ptr(), bufdata(&name), true, false);
                    set_token_ptr(save_token_ptr);

                }
                else
                {
                    PP_ERROR ("Expecting \"(\"\n");
                }

            }
            else
            {
                /* This is a regular #define */
                save_token_ptr = get_token_ptr();
                read_it("", bufdata(&value), macro_name, true, false);
                set_token_ptr(save_token_ptr);
            }

            break;

        case TT_END_OF_LINE:
            DEBUG (5, DEBUG_HDR, "Handling TT_END_OF_LINE\n");

            if (skip_token('G'))
            {
            	break;
            }

            binary_bufncat(&program, ubyte_type(type), 1);  /* Save the token type */
            if (strcmp(bufdata(&token), ";") == 0)
            {
            	;
            }
            else
            {
                line_number++;
                type = TT_LINE_NUMBER;
                DEBUG (5, DEBUG_HDR, "Saving token type <%s>\n", show_type(type));
                binary_bufncat(&program, ubyte_type(type), 1);  /* Save the token type */
                sprintf(num, "%s:%d", file, line_number);
                set_line_number(num);
                DEBUG (5, DEBUG_HDR, "Saving token <%s>\n", num);
                binary_bufncat(&program, (uint8_t *)num, (uint32_t)(strlen(num) + 1));
//                bufncat(&program, "", 1);
            }
            break;
        
        case TT_STRING:
            DEBUG (5, DEBUG_HDR, "Handling TT_STRING\n");

            if (skip_token('G'))
            {
            	break;
            }

            {
            	uint32_t length = 0;

				/* These tokens use an int to define the length */
				DEBUG (5, DEBUG_HDR, "Saving token type <%s>\n", show_type(type));
				binary_bufncat(&program, ubyte_type(type), 1);  /* Save the token type */
				length = bufsize(&token);
				DEBUG (5, DEBUG_HDR, "Saving token <%s>, length <%d>\n", bufmkprint(&token), length);
				/*
				 * Note: Items of type TT_STRING are not stored with null terminators.
				 */
				binary_bufncat(&program, (uint8_t *) &length, sizeof(length));
				binary_bufncat(&program, (uint8_t *)bufdata(&token), length);
            }
            break;

        case TT_POUND_INCLUDE:
            DEBUG (5, DEBUG_HDR, "Handling TT_POUND_INCLUDE\n");

            if (skip_token('G'))
            {
            	break;
            }

            type = TT_END_OF_LINE;
            binary_bufncat(&program, ubyte_type(type), 1);
            save_token_ptr = get_token_ptr();
            read_it(bufdata(&token), "", "", true, false);
            set_token_ptr(save_token_ptr);
            break;

        case TT_POUND_DEFINE:
            DEBUG (5, DEBUG_HDR, "Handling TT_POUND_DEFINE\n");

            if (skip_token('G'))
            {
            	break;
            }

            bufrtrim(&token);  /* Remove the crap on the end, any extra spaces */
            /* Need to separate the name from the value, value will contain the name, and
               p will point to the value */
            p = bufdata(&token);
            len = parse(&value, p, &parse_type, "", false, false);
            p+=len;
            while (*p == ' ')
            {
            	p++;  /* Remove leading spaces */
            }

            /* Create the variable, check if it already exists first */
            if (variable_get(bufdata(&value), &tmp, VT_SC) == 1)
            {
                variable_put(bufdata(&value), p, 0, VT_SC); 

            }
            else
            {
                PP_ERROR ("Redefinition of #define not allowed <%s>\n", bufdata(&value));
            }
            break;

        case TT_POUND_IFDEF:
            DEBUG (5, DEBUG_HDR, "Handling TT_POUND_IFDEF\n");

            if (skip_token('G'))
            {
            	break;
            }

            bufrtrim(&token);  /* Remove the crap on the end, any extra spaces */
            /* Need to separate the name from the value, value will contain the name, and
               p will point to the value */
            p = bufdata(&token);
            len = parse(&value, p, &parse_type, "", false, false);
            p+=len;
            while (*p == ' ') p++;  /* Remove leading spaces */

            if (bufsize(&value) == 0)
            {
                PP_ERROR ("Missing argument for #ifdef\n");
            }

            /* Create the variable, check if it already exists first */
            if (variable_get(bufdata(&value), &tmp, VT_SC) == 1)
            {
                /* Does not exist */
                skip_token('S');
            }
            else
            {
                /* Exists */
                skip_token('C');
            }
       
            break;

        case TT_POUND_ELSE:
            DEBUG (5, DEBUG_HDR, "Handling TT_POUND_ENDIF\n");

            if (skip_token('G'))
            {
                /* If set, clear it */
                skip_token('C');
            }
            else
            {
                /*  clear it */
                skip_token('S');
            }
            break;

        case TT_POUND_ENDIF:
            DEBUG (5, DEBUG_HDR, "Handling TT_POUND_ENDIF\n");
            skip_token('C');
            break;

        case TT_COMMENT:
            break;

        default:
            DEBUG (5, DEBUG_HDR, "Handling default TT type\n");

            if (skip_token('G'))
            {
            	break;
            }

            /* These tokens have no length */
            DEBUG (5, DEBUG_HDR, "Saving token type <%s>\n", show_type(type));
            binary_bufncat(&program, ubyte_type(type), 1);  /* Save the token type */
            break;
        }

    }  /* End of while */

    program_idx = 0;
    buffree(&token);
    buffree(&source);
    buffree(&tmp);
    buffree(&tmp2);
    buffree(&name);
    buffree(&value);
    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/********************************************/

static bool is_file(char *file_name)
{
	bool ret = false;
	FILE *fd = (FILE *)NULL;

#undef NAME
#define NAME "is_file()"


    if ((fd = fopen(file_name, "r")) == NULL)
    {
        ret = false;
    }
    else
    {
        ret = true;
        fclose(fd);
    }

    DEBUG (3, DEBUG_HDR, "Called/returning, file <%s>, returning <%d>\n", file_name, ret);
    return ret;

}

/********************************************/

// TODO   does this really need to be static?
static bool skip_token(char flag)
{ 
	static bool skip_flag;

#undef NAME
#define NAME "skip_token()"

    if (flag == 'S')
    {
        /* Set the flag */
        skip_flag = true;
    }
    else if (flag == 'C')
    {
        /* Clear the flag */
        skip_flag = false;
    }
    else if (flag == 'G')
    {
        /* Get the flag */
    	// FIXME is this a bug where the state of skip token is left unchanged?
    	;
    }
    else
    {
        FATAL ("Invalid flag <%c>\n", flag);
    }
   
    /* Return the flag */ 
    DEBUG (3, DEBUG_HDR, "Called/returning, returning <%d>\n", skip_flag);
    return skip_flag;
}

/********************************************/

// I don't like this
uint8_t *ubyte_type(enum token_types type)
{
	static uint8_t ubyte_type = 0;
#undef NAME
#define NAME "short_type()"

	ubyte_type = type;

    return &ubyte_type;

}
