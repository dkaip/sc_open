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
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

static void look(const struct buffer *, struct buffer *, uint32_t *, uint32_t *);
//static void toupper_token(struct buffer *);
static int is_variable(struct buffer *);
static enum token_types get_token_type(const char *);
static enum token_types get_token_type_function(const char *);

static char *buffer;
static char *prev_buffer;

/*************************************************/

void set_token(struct buffer *buf)
{
    buffer = bufdata(buf);
    prev_buffer = buffer;
    return;
}

/*************************************************/

char *get_token_ptr()
{
/*int len;*/
#undef NAME
#define NAME "get_token_ptr()"

    return buffer;
}

/*************************************************/

void set_token_ptr(char *ptr)
{
/*int len;*/
#undef NAME
#define NAME "set_token_ptr()"

    buffer = ptr;
    DEBUG (4, DEBUG_HDR, "Set to <%d>\n", buffer);
    return;
}

/*************************************************/

void get_token(struct buffer *token, enum token_types *token_type, bool pp_flag)
{
	uint32_t len = 0;;
	enum parse_types parse_type = PT_NOT_ASSIGNED_YET;
	struct buffer tmp = INIT_BUFFER;

#undef NAME
#define NAME "get_token()"

    len = parse(token, buffer, &parse_type, "", true, pp_flag);
    prev_buffer = buffer;
    buffer+=len;

    switch (parse_type)
    {
        case PT_NUMBER:
            *token_type = TT_NUMBER;
            break;

        case PT_NULL:
            *token_type = TT_NULL_TOKEN;
            break;

        case PT_END_OF_LINE:
            *token_type = TT_END_OF_LINE;
            break;

        case PT_OTHER:
            *token_type = get_token_type(bufdata(token));
            break;

        case PT_NAME:

            if (is_variable(token)) {
                *token_type = TT_VARIABLE;

            } else {
                /* See if token is a keywword */
                *token_type = get_token_type(bufdata(token));
                if (*token_type == TT_UNKNOWN) {
                    /* Not a keyword, might be a function if followed by a "(" */
                    parse(&tmp, buffer, &parse_type, "", true, pp_flag);  /* Peak at the next token */
                    if (strcmp(bufdata(&tmp), "(") == 0) {
                        *token_type = get_token_type_function(bufdata(token));
                        if (*token_type == TT_UNKNOWN) {
                            *token_type = TT_IDENTIFIER;
                        }
                    } else {
                        *token_type = TT_IDENTIFIER;
                    }
                }
            }
            break;

        case PT_STRING:
            *token_type = TT_STRING;
            break;

        case PT_CONSTANT:
            *token_type = TT_CONSTANT;
            break;

        case PT_POUND_INCLUDE:
            *token_type = TT_POUND_INCLUDE;
            break;

        case PT_POUND_IFDEF:
            *token_type = TT_POUND_IFDEF;
            break;

        case PT_POUND_ELSE:
            *token_type = TT_POUND_ELSE;
            break;

        case PT_POUND_ENDIF:
            *token_type = TT_POUND_ENDIF;
            break;

        case PT_POUND_DEFINE:
            *token_type = TT_POUND_DEFINE;
            break;

        case PT_COMMENT:
            *token_type = TT_COMMENT;
            break;

        default:
            FATAL ("Don't know how to handle parsed value <%d>\n", parse_type);
    }

    DEBUG (4, DEBUG_HDR, "Returning token <%s>, token type <%s>\n", 
        bufdata(token), show_type(*token_type));
    buffree(&tmp);
    return;

}

/*************************************************/

void get_token_eval(struct buffer *token, enum token_types *type)
{

#undef NAME
#define NAME "get_token_eval()"

    get_token(token, type, false);

    if (*type == TT_VARIABLE)
    {
        evaluate_token(token);
    }

    DEBUG (4, DEBUG_HDR, "Returning token <%s>, type <%s>\n", bufdata(token), show_type(*type));
    return;

}

/*************************************************/

void get_token_toupper(struct buffer *token, enum token_types *type)
{
	char *c = (char *)NULL;

#undef NAME
#define NAME "get_token_toupper()"

    get_token(token, type, false);
    c = bufdata(token);
    for (uint32_t i=0; i<bufsize(token); i++)
    {
        c[i] = (char)toupper(c[i]);
    }

    DEBUG (4, DEBUG_HDR, "Returning token <%s>, type <%s>\n", bufdata(token), show_type(*type));

}

/*************************************************/

void put_token()
{

#undef NAME
#define NAME "put_token()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    buffer = prev_buffer;

}

/*************************************************/

bool is_operator(enum token_types type)
{
  
    if ((type == TT_PLUS) ||
        (type == TT_MINUS) ||
        (type == TT_UNARY_MINUS) ||
        (type == TT_UNARY_PLUS) ||
        (type == TT_MULT) ||
        (type == TT_DIVIDE) ||
        (type == TT_LOGICAL_AND) ||
        (type == TT_LOGICAL_OR) ||
        (type == TT_BITWISE_AND) ||
        (type == TT_BITWISE_OR) ||
        (type == TT_BITWISE_XOR) ||
        (type == TT_LESS_THAN) ||
        (type == TT_GREATER_THAN) ||
        (type == TT_EQUAL) ||
        (type == TT_STR_EQUAL) ||
        (type == TT_STR_NOT_EQUAL) ||
        (type == TT_LESS_THAN_EQUAL) ||
        (type == TT_GREATER_THAN_EQUAL) ||
        (type == TT_NOT_EQUAL) ||
        (type == TT_COLON) ||
        (type == TT_UNARY_ONES_COMPLEMENT) ||
        (type == TT_PERIOD))
    {
        return true;
    }
    else
    {
        return false;
    }

}

/*************************************************/

/*
 * was never used so I commented it out
 */
//static void toupper_token(struct buffer *token)
//{
//char *value;
//int i;
//
//    value = bufdata(token);
//    for (i=0; i<bufsize(token); i++) {
//        value[i] = toupper(value[i]);
//    }
//}

/*************************************************/

bool is_function(enum token_types type)
{
#undef NAME
#define NAME "is_function()"

    DEBUG (4, DEBUG_HDR, "Called, type <%s>\n", show_type(type));

    if ((type > TT_FUNCTION_MIN) && (type < TT_FUNCTION_MAX))
    {
        DEBUG (4, DEBUG_HDR, "True\n");
        return true;
    }
    else
    {
        DEBUG (4, DEBUG_HDR, "False\n");
        return false;
    }
}

/*************************************************/

static int is_variable(struct buffer *token)
{
char *p, *value;
#undef NAME
#define NAME "is_variable()"

    DEBUG (4, DEBUG_HDR, "Called,  <%s>\n", bufdata(token));
 
    value = bufdata(token);
    p = strchr(value, '$');

    if (p != (char *)NULL)
    {
        DEBUG (4, DEBUG_HDR, "Is a variable\n");
        return 1;
    }

	DEBUG (4, DEBUG_HDR, "Not a variable\n");
	return 0;
}

/*************************************************/

void evaluate_token(struct buffer *token)
{
	uint32_t idx1 = 0;
	uint32_t idx2 = 0;
//char *ptr;
struct buffer tmp = INIT_BUFFER;
struct buffer name = INIT_BUFFER;
struct buffer valu = INIT_BUFFER;
#undef NAME
#define NAME "evaluate_token()"

    DEBUG (4, DEBUG_HDR, "Called <%s>\n", bufdata(token));
//    ptr = bufdata(token);
    bufcpy(&name, "");
    bufcpy(&valu, "");

//    LOG ("Evaluate token <%s>\n", bufdata(token));

    while (is_variable(token))
    {
        look(token, &name, &idx1, &idx2);
        if (variable_get(bufdata(&name), &valu, VT_USER))
        {
            if (g_syntax_check)
            {
            	;
            }
            else
            {
                WARNING ("Variable <%s> not found, using \"\" for its value\n", bufdata(&name));
            }
        }

        /* Temporarily replace the $'s with \xff.  They will be replace below. 
           By doing so here, the $'s within a value will not be intrepreted as another variable. */
        while (strchr(bufdata(&valu), '$') != '\0')
        {
            *strchr(bufdata(&valu), '$') = '\xff';
        }

        bufcpy(&tmp, bufdata(token));
        bufncpy(token, bufdata(&tmp), idx1); 

        /* bufcat(token, bufdata(&valu)); old way */
        bufncat(token, bufdata(&valu), bufsize(&valu));

        bufcat(token, bufdata(&tmp)+idx2);
    }

    DEBUG (4, DEBUG_HDR, "Token <%s>\n", bufdata(token));

    /* Now convert the \xff (from above) back to $'s */
    while (strchr(bufdata(token), '\xff') != '\0')
    {
        *strchr(bufdata(token), '\xff') = '$';
    }

    buffree(&tmp);
    buffree(&name);
    buffree(&valu);
    DEBUG (4, DEBUG_HDR, "Returning <%s>\n", bufdata(token));
    return;
}

/*************************************************/

static void look(const struct buffer *token, struct buffer *name, uint32_t *idx1, uint32_t *idx2)
{
	char *data = (char *)NULL;
	int var, nest, nest_var;
#undef NAME
#define NAME "look()"

    DEBUG (4, DEBUG_HDR, "Look called, token <%s>\n", bufdata(token));    
    var = nest = nest_var = 0;
    data = bufdata(token);
    bufcpy(name, "");
    *idx1 = *idx2 = 0;

    for (uint32_t i=0; i<bufsize(token); i++)
    {
        if (data[i] == '$')
        {

            /* if ((data[i+1] == '$') || (data[i-1] == '$')) { */
            if (data[i+1] == '$')
            {
                /* The following is a $ so skip this one for now */
            	;
            }
            else if (nest)
            {
                if (nest_var == nest)
                {
                    break;
                }
                else
                {
                    bufcpy(name, "");
                    *idx1 = i;
                    *idx2 = *idx1 + 1;
                    nest_var = nest;
                    var = 1;  /* added */
                }
            }
            else if (bufsize(name))
            {
                break;                
            }
            else
            {
                var = 1;
                bufcpy(name, "");
                *idx1 = i;
                *idx2 = *idx1 + 1;
            }

        }
        else if (data[i] == '{')
        {
            (*idx2)++;;
            nest++;
        }
        else if (data[i] == '}')
        {
            if (nest_var == nest)
            {
                break;
            }
            (*idx2)++;;
            nest--;
        }
        else if (data[i] == '[')
        {
            bufncat(name, &data[i], 1);
            (*idx2)++;;
            nest++;
        }
        else if (data[i] == ']')
        {
            if (nest_var == nest)
            {
                break;
            }
            bufncat(name, &data[i], 1);
            (*idx2)++;;
            nest--;

        }
        else if (var)
        {
            if (isalnum(data[i]) ||
                (data[i] == '_') ||
                (data[i] == '.'))
            {
                bufncat(name, &data[i], 1);
                (*idx2)++;;

            }
            else
            {
                break;
            }
        }
    }

    DEBUG (4, DEBUG_HDR, "Returning name <%s>, idx1 <%d>, idx2 <%d>\n", 
        bufdata(name), *idx1, *idx2);
    return;
}

/*************************************************/

/*
 * Modified 12/30/16 to severely reduce the number of strcmp functions called.
 */
static enum token_types get_token_type_function(const char *t)
{
	enum token_types type = TT_NOT_ASSIGNED_YET;
	char token[128];

#undef NAME
#define NAME "get_token_type_function()"

    DEBUG (4, DEBUG_HDR, "Called, token <%s>\n", t);

//    memset(token, '\0', sizeof(token));
    token[0] = 0;
    strncpy(token, t, sizeof(token)-1);

    for (uint32_t i=0; i<strlen(token); i++)
    {
        token[i] = (char)toupper(t[i]);
    }

    if (token[0] == 'A')
    {
		if (strcmp(token, "ABS") == 0)
		{
			type = TT_ABS;
		}
		else if (strcmp(token, "ADD_TMP_VAR") == 0)
		{
			type = TT_ADD_TMP_VAR;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'B')
    {
		if (strcmp(token, "BTOI") == 0)
		{
			type = TT_BTOI;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'D')
    {
		if (strcmp(token, "DEL_TMP_VAR") == 0)
		{
			type = TT_DEL_TMP_VAR;
		}
		else if (strcmp(token, "DUMP") == 0)
		{
			type = TT_DUMP;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'E')
    {
		if (strcmp(token, "EXPORT") == 0)
		{
			type = TT_EXPORT;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'F')
    {
		if (strcmp(token, "FILE_CLOSE") == 0)
		{
			type = TT_FILE_CLOSE;
		}
		else if (strcmp(token, "FILE_OPEN") == 0)
		{
			type = TT_FILE_OPEN;
		}
		else if (strcmp(token, "FILE_OWC") == 0)
		{
			type = TT_FILE_OWC;
		}
		else if (strcmp(token, "FILE_READ") == 0)
		{
			type = TT_FILE_READ;
		}
		else if (strcmp(token, "FILE_WRITE") == 0)
		{
			type = TT_FILE_WRITE;
		}
		else if (strcmp(token, "FORMAT") == 0)
		{
			type = TT_FORMAT;
		}
		else if (strcmp(token, "FTOH") == 0)
		{
			type = TT_FTOH;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'G')
    {
		if (strcmp(token, "GET_CLIENT") == 0)
		{
			type = TT_GET_CLIENT;
		}
		else if (strcmp(token, "GET_ENV") == 0)
		{
			type = TT_GET_ENV;
		}
		else if (strcmp(token, "GET_TMP_VAR") == 0)
		{
			type = TT_GET_TMP_VAR;
		}
		else if (strcmp(token, "GET_VERSION") == 0)
		{
			type = TT_GET_VERSION;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'H')
    {
		if (strcmp(token, "HTOF") == 0)
		{
			type = TT_HTOF;
		}
		else if (strcmp(token, "HTOI") == 0)
		{
			type = TT_HTOI;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'I')
    {
		if (strcmp(token, "IMPORT") == 0)
		{
			type = TT_IMPORT;
		}
		else if (strcmp(token, "INSTRING") == 0)
		{
			type = TT_INSTRING;
		}
		else if (strcmp(token, "IS_LABEL") == 0)
		{
			type = TT_IS_LABEL;
		}
		else if (strcmp(token, "IS_TMP_VAR") == 0)
		{
			type = TT_IS_TMP_VAR;
		}
		else if (strcmp(token, "IS_VAR") == 0)
		{
			type = TT_IS_VAR;
		}
		else if (strcmp(token, "ITOA") == 0)
		{
			type = TT_ITOA;
		}
		else if (strcmp(token, "ITOB") == 0)
		{
			type = TT_ITOB;
		}
		else if (strcmp(token, "ITOH") == 0)
		{
			type = TT_ITOH;
		}
		else if (strcmp(token, "ITOO") == 0)
		{
			type = TT_ITOO;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'M')
    {
		if (strcmp(token, "MOD") == 0)
		{
			type = TT_MOD;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'O')
    {
		if (strcmp(token, "OTOI") == 0)
		{
			type = TT_OTOI;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'P')
    {
		if (strcmp(token, "PARSE") == 0)
		{
			type = TT_PARSE;
		}
		else if (strcmp(token, "POW") == 0)
		{
			type = TT_POW;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'S')
    {
		if (strcmp(token, "SCAN") == 0)
		{
			type = TT_SCAN;
		}
		else if (strcmp(token, "SET_TMP_VAR") == 0)
		{
			type = TT_SET_TMP_VAR;
		}
		else if (strcmp(token, "SORT_TMP_VAR") == 0)
		{
			type = TT_SORT_TMP_VAR;
		}
		else if (strcmp(token, "SQRT") == 0)
		{
			type = TT_SQRT;
		}
		else if (strcmp(token, "STRFTIME") == 0)
		{
			type = TT_STRFTIME;
		}
		else if (strcmp(token, "STRLEN") == 0)
		{
			type = TT_STRLEN;
		}
		else if (strcmp(token, "SUBSTR") == 0)
		{
			type = TT_SUBSTR;
		}
		else if (strcmp(token, "SWAP") == 0)
		{
			type = TT_SWAP;
		}
		else if (strcmp(token, "SYSTEM") == 0)
		{
			type = TT_SYSTEM;
		}
		else if (strcmp(token, "SYSTEM_SHELL") == 0)
		{
			type = TT_SYSTEM_SHELL;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'T')
    {
		if (strcmp(token, "TEST") == 0)
		{
			type = TT_TEST;
		}
		else if (strcmp(token, "TIME") == 0)
		{
			type = TT_TIME;
		}
		else if (strcmp(token, "TOUPPER") == 0)
		{
			type = TT_TOUPPER;
		}
		else if (strcmp(token, "TOLOWER") == 0)
		{
			type = TT_TOLOWER;
		}
		else if (strcmp(token, "TRACE") == 0)
		{
			type = TT_TRACE;
		}
		else if (strcmp(token, "T_PARSE") == 0)
		{
			type = TT_T_PARSE;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else
    {
        /* OK if there is no match here */
        type = TT_UNKNOWN;
    }

    DEBUG (4, DEBUG_HDR, "Returning type <%d>, <%s>\n", type, show_type(type));
    return type;
}

/*************************************************/

/*
 * Modified 12/30/16 to severely reduce the number of strcmp functions called.
 */
static enum token_types get_token_type(const char *t)
{
	enum token_types type = TT_NOT_ASSIGNED_YET;
	char token[128];

#undef NAME
#define NAME "get_token_type()"

    DEBUG (4, DEBUG_HDR, "Called, token <%s>\n", t);

//    memset(token, '\0', sizeof(token));
    token[0] = 0;
    strncpy(token, t, sizeof(token)-1);

    for (uint32_t i=0; i<strlen(token); i++)
    {
        token[i] = (char)toupper(t[i]);
    }

    if (token[0] == 'A')
    {
    	//    else if (strcmp(token, "AND") == 0)
		if ((token[0] == 'A') && (token[1] == 'N') && (token[2] == 'D') && (token[3] == 0))
		{
			type = TT_LOGICAL_AND;
		}
    	else if (strcmp(token, "AFTER") == 0)
		{
			type = TT_AFTER;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'B')
    {
		if (strcmp(token, "BREAK") == 0)
		{
			type = TT_BREAK;
		}
		else if (strcmp(token, "BREAK_LOOP") == 0)
		{
			type = TT_BREAK_LOOP;
		}
		else if (strcmp(token, "BEGIN") == 0)
		{
			type = TT_BEGIN;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'C')
    {
		if (strcmp(token, "CLOSE") == 0)
		{
			type = TT_CLOSE;
		}
		else if (strcmp(token, "CONTINUE") == 0)
		{
			type = TT_CONTINUE;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'D')
    {
		if (strcmp(token, "DEFINE") == 0)
		{
			type = TT_DEFINE;
		}
		else if (strcmp(token, "DEFINE_ARRAY") == 0)
		{
			type = TT_DEFINE_ARRAY;
		}
		else if (strcmp(token, "DEFINE_XREF") == 0)
		{
			type = TT_DEFINE_XREF;
		}
		else if (strcmp(token, "DELETE") == 0)
		{
			type = TT_DELETE;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'E')
    {
    	//    else if (strcmp(token, "END") == 0)
		if ((token[0] == 'E') && (token[1] == 'N') && (token[2] == 'D') && (token[3] == 0))
		{
			type = TT_END;
		}
		//    else if (strcmp(token, "EQ") == 0)
		else if ((token[0] == 'E') && (token[1] == 'Q') && (token[2] == 0))
		{
			type = TT_STR_EQUAL;
		}
		else if (strcmp(token, "ELSE") == 0)
		{
			type = TT_ELSE;
		}
		else if (strcmp(token, "ELSE_IF") == 0)
		{
			type = TT_ELSE_IF;
		}
		else if (strcmp(token, "ERROR") == 0)
		{
			type = TT_ERROR;
		}
		else if (strcmp(token, "END_IF") == 0)
		{
			type = TT_END_IF;
		}
		else if (strcmp(token, "END_WHILE") == 0)
		{
			type = TT_END_WHILE;
		}
		else if (strcmp(token, "EXIT") == 0)
		{
			type = TT_EXIT;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
//    else if (strcmp(token, "FOR") == 0)
    else if ((token[0] == 'F') && (token[1] == 'O') && (token[2] == 'R') && (token[3] == 0))
    {
        type = TT_FOR;
    }
    else if (token[0] == 'G')
    {
		if (strcmp(token, "GOSUB") == 0)
		{
			type = TT_GOSUB;
		}
		else if (strcmp(token, "GOTO") == 0)
		{
			type = TT_GOTO;
		}
		else if (strcmp(token, "GLET") == 0)
		{
			type = TT_GLET;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
//    else if (strcmp(token, "IF") == 0)
    else if ((token[0] == 'I') && (token[1] == 'F') && (token[2] == 0))
    {
        type = TT_IF;
    }
    else if (token[0] == 'L')
    {
    	//    else if (strcmp(token, "LET") == 0)
		if ((token[0] == 'L') && (token[1] == 'E') && (token[2] == 'T') && (token[3] == 0))
		{
			type = TT_LET;
		}
    	else if (strcmp(token, "LABEL") == 0)
		{
			type = TT_LABEL;
		}
		else if (strcmp(token, "LOCAL_VAR") == 0)
		{
			type = TT_LOCAL_VAR;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (token[0] == 'N')
    {
    	//    else if (strcmp(token, "NE") == 0)
		if ((token[0] == 'N') && (token[1] == 'E') && (token[2] == 0))
		{
			type = TT_STR_NOT_EQUAL;
		}
		else if (strcmp(token, "NO_ERROR") == 0)
		{
			type = TT_NO_ERROR;
		}
		else if (strcmp(token, "NEXT") == 0)
		{
			type = TT_NEXT;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
    else if (strcmp(token, "OPEN") == 0)
    {
        type = TT_OPEN;
    }
//    else if (strcmp(token, "OR") == 0)
    else if ((token[0] == 'O') && (token[1] == 'R') && (token[2] == 0))
    {
        type = TT_LOGICAL_OR;
    }
    else if (strcmp(token, "PRAGMA") == 0)
    {
        type = TT_PRAGMA;
    }
    else if (strcmp(token, "PRINT") == 0)
    {
        type = TT_PRINT;
    }
    else if (strcmp(token, "RETURN") == 0)
    {
        type = TT_RETURN;
    }
    else if (token[0] == 'S')
    {
		if (strcmp(token, "SEND") == 0)
		{
			type = TT_SEND;
		}
		else if (strcmp(token, "SEND_REPLY") == 0)
		{
			type = TT_SEND_REPLY;
		}
		else if (strcmp(token, "SLEEP") == 0)
		{
			type = TT_SLEEP;
		}
		else if (strcmp(token, "START_TIMER") == 0)
		{
			type = TT_START_TIMER;
		}
		else if (strcmp(token, "STEP") == 0)
		{
			type = TT_STEP;
		}
		else if (strcmp(token, "STOP_TIMER") == 0)
		{
			type = TT_STOP_TIMER;
		}
		else if (strcmp(token, "SET_PROGRAM") == 0)
		{
			type = TT_SET_PROGRAM;
		}
	    else
	    {
	        type = TT_UNKNOWN;
	    }
    }
//    else if (strcmp(token, "TO") == 0)
    else if ((token[0] == 'T') && (token[1] == 'O') && (token[2] == 0))
    {
        type = TT_TO;
    }
    else if (strcmp(token, "THEN") == 0)
    {
        type = TT_THEN;
    }
    else if (strcmp(token, "WHILE") == 0)
    {
        type = TT_WHILE;
    }
//    else if (strcmp(token, "<") == 0)
    else if (token[0] == '<' && token[1] == 0)
    {
        type = TT_LESS_THAN;
    }
//    else if (strcmp(token, ">") == 0)
    else if (token[0] == '>' && token[1] == 0)
    {
        type = TT_GREATER_THAN;
    }
//    else if (strcmp(token, "=") == 0)
    else if (token[0] == '=' && token[1] == 0)
    {
        type = TT_ASSIGN;
    }
//    else if (strcmp(token, "!") == 0)
    else if (token[0] == '!' && token[1] == 0)
    {
        type = TT_BANG;
    }
//    else if (strcmp(token, "<=") == 0)
    else if ((token[0] == '<') && (token[1] == '=') && (token[2] == 0))
    {
        type = TT_LESS_THAN_EQUAL;
    }
//    else if (strcmp(token, ">=") == 0)
    else if ((token[0] == '>') && (token[1] == '=') && (token[2] == 0))
    {
        type = TT_GREATER_THAN_EQUAL;
    }
//    else if (strcmp(token, "==") == 0)
    else if ((token[0] == '=') && (token[1] == '=') && (token[2] == 0))
    {
        type = TT_EQUAL;
    }
//    else if (strcmp(token, "!=") == 0)
    else if ((token[0] == '!') && (token[1] == '=') && (token[2] == 0))
    {
        type = TT_NOT_EQUAL;
    }
//    else if (strcmp(token, "(") == 0)
    else if (token[0] == '(' && token [1] == 0)
    {
        type = TT_OPEN_PAREN;
    }
//    else if (strcmp(token, ")") == 0)
    else if (token[0] == ')' && token [1] == 0)
    {
        type = TT_CLOSE_PAREN;
    }
//    else if (strcmp(token, ",") == 0)
    else if (token[0] == ',' && token [1] == 0)
    {
        type = TT_COMMA;
    }
//    else if (strcmp(token, "-") == 0)
    else if (token[0] == '-' && token [1] == 0)
    {
        type = TT_MINUS;
    }
//    else if (strcmp(token, "~") == 0)
    else if (token[0] == '~' && token [1] == 0)
    {
        type = TT_UNARY_ONES_COMPLEMENT;
    }
//    else if (strcmp(token, "+") == 0)
    else if (token[0] == '+' && token [1] == 0)
    {
        type = TT_PLUS;
    }
//    else if (strcmp(token, "/") == 0)
    else if (token[0] == '/' && token [1] == 0)
    {
        type = TT_DIVIDE;
    }
//    else if (strcmp(token, "*") == 0)
    else if (token[0] == '*' && token [1] == 0)
    {
        type = TT_MULT;
    }
//    else if (strcmp(token, "|") == 0)
    else if (token[0] == '|' && token [1] == 0)
    {
        type = TT_BITWISE_OR;
    }
//    else if (strcmp(token, "^") == 0)
    else if (token[0] == '^' && token [1] == 0)
    {
        type = TT_BITWISE_XOR;
    }
//    else if (strcmp(token, "&") == 0)
    else if (token[0] == '&' && token [1] == 0)
    {
        type = TT_BITWISE_AND;
    }
//    else if (strcmp(token, ".") == 0)
    else if (token[0] == '.' && token [1] == 0)
    {
        type = TT_PERIOD;
    }
//    else if (strcmp(token, ":") == 0)
    else if (token[0] == ':' && token [1] == 0)
    {
        type = TT_COLON;
    }
//    else if (strcmp(token, "\\") == 0)
    else if (token[0] == '\\' && token [1] == 0)
    {
        type = TT_ESC;
    }
    else if (
        (strcmp(token, "XTREALIZEWIDGET") == 0) ||
        (strcmp(token, "XTSETVALUES") == 0) ||
        (strcmp(token, "XTAPPINITIALIZE") == 0) ||
        (strncmp(token, "XMCREATE", strlen("XMCREATE")) == 0)
        )
    {
        type = TT_X;
    }
    else
    {
        type = TT_UNKNOWN;
    }
    
    DEBUG (4, DEBUG_HDR, "Returning type <%d>, <%s>\n", type, show_type(type));
    return type;
}

/*************************************************/

char *show_type(enum token_types type)
{
#undef NAME
#define NAME "show_type()"

    if (type == TT_IDENTIFIER) return "identifier";
    if (type == TT_LINE_NUMBER) return "#";
    if (type == TT_NUMBER) return "number";
    if (type == TT_VARIABLE) return "variable";
    if (type == TT_STRING) return "string";
    if (type == TT_CONSTANT) return "constant";
    if (type == TT_OPERATOR) return "operator";
    if (type == TT_END_OF_LINE) return "eol";
    if (type == TT_NULL_TOKEN) return "null token";
    if (type == TT_SPECIAL) return "special";
    if (type == TT_POUND_INCLUDE) return "#include";
    if (type == TT_POUND_DEFINE) return "#define";
    if (type == TT_POUND_IFDEF) return "#ifdef";
    if (type == TT_POUND_ELSE) return "#else";
    if (type == TT_POUND_ENDIF) return "#endif";
    if (type == TT_COMMENT) return "comment";
    if (type == TT_UNKNOWN) return "unknown";

    if (type == TT_AFTER) return "after";
    if (type == TT_BREAK) return "break";
    if (type == TT_BREAK_LOOP) return "break_loop";
    if (type == TT_BEGIN) return "begin";
    if (type == TT_CLOSE) return "close";
    if (type == TT_CONTINUE) return "continue";
    if (type == TT_DEFINE) return "define";
    if (type == TT_LOCAL_VAR) return "local_var";
    if (type == TT_TRACE) return "trace";
    if (type == TT_DEFINE_ARRAY) return "define_array";
    if (type == TT_DEFINE_XREF) return "define_xref";
    if (type == TT_DELETE) return "delete";
    if (type == TT_DUMP) return "dump";
    if (type == TT_LET) return "let";
    if (type == TT_GLET) return "glet";
    if (type == TT_END) return "end";
    if (type == TT_ELSE) return "else";
    if (type == TT_ELSE_IF) return "else_if";
    if (type == TT_END_IF) return "end_if";
    if (type == TT_END_WHILE) return "end_while";
    if (type == TT_ERROR) return "error";
    if (type == TT_EXIT) return "exit";
    if (type == TT_FILE_CLOSE) return "file_close";
    if (type == TT_FILE_OPEN) return "file_open";
    if (type == TT_FILE_OWC) return "file_owc";
    if (type == TT_FILE_READ) return "file_read";
    if (type == TT_FILE_WRITE) return "file_write";
    if (type == TT_FOR) return "for";
    if (type == TT_FORMAT) return "format";
    if (type == TT_GOSUB) return "gosub";
    if (type == TT_GOTO) return "goto";
    if (type == TT_IF) return "if";
    if (type == TT_LABEL) return "label";
    if (type == TT_NEXT) return "next";
    if (type == TT_NO_ERROR) return "no_error";
    if (type == TT_OPEN) return "open";
    if (type == TT_T_PARSE) return "t_parse";
    if (type == TT_PARSE) return "parse";
    if (type == TT_PRAGMA) return "pragma";
    if (type == TT_PRINT) return "print";
    if (type == TT_RETURN) return "return";
    if (type == TT_SCAN) return "scan";
    if (type == TT_SEND) return "send";
    if (type == TT_SEND_REPLY) return "send_reply";
    if (type == TT_SET_PROGRAM) return "set_program";
    if (type == TT_SLEEP) return "sleep";
    if (type == TT_THEN) return "then";
    if (type == TT_WHILE) return "while";

    if (type == TT_ASSIGN) return "=";
    if (type == TT_BANG) return "!";
    if (type == TT_EQUAL) return "==";
    if (type == TT_GREATER_THAN) return ">";
    if (type == TT_GREATER_THAN_EQUAL) return ">=";
    if (type == TT_LESS_THAN) return "<";
    if (type == TT_LESS_THAN_EQUAL) return "<=";
    if (type == TT_NOT_EQUAL) return "!=";

    if (type == TT_BITWISE_AND) return "&";
    if (type == TT_CLOSE_PAREN) return ")";
    if (type == TT_COMMA) return ",";
    if (type == TT_DIVIDE) return "/";
    if (type == TT_LOGICAL_AND) return "and";
    if (type == TT_LOGICAL_OR) return "or";
    if (type == TT_MINUS) return "-";
    if (type == TT_UNARY_MINUS) return "unary -";
    if (type == TT_UNARY_ONES_COMPLEMENT) return "~";
    if (type == TT_MULT) return "*";
    if (type == TT_OPEN_PAREN) return "(";
    if (type == TT_BITWISE_OR) return "|";
    if (type == TT_BITWISE_XOR) return "^";
    if (type == TT_PLUS) return "+";
    if (type == TT_UNARY_PLUS) return "unary +";
    if (type == TT_PERIOD) return ".";
    if (type == TT_COLON) return ":";
    if (type == TT_ESC) return "esc";
    if (type == TT_STR_EQUAL) return "eq";
    if (type == TT_STR_NOT_EQUAL) return "ne";

    if (type == TT_FUNCTION_MIN) return "function_min";
    if (type == TT_FUNCTION_MAX) return "function_max";

    if (type == TT_FUNCTION) return "function";
    if (type == TT_SUBSTR) return "substr";
    if (type == TT_SYSTEM) return "system";
    if (type == TT_SYSTEM_SHELL) return "system_shell";
    if (type == TT_GET_CLIENT) return "get_client";
    if (type == TT_GET_ENV) return "get_env";
    if (type == TT_GET_VERSION) return "get_version";
    if (type == TT_ABS) return "abs";
    if (type == TT_HTOI) return "htoi";
    if (type == TT_OTOI) return "otoi";
    if (type == TT_ITOH) return "itoh";
    if (type == TT_BTOI) return "btoi";
    if (type == TT_EXPORT) return "export";
    if (type == TT_IMPORT) return "import";
    if (type == TT_ITOA) return "itoa";
    if (type == TT_HTOF) return "htof";
    if (type == TT_FTOH) return "ftoh";
    if (type == TT_SWAP) return "swap";
    if (type == TT_ITOB) return "itob";
    if (type == TT_ITOO) return "itoo";
    if (type == TT_INSTRING) return "instring";
    if (type == TT_IS_LABEL) return "is_label";
    if (type == TT_IS_VAR) return "is_var";
    if (type == TT_IS_TMP_VAR) return "is_tmp_var";
    if (type == TT_SQRT) return "sqrt";
    if (type == TT_STRFTIME) return "strftime";
    if (type == TT_MOD) return "mod";
    if (type == TT_STRLEN) return "strlen";
    if (type == TT_GET_TMP_VAR) return "get_tmp_var";
    if (type == TT_DEL_TMP_VAR) return "del_tmp_var";
    if (type == TT_SORT_TMP_VAR) return "sort_tmp_var";
    if (type == TT_ADD_TMP_VAR) return "add_tmp_var";
    if (type == TT_SET_TMP_VAR) return "set_tmp_var";
    if (type == TT_TEST) return "test";
    if (type == TT_TIME) return "time";
    if (type == TT_POW) return "pow";

    if (type == TT_TO) return "to";
    if (type == TT_TOLOWER) return "tolower";
    if (type == TT_TOUPPER) return "toupper";
    if (type == TT_START_TIMER) return "start_timer";
    if (type == TT_STEP) return "step";
    if (type == TT_STOP_TIMER) return "stop_timer";

    if (type == TT_X) return "x<command>";

    FATAL ("Token type <%d> NOT DEFINED\n", type);

    return ("Not defined, but it should be");
}

/*************************************************/


void show_all_types()
{

/*
 * For now just commenting this out since it does not do anything.
 */
//int i;
//char *s;
//
//    for (i=1; i<TT_END_OF_LIST; i++) {
//        /* printf ("  <%d) <%s>\n", i, show_type(i)); */
//        s = show_type(i);
//    }

    return;
}

