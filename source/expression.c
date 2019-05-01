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
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"

#define MAX_Q 500

static uint32_t expr(struct buffer *);
static uint32_t term(struct buffer *);
static uint32_t func(struct buffer *);
static uint32_t solve(struct buffer *);
static uint32_t solve_function(struct buffer *);
static uint32_t add_sub_mult_div(enum token_types, char *, char *, struct buffer *);
static uint32_t logical_bitwise(enum token_types, struct buffer *, struct buffer *, struct buffer *);
static void str_cmp(enum token_types, const struct buffer *, const struct buffer *, struct buffer *);
static void concat(struct buffer *, struct buffer *, struct buffer *, char *);
static uint32_t ones_complement(struct buffer *);
static bool has_null(const struct buffer *);

static void q_token(const struct buffer *, enum token_types);
static void dq_token(struct buffer *, enum token_types *);
static void q_init();
static void q_free();
//static void q_dump();

static char *q_of_tokens[MAX_Q];
static enum token_types q_of_types[MAX_Q];
static int q_idx;
static int isint(const char *);
static int isnumber(const char *);

/********************************************/

uint32_t expression(struct buffer *token)
{
	uint32_t iret = 1;

#undef NAME
#define NAME "expression()"

    DEBUG (3, DEBUG_HDR, "Called\n");
    q_init();

    iret = expr(token);
    if (iret) {
        /* ERROR ("Error with expression\n"); */
        DEBUG (3, DEBUG_HDR, "Expr failed\n");
        q_free();
        return iret;
    }

    /* q_dump();  */

    iret = solve(token);
    if (iret) {
        /* ERROR ("Error while solving expression\n"); */
        DEBUG (3, DEBUG_HDR, "Solve failed\n");
        q_free();
        return iret;
    }

    q_free();
    DEBUG (3, DEBUG_HDR, "Returning <%s>, length <%d>\n", bufdata(token), bufsize(token));
    return 0;
}

/********************************************/

static uint32_t expr(struct buffer *token)
{
	uint32_t iret = 1;
	enum token_types type = TT_NOT_ASSIGNED_YET;

#undef NAME
#define NAME "expr()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    iret = term(token);
    if (iret)
    {
        /* ERROR ("Error with expression term\n"); */
        DEBUG (4, DEBUG_HDR, "Returning failed\n");
        return iret;
    }
    
    while (1)
    {
        get_program_token(token, &type);
        DEBUG (4, DEBUG_HDR, "Got token <%s>, type <%s>\n", bufdata(token), show_type(type));
        if (is_operator(type))
        {
            q_token(token, type);
            iret = term(token);
            if (iret != 0)
            {
                /* ERROR ("Error with expression term\n"); */
                DEBUG (4, DEBUG_HDR, "Returning failed\n");
                return iret;
            }
        }
        else
        {
            put_program_token();
            break;
        }
    }

    DEBUG (4, DEBUG_HDR, "Returning success\n");
    return 0;
}


/********************************************/

static uint32_t term(struct buffer *token)
{
	uint32_t iret = 1;
	enum token_types type = TT_NOT_ASSIGNED_YET;

#undef NAME
#define NAME "term()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    get_program_token(token, &type);

    if (is_function(type))
    {
        q_token(token, type);
        iret = func(token);
        if (iret)
        {
            /* ERROR ("Error with function\n"); */
            return iret;

        }
        else
        {
            return 0;
        }

    }
    else if (type == TT_VARIABLE)
    {
        q_token(token, type);
        return 0;

    }
    else if (type == TT_IDENTIFIER)
    {
        q_token(token, type);
        return 0;

    }
    else if (type == TT_STRING)
    {
        q_token(token, type);
        return 0;

    }
    else if (type == TT_CONSTANT)
    {
        q_token(token, type);
        return 0;

    }
    else if (type == TT_NUMBER)
    {
        q_token(token, type);
        return 0;

    }
    else if (type == TT_MINUS)
    {
        q_token(token, TT_UNARY_MINUS);
        term(token);
        return 0;

    }
    else if (type == TT_PLUS)
    {
        q_token(token, TT_UNARY_PLUS);
        term(token);
        return 0;

    }
    else if (type == TT_UNARY_ONES_COMPLEMENT)
    {
        q_token(token, TT_UNARY_ONES_COMPLEMENT);
        term(token);
        return 0;
        
    }
    else if (type == TT_OPEN_PAREN)
    {
        q_token(token, type);
        iret = expr(token);
        if (iret != 0)
        {
            /* ERROR ("Error with expression\n"); */
            return iret;
        }

        get_program_token(token, &type);
        DEBUG (4, DEBUG_HDR, "Got token <%s>, type <%s>\n", bufdata(token), show_type(type));
        
        if (type == TT_CLOSE_PAREN)
        {
            q_token(token, type);
            return 0;

        }
        else
        {
            ERROR ("Invalid token <%s>, type <%s>, expecting \")\"\n", 
                bufdata(token), show_type(type)); 
            return 1;
        }

    }
    else
    {
        ERROR ("Invalid token <%s>, type <%s>, expecting IDENTIFIER, NUMBER, VARIABLE, CONSTANT, or STRING\n", 
            bufdata(token), show_type(type)); 
        return 1;
    }

    return 1;
}

/********************************************/

static uint32_t func(struct buffer *token)
{
	uint32_t iret = 1;
	enum token_types type = TT_NOT_ASSIGNED_YET;

#undef NAME
#define NAME "func()"

    DEBUG (4, DEBUG_HDR, "Called, token is <%s>\n", bufdata(token));

    get_program_token(token, &type);
    DEBUG (4, DEBUG_HDR, "Got token <%s>, type <%s>\n", bufdata(token), show_type(type));
        
    if (type == TT_OPEN_PAREN)
    {
        get_program_token(token, &type);
        DEBUG (4, DEBUG_HDR, "Got token <%s>, type <%s>\n", bufdata(token), show_type(type));
        if (type == TT_CLOSE_PAREN)
        {
            q_token(token, TT_FUNCTION);
            DEBUG (4, DEBUG_HDR, "Returning success\n");
            return 0;
        }
        put_program_token();
 
        iret = expr(token);
        if (iret != 0)
        {
            /* ERROR ("Error with expression\n"); */
            DEBUG (4, DEBUG_HDR, "Returning failed\n");
            return iret;
        }

        while (1)
        {
            get_program_token(token, &type);
            DEBUG (4, DEBUG_HDR, "Got token <%s>, type <%s>\n", bufdata(token), show_type(type));
            if (type == TT_CLOSE_PAREN)
            {
                q_token(token, TT_FUNCTION);
                DEBUG (4, DEBUG_HDR, "Returning success\n");
                return 0;

            }
            else if (type == TT_COMMA)
            {
                q_token(token, type);
                iret = expr(token);
                if (iret != 0)
                {
                    /* ERROR ("Error with expression\n"); */
                    DEBUG (4, DEBUG_HDR, "Returning failed\n");
                    return iret;
                }

            }
            else
            {
                ERROR ("Invalid token <%s>, type <%s>, expecting \")\" or \",\"\n", 
                    bufdata(token), show_type(type)); 
                return 1;
            }
        }

    }
    else
    {
        ERROR ("Invalid token <%s>, type <%s>, expecting \"(\"\n", 
            bufdata(token), show_type(type)); 
        return 1;
    }
}

/********************************************/

static uint32_t solve(struct buffer *token)
{
	struct buffer arg1 = INIT_BUFFER;
	struct buffer arg2 = INIT_BUFFER;
	struct buffer op = INIT_BUFFER;
	uint32_t iret = 1;
	enum token_types op1_type = TT_NOT_ASSIGNED_YET;
	enum token_types op2_type = TT_NOT_ASSIGNED_YET;
	enum token_types arg1_type = TT_NOT_ASSIGNED_YET;
	enum token_types arg2_type = TT_NOT_ASSIGNED_YET;

#undef NAME
#define NAME "solve()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    iret = 0;
    while (q_idx != 0)
    {

        /* Get the first token */

        dq_token(&arg1, &arg1_type); 

        if (arg1_type == TT_CLOSE_PAREN)
        {
            iret = solve(token);
            if (iret != 0)
            {
                /* ERROR ("Error solving expression\n"); */
                break;
            }
            dq_token(&arg1, &arg1_type); 

        }
        else if (arg1_type == TT_FUNCTION)
        {
            iret = solve_function(token);
            if (iret != 0)
            {
                /* ERROR ("Error solving function\n"); */
                break;
            }
            dq_token(&arg1, &arg1_type); 
        }
       
        /* Get the operator */ 

        dq_token(&op, &op1_type); 

        if (op1_type == TT_NULL_TOKEN)
        {
            /* bufcpy(token, bufdata(&arg1)); */
            bufncpy(token, bufdata(&arg1), bufsize(&arg1));
            continue;
        }

        if (op1_type == TT_OPEN_PAREN)
        {
            q_token(&arg1, arg1_type);
            break;

        }
        else if (op1_type == TT_COMMA)
        {
            q_token(&op, op1_type);
            q_token(&arg1, arg1_type);
            break;

        }
        else if (is_function(op1_type))
        {
            q_token(&op, op1_type);
            q_token(&arg1, arg1_type);
            break;

        }
        else if (op1_type == TT_UNARY_MINUS)
        {
            iret = add_sub_mult_div(TT_MULT, bufdata(&arg1), "-1", token);
            if (iret != 0)
            {
                ERROR ("Error with add/sub/mult/div operation\n");
                break;
            }
            q_token(token, arg1_type);
            continue;

        }
        else if (op1_type == TT_UNARY_ONES_COMPLEMENT)
        {
            iret = ones_complement(&arg1);
            if (iret != 0)
            {
                break;
            }
            q_token(&arg1, arg1_type);
            continue;

        }
        else if (op1_type == TT_UNARY_PLUS)
        {
            q_token(&arg1, arg1_type);
            continue;
        }

        /* Get the second token */  

        dq_token(&arg2, &arg2_type); 

        if (arg2_type == TT_NULL_TOKEN)
        {
            continue;
        }

        if (arg2_type == TT_FUNCTION)
        {
            iret = solve_function(token);
            if (iret != 0)
            {
                /* ERROR ("Error solving function\n"); */
                break;
            }
            dq_token(&arg2, &arg2_type); 
        }
        
        if (arg2_type == TT_CLOSE_PAREN)
        {
            iret = solve(token);
            if (iret != 0)
            {
                /* ERROR ("Error solving expression\n"); */
                break;
            }
            dq_token(&arg2, &arg2_type); 
        }

        /* Check for a Unary operator */

        dq_token(&op, &op2_type); 

        if (op2_type == TT_UNARY_MINUS)
        {
            iret = add_sub_mult_div(TT_MULT, bufdata(&arg2), "-1", token);
            if (iret != 0)
            {
                ERROR ("Error with add/sub/mult/div operation\n");
                break;
            }
            bufcpy(&arg2, bufdata(token));

        }
        else if (op2_type == TT_UNARY_ONES_COMPLEMENT)
        {
            iret = ones_complement(&arg2);
            if (iret != 0)
            {
                break;
            }

        }
        else if (op2_type == TT_UNARY_PLUS)
        {
        	;
        }
        else
        {
            q_token(&op, op2_type);  /* Put it back */
        }

        if ((op1_type == TT_PLUS) || 
            (op1_type == TT_MINUS) || 
            (op1_type == TT_MULT) || 
            (op1_type == TT_DIVIDE))
        {
            iret = add_sub_mult_div(op1_type, bufdata(&arg1), bufdata(&arg2), token);
            if (iret != 0)
            {
                ERROR ("Error with add/sub/mult/div operation\n");
                break;
            }
            q_token(token, arg1_type);

        }
        else if (op1_type == TT_PERIOD)
        {
//            iret = concat(&arg1, &arg2, token, "");
//            if (iret != 0)
//            {
//                ERROR ("Error with string concatenation\n");
//                break;
//            }
            concat(&arg1, &arg2, token, "");
            q_token(token, arg1_type);

        }
        else if (op1_type == TT_COLON)
        {
//            iret = concat(&arg1, &arg2, token, " ");
//            if (iret != 0)
//            {
//                ERROR ("Error with string concatenation\n");
//                break;
//            }
            concat(&arg1, &arg2, token, " ");
            q_token(token, arg1_type);

        }
        else if ((op1_type == TT_LOGICAL_AND) ||
            (op1_type == TT_LOGICAL_OR) ||
            (op1_type == TT_BITWISE_AND) ||
            (op1_type == TT_BITWISE_OR) || 
            (op1_type == TT_BITWISE_XOR) || 
            (op1_type == TT_GREATER_THAN) || 
            (op1_type == TT_LESS_THAN) || 
            (op1_type == TT_EQUAL) ||
            (op1_type == TT_NOT_EQUAL) ||
            (op1_type == TT_LESS_THAN_EQUAL) ||
            (op1_type == TT_GREATER_THAN_EQUAL))
        {
            iret = logical_bitwise(op1_type, &arg1, &arg2, token);
            if (iret != 0)
            {
                ERROR ("Error with bitwise operation\n");
                break;
            }
            q_token(token, arg1_type);

        }
        else if ((op1_type == TT_STR_EQUAL) ||
            (op1_type == TT_STR_NOT_EQUAL))
        {
//            iret = str_cmp(op1_type, &arg1, &arg2, token);
//            if (iret != 0)
//            {
//                ERROR ("Error with string compare\n");
//                break;
//            }
            str_cmp(op1_type, &arg1, &arg2, token);
            q_token(token, arg1_type);

        }
        else
        {
            ERROR ("Not a valid operator <%s>\n", bufdata(&op)); 
            iret = 1;
            break;
        }
    }
   
    buffree(&arg1);
    buffree(&arg2);
    buffree(&op);

    return iret;
}

/********************************************/

static uint32_t solve_function(struct buffer *token)
{
	uint32_t iret = 0;  // Needs to be zero
	int sysret = 1;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	FILE *fp = NULL;
	int i_arg = 0;
	uint32_t j = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	bool flag = false;
//	uint32_t ul = 0L;
	uint8_t uc = 0;
	float f = 0.0, fx = 0.0, fy = 0.0;
	char *p = (char *)NULL;
	time_t t;
	double d/*, dx, dy*/ = 0.0;
	static char ts[64];
	struct tm *tm = NULL;
	struct buffer arg0 = INIT_BUFFER;
	struct buffer arg1 = INIT_BUFFER;
	struct buffer arg2 = INIT_BUFFER;
	char num_buf[64];
	char buf[1024];

#undef NAME
#define NAME "solve_function()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    /***
    bufcpy(&arg0, "");
    bufcpy(&arg1, "");
    bufcpy(&arg2, "");
    ***/

    while (1)
    {
        dq_token(token, &type);
        DEBUG (4, DEBUG_HDR, "Token is  <%s>, type <%s>\n", bufdata(token), show_type(type));
        if (is_function(type))
        {
            break;
        }

        if (type == TT_COMMA)
        {
            i_arg++;

        }
        else
        {
            q_token(token, type);
            iret = solve(token);
            if (iret != 0)
            {
                /* ERROR ("Error solving expression\n"); */
                break;
            }

            dq_token(token, &type);
            DEBUG (4, DEBUG_HDR, "Token is <%s>, type <%s>\n", bufdata(token), show_type(type));

            if (i_arg == 0)
            {
                bufncpy(&arg0, bufdata(token), bufsize(token));
            }
            else if (i_arg == 1)
            {
                bufncpy(&arg1, bufdata(token), bufsize(token));
            }
            else if (i_arg == 2)
            {
                bufncpy(&arg2, bufdata(token), bufsize(token));
            }
            else
            {
                ERROR ("Too many arguments for function\n");
                iret = 1; 
                break;
            }
        }
    }

    if (iret != 0)
    {
        buffree(&arg0);
        buffree(&arg1);
        buffree(&arg2);
        DEBUG (4, DEBUG_HDR, "Returning with error\n");
        return iret;
    }

    DEBUG (4, DEBUG_HDR, "Processing function (token type) <%s>\n", show_type(type));
    DEBUG (4, DEBUG_HDR, "Arg0 <%s>\n", bufdata(&arg0));
    DEBUG (4, DEBUG_HDR, "Arg1 <%s>\n", bufdata(&arg1));
    DEBUG (4, DEBUG_HDR, "Arg2 <%s>\n", bufdata(&arg2));

    DEBUG (3, DEBUG_HDR, "function() arg0 <%s>, arg1 <%s>, arg2 <%s>\n",
        bufdata(&arg0), bufdata(&arg1), bufdata(&arg2));

    if (g_syntax_check)
    {
        bufcpy(token, "0");
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_SQRT)
    {
        f = 0.0;
        sscanf(bufdata(&arg0), "%g", &f);
        d = sqrt((double)f);
        sprintf(num_buf, "%G", d);
        bufcpy(token, num_buf);
        DEBUG (2, DEBUG_HDR, "sqrt() <%s>\n", num_buf);
        q_token(token, TT_NUMBER);
    }
    else if (type == TT_STRFTIME)
    {
        if (bufsize(&arg1) == 0)
        {
            t = time(NULL);
            tm = localtime(&t);
            strftime(ts, sizeof(ts)-1, bufdata(&arg0), tm); 
      
        }
        else
        {
            f = 0.0;
            sscanf(bufdata(&arg0), "%u", &x);
            t = x;
            tm = localtime(&t);
            strftime(ts, sizeof(ts)-1, bufdata(&arg1), tm);
        }
        bufcpy(token, ts);
        DEBUG (2, DEBUG_HDR, "strftime() <%s>\n", ts);
        q_token(token, TT_STRING);

    }
    else if (type == TT_TEST)
    {
        bufprefix(&arg0, "test ");
        sysret = system(bufdata(&arg0));
        if (sysret == 0)
        {
            bufcpy(token, "1");
        }
        else
        {
            bufcpy(token, "0");
        }
        DEBUG (2, DEBUG_HDR, "<%d> = system(<%s>)\n", iret, bufdata(&arg0));
        iret = 0;
        q_token(token, TT_STRING);

    }
    else if (type == TT_TIME)
    {
        t = time(NULL);
        if (t == -1)
        {
            ERROR ("Can't get time\n");
            iret = 1;
        }   
        sprintf(buf, "%d", (int)t);
        bufcpy(token, buf);
        q_token(token, TT_STRING);

    }
    else if (type == TT_FORMAT)
    {
        f = 0.0;
        if (strchr(bufdata(&arg0), '%'))
        {
            if (sscanf(bufdata(&arg1), "%g", &f) == 1)
            {
                DEBUG (2, DEBUG_HDR, "Format is <%s>\n", bufdata(&arg0));
                sprintf(buf, bufdata(&arg0), f);
                bufcpy(token, buf);
                q_token(token, TT_NUMBER);

            }
            else
            {
                DEBUG (2, DEBUG_HDR, "Format is <%s>\n", bufdata(&arg0));
                sprintf(buf, bufdata(&arg0), bufdata(&arg1));
                bufcpy(token, buf);
                q_token(token, TT_NUMBER);
            }

        }
        else
        {
            bufprefix(&arg0, "%");
            if (sscanf(bufdata(&arg1), "%g", &f) == 1)
            {
                bufcat(&arg0, "g");
                DEBUG (2, DEBUG_HDR, "Format is <%s>\n", bufdata(&arg0));
                sprintf(buf, bufdata(&arg0), f);
                bufcpy(token, buf);
                q_token(token, TT_NUMBER);

            }
            else
            {
                bufcat(&arg0, "s");
                DEBUG (2, DEBUG_HDR, "Format is <%s>\n", bufdata(&arg0));
                sprintf(buf, bufdata(&arg0), bufdata(&arg1));
                bufcpy(token, buf);
                q_token(token, TT_NUMBER);
            }
        }

    }
    else if (type == TT_SCAN)
    {
        strcpy(buf, "");
        bufcat(&arg0, "%s"); 
        sscanf(bufdata(&arg1), bufdata(&arg0), buf);
        bufcpy(token, buf);
        q_token(token, TT_STRING);

    }
    else if (type == TT_STRLEN)
    {
        /* sprintf(num_buf, "%d", strlen(bufdata(&arg0))); */
        sprintf(num_buf, "%d", (bufsize(&arg0)));
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_TRACE)
    {
        g_trace = 0;
        sscanf(bufdata(&arg0), "%u", &g_trace);
        sprintf(num_buf, "%d", 0);
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_DUMP)
    {
        if (bufsize(&arg0))
        {
            variable_dump(token, bufdata(&arg0));

        }
        else
        {
            variable_dump_all(token);
        }
        dump_server(token);
        dump_handler(token);
        q_token(token, TT_STRING);

    }
    else if (type == TT_GET_VERSION)
    {
        bufcpy(token, SC_VERSION);
        q_token(token, TT_STRING);

    }
    else if (type == TT_GET_CLIENT)
    {
        sprintf(num_buf, "%d", get_client_fd(bufdata(&arg0)));
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_GET_ENV)
    {
        if (getenv(bufdata(&arg0)) == (char *)NULL)
        {
            bufcpy(token, "");
   
        }
        else
        {
            bufcpy(token, getenv(bufdata(&arg0)));
        }

        q_token(token, TT_STRING);

    }
    else if (type == TT_TOLOWER)
    {
        buftolower(&arg0);
        /* bufcpy(token, bufdata(&arg0)); */
        buf2bufcpy(token, &arg0);
        q_token(token, TT_STRING);

    }
    else if (type == TT_TOUPPER)
    {
        buftoupper(&arg0);
        /* bufcpy(token, bufdata(&arg0)); */
        buf2bufcpy(token, &arg0);
        q_token(token, TT_STRING);

    }
    else if (type == TT_HTOF)
    {
        if (bufsize(&arg1)) {  /* 2 arguments */
            sscanf(bufdata(&arg1), "%x", &x);
            /* swap((char *) &x, 4); */
            memcpy(&f, &x, 4);
            sprintf(num_buf, bufdata(&arg0), f);
        }
        else
        {
            sscanf(bufdata(&arg0), "%x", &x);
            /* swap((char *) &x, 4); */
            memcpy(&f, &x, 4);
            sprintf(num_buf, "%f", f);
        }
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_FTOH)
    {
    	union F4Union fu;
        sscanf(bufdata(&arg0), "%g", &fu.the_float);
//        p = (unsigned char *) &f;
//        swap((char *) p, 4);
//        sprintf(buf, "%2.2X%2.2X%2.2X%2.2X", (unsigned char) *p, (unsigned char) *(p+1),
//            (unsigned char) *(p+2), (unsigned char) *(p+3));
        swap(fu.the_bytes, sizeof(fu.the_bytes));
        sprintf(buf, "%2.2X%2.2X%2.2X%2.2X", (unsigned char) fu.the_bytes[0], (unsigned char) fu.the_bytes[1],
            (unsigned char) fu.the_bytes[2], (unsigned char) fu.the_bytes[3]);
        bufcpy(token, buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_SWAP)
    {
        sscanf(bufdata(&arg0), "%u", &j);
        for (uint32_t i=0; i<j; i++)
        {
            memcpy(&buf[(j*2)-(i*2)-1], bufdata(&arg1)+(i*2), 2);
        }
        bufcpy(token, buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_HTOI)
    {
        uint64_t ul = (uint64_t)strtol(bufdata(&arg0), &p, 16);
        if (*p)
        {
            ERROR ("Can't be used in HTOI <%s>\n", bufdata(&arg0));
            bufcpy(token, "");
            iret = 1;

        }
        else
        {
            sprintf(num_buf, "%lu", ul);
            bufcpy(token, num_buf);
            q_token(token, TT_NUMBER);
        }

    }
    else if (type == TT_OTOI)
    {
    	uint64_t ul = (uint64_t)strtol(bufdata(&arg0), &p, 8);
        if (*p)
        {
            ERROR ("Can't be used in OTOI <%s>\n", bufdata(&arg0));
            bufcpy(token, "");
            iret = 1;

        }
        else
        {
            sprintf(num_buf, "%lu", ul);
            bufcpy(token, num_buf);
            q_token(token, TT_NUMBER);
        }
    }
    else if (type == TT_BTOI)
    {
        uint64_t ul = (uint64_t)strtol(bufdata(&arg0), &p, 2);
        if (*p)
        {
            ERROR ("Can't be used in BTOI <%s>\n", bufdata(&arg0));
            bufcpy(token, "");
            iret = 1;

        }
        else
        {
            sprintf(num_buf, "%lu", ul);
            bufcpy(token, num_buf);
            q_token(token, TT_NUMBER);
        }
    }
    else if (type == TT_ITOH)
    {
        bufprefix(&arg0, "%");
        bufcat(&arg0, "X");
        x = 0;
        sscanf(bufdata(&arg1), "%u", &x);
        sprintf(buf, bufdata(&arg0), x);
        bufcpy(token, buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_ITOO)
    {
        bufprefix(&arg0, "%");
        bufcat(&arg0, "o");
        x = 0;
        sscanf(bufdata(&arg1), "%u", &x);
        sprintf(buf, bufdata(&arg0), x);
        bufcpy(token, buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_ITOA)
    {
    	// TODO Don't understand this...are we reading a byte only?
    	int32_t tmp = 0;
        sscanf(bufdata(&arg0), "%d", &tmp);
        if (isprint(tmp) && (tmp <= 255))
        {
            sprintf(buf, "%c", tmp);

        }
        else
        {
            sprintf(buf, "%c", '?');
        }
        bufcpy(token, buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_ITOB)
    {
    	// TODO Don't understand this...are we reading a byte only?
        bufprefix(&arg0, "%");
        bufcat(&arg0, "X");
        int32_t tmp = 0;
//        x = 0;
        sscanf(bufdata(&arg1), "%d", &tmp);
        sprintf(buf, bufdata(&arg0), tmp);
        bufcpy(token, "");
        for (uint32_t i=0; i<(uint32_t)strlen(buf); i++)
        {
            uc = (uint8_t)buf[i];
            if (uc == ' ')
            {
                uc = 0;

            }
            else if (uc <= '9')
            {
                uc = uc & 15;

            }
            else
            {
                uc = (uint8_t)((uc & 15) + 9);
            }
               
            for (j=0; j<4; j++)
            {
               if (uc & 8)
               {
                   bufcat(token, "1");

               }
               else
               {
                   bufcat(token, "0");
               }
               uc = (uint8_t)(uc << 1);
            }
        }
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_INSTRING)
    {
        if (has_null(&arg0))
        {
            ERROR ("instring will not work because of imbedded nulls in <%s>\n", 
                strmkprint(bufdata(&arg0), bufsize(&arg0)));
        }

        if (has_null(&arg1))
        {
            ERROR ("instring will not work because of imbedded nulls in <%s>\n", 
                strmkprint(bufdata(&arg1), bufsize(&arg1)));
        }

        p = strstr(bufdata(&arg1), bufdata(&arg0));
        if (p == (char *)NULL)
        {
            bufcpy(token, "0");
        }
        else
        {
        	char *cp = strstr(bufdata(&arg1), bufdata(&arg0));
            sprintf(num_buf, "%d", (int)(cp - bufdata(&arg1)) + 1);
//            x = (int) strstr(bufdata(&arg1), bufdata(&arg0)) - (int) bufdata(&arg1);
//            x++;
//            sprintf(num_buf, "%d", x);
            bufcpy(token, num_buf);
        }
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_IS_LABEL)
    {
        iret = variable_get(bufdata(&arg0), token, VT_LABEL);
        if (iret)
        {
            sprintf(num_buf, "%d", 0);

        }
        else
        {
            sprintf(num_buf, "%d", 1);
        }
        iret = 0;  /* Not an error */
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_IS_VAR)
    {
        iret = variable_get(bufdata(&arg0), token, VT_USER);
        if (iret)
        {
            sprintf(num_buf, "%d", 0);

        }
        else
        {
            sprintf(num_buf, "%d", 1);
        }
        iret = 0;  /* Not an error */
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_IS_TMP_VAR)
    {
        iret = tmp_variable_get(bufdata(&arg0), token);
        if (iret)
        {
            sprintf(num_buf, "%d", 0);
        }
        else
        {
            sprintf(num_buf, "%d", 1);
        }
        iret = 0;  /* Not an error */
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_DEL_TMP_VAR)
    {
        tmp_variable_delete_all();
        bufcpy(token, "0");
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_SORT_TMP_VAR)
    {
        tmp_variable_sort();
        bufcpy(token, "0");
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_ADD_TMP_VAR)
    {
        tmp_variable_add(bufdata(&arg1), bufdata(&arg0), (int)bufsize(&arg0));
        bufcpy(token, "0");
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_SET_TMP_VAR)
    {
        tmp_variable_put(bufdata(&arg1), bufdata(&arg0), bufsize(&arg0));
        bufcpy(token, "0");
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_GET_TMP_VAR)
    {
        if (bufsize(&arg0) == 0)
        {
            tmp_variable_get_all(token);

        }
        else if (bufsize(&arg1) == 0)
        {
            /* No default value supplied */
            tmp_variable_get(bufdata(&arg0), token);
        }
        else
        {
            /* Default value supplied */
            iret = tmp_variable_get(bufdata(&arg1), token);
            if (iret)
            {
                iret = 0;  /* Not an error */
                bufcpy(token, bufdata(&arg0));
            }
        }
        q_token(token, TT_IDENTIFIER);

    }
    else if (type == TT_SUBSTR)
    {
        y = 0;  /* Number of chars to copy */
        sscanf(bufdata(&arg0), "%u", &y);

        x = 0;  /* Index */
        sscanf(bufdata(&arg1), "%u", &x);

        /* Need to make sure not to extend beyond the value */
        if (y > (bufsize(&arg2) - x))
        {
            /* Will extend beyond the end, trim the length */
            y = (bufsize(&arg2) - x) + 1;  /* Then trim some off the cnt */
        }
 
        DEBUG (3, DEBUG_HDR, "SUBSTR str <%s>, idx <%d>, cnt <%d>\n", 
            bufdata(&arg2), x, y);
//        if ((x >= 1) && (y >= 0))
        if (x >= 1)
        {
            bufncpy(token, bufdata(&arg2)+x-1, y); 

        }
        else
        {
            ERROR ("Invalid values for substr(<%s>, <%d>, <%d>)\n", 
                bufdata(&arg2), x, y);
        }
        q_token(token, TT_STRING);

    }
    else if (type == TT_SYSTEM_SHELL)
    {
        sysret = system(bufdata(&arg0));
        sysret = sysret >> 8;
        sprintf(num_buf, "%d", sysret);  /* Return the system() iret value */
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);
        iret = 0;  /* Make so that this command will not fail */
    }
    else if (type == TT_SYSTEM)
    {
        if (i_arg == 0)
        {  /* Only one arg */
            bufcat(&arg0, " 2>&1");  /* Append redirection of stderr to stdout */
            fp = (FILE *) popen(bufdata(&arg0), "r");
            if (fp == (FILE *)NULL)
            {
                ERROR ("Can't get reply from system command <%s>\n", bufdata(&arg0));
                bufcpy(token, "");
                iret = 1;
            }
            bufcpy(token, "");
            while ((fgets(buf, sizeof(buf), fp) != (char *)NULL))
            {
                data_to_char(token, buf, (uint32_t)strlen(buf));  /* Convert data chars to regular chars */
            } 

            sysret = pclose(fp);
            if (sysret != 0)
            {
                /* Per popen() doc, success is determined at pclose() time */
                ERROR ("System command <%s> failed <%d>\n", bufdata(&arg0), iret);
                bufcpy(token, "");
                iret = 1;
            }
            else
            {
            	iret = 0;
                make_tmp_variables(bufdata(token), "", 1); 
                bufcpy(token, "0");
                q_token(token, TT_NUMBER);
            }

        }
        else if (i_arg == 1)
        {  /* 2 args */
            bufcat(&arg1, " 2>&1");  /* Append redirection of stderr to stdout */
            fp = (FILE *) popen(bufdata(&arg1), "r");
            if (fp == (FILE *)NULL)
            {
                ERROR ("Can't get reply from system command <%s>\n", bufdata(&arg1));
                bufcpy(token, "");
                iret = 1;
            }

            bufcpy(token, "");
            buf[0] = 0;
            while ((fgets(buf, sizeof(buf), fp) != (char *)NULL))
            {
                data_to_char(token, buf, (uint32_t)strlen(buf));  /* Convert data chars to regular chars */
            } 

            sysret = pclose(fp);
            if (sysret != 0)
            {
                /* Per popen() doc, success is determined at pclose() time */
                ERROR ("System command <%s> failed <%d>\n", bufdata(&arg1), iret);
                bufcpy(token, "");
                iret = 1;
            }
            else
            {
                make_tmp_variables(bufdata(token), bufdata(&arg0), 1);
                bufcpy(token, "0");
                q_token(token, TT_NUMBER);
                iret = 0;
            }
        }
        else
        {
            ERROR ("Wrong number of arguments\n");
        }

    }
    else if (type == TT_POW)
    {
        fy = 0.0;
        sscanf(bufdata(&arg0), "%g", &fy);

        fx = 0.0;
        sscanf(bufdata(&arg1), "%g", &fx);

        d = pow((double)fx, (double)fy);
        DEBUG (2, DEBUG_HDR, "<%G>=pow(<%G>, <%G>)\n", d, fx, fy);
        sprintf(num_buf, "%G", d);
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_ABS)
    {
        x = 0;
        sscanf(bufdata(&arg0), "%u", &x);
//        x = abs(x);
        sprintf(num_buf, "%u", x);
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_MOD)
    {
    	// TODO check to make sure conversion to unsigned is not a problem
        y = 0;
        sscanf(bufdata(&arg0), "%u", &y);

        if (y == 0)
        {
            ERROR ("Divide by zero detected\n");
            bufcpy(token, "");
            return 1;
        }

        x = 0;
        sscanf(bufdata(&arg1), "%u", &x);
 
        DEBUG (3, DEBUG_HDR, "<%d>=mod(<%d>,<%d>)\n", x%y, x, y);
        sprintf(num_buf, "%d", x%y);
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_IMPORT)
    {
        flag = false;  /* binary flag */
        if (strchr(bufdata(&arg1), 'b') != (char *)NULL)
        {  /* Check if binary data */
            flag = true;  /* This is binary data */
        }

        if ((fp = fopen(bufdata(&arg2), bufdata(&arg1))) == (FILE *)NULL)
        {
            ERROR ("Can't open file <%s>\n", bufdata(&arg2));
            iret = 1;
        }
        else
        {
            bufcpy(&arg1, "");
            j = 0;
            // TODO buf is only 1024 at the moment...a problem?
            memset(buf, '\0', sizeof(buf));
            while (1)
            {
            	int character = 0;
            	character = fgetc(fp);
                if (character == EOF)
                {
                    break;
                }
                else
                {
                    j++;
                    if (flag == true)
                    {  /* Binary data */
                        /* sprintf(buf, "%2.2X", x); */
                        uc = (uint8_t)(character / 16);
                        if (uc < 10)
                        {
                            buf[0] = (char)(uc + '\x30');
                        }
                        else
                        {
                            buf[0] = (char)(uc + '\x37');
                        }

                        uc = (uint8_t)(character % 16);
                        if (uc < 10)
                        {
                            buf[1] = (char)(uc + '\x30');
                        }
                        else
                        {
                            buf[1] = (char)(uc + '\x37');
                        }
                        bufcat(&arg1, buf); 
                    }
                    else
                    {
                        /* sprintf(buf, "%c", x); */
                        uc = (uint8_t)character;
                        bufncat(&arg1, (char *) &uc, 1);
                    }
                }
            }
            fclose(fp);
            variable_put(bufdata(&arg0), bufdata(&arg1), bufsize(&arg1), VT_USER);
            sprintf(num_buf, "%d", j);
            bufcpy(token, num_buf);
            q_token(token, TT_NUMBER);
        }
    }
    else if (type == TT_EXPORT)
    {
        flag = false;  /* binary flag */
        if (strchr(bufdata(&arg1), 'b') != (char *)NULL)
        {  /* Check if binary data */
            flag = true;  /* This is binary data */
        }

        if ((fp = fopen(bufdata(&arg2), bufdata(&arg1))) == 0)
        {
            ERROR ("Can't open file <%s>\n", bufdata(&arg2));
            iret = 1;
        }
        else
        {
            p = bufdata(&arg0);
            memset(buf, '\0', sizeof(buf));
            while (*p != '\0')
            {
                if (flag == true)
                {  /* Binary data */
                    /* sscanf(p, "%2x", &j); The following is much faster */
                    memcpy(buf, p, 2);
                    x = (uint32_t)strtol(buf, (char**) NULL, 16);
                    fputc((uint8_t) x, fp);
                    p++;
                    p++;

                }
                else
                {
                    fputc(*p, fp);
                    p++;
                }
            }
            fclose(fp);
            bufcpy(token, "");
            q_token(token, TT_NUMBER);
        }

    }
    else if (type == TT_FILE_OWC)
    {
        fp = 0;
        if ((fp = fopen(bufdata(&arg2), bufdata(&arg1))) == 0)
        {
            ERROR ("Can't open file <%s>\n", bufdata(&arg2));
            iret = 1;

        }
        else
        {
            DEBUG (4, DEBUG_HDR, "File opened <%s>\n", bufdata(&arg2));
            if (fputs(bufdata(&arg0), fp) < 0)
            {
                ERROR ("Error while writing to file\n");
                bufcpy(token, "");
                iret = 1;

            }
            else
            {
                fputs("\n", fp);  /* Add a newline on the end */
                DEBUG (4, DEBUG_HDR, "Wrote <%s> to file\n", bufdata(&arg0));
                fclose(fp);
                DEBUG (4, DEBUG_HDR, "File closed\n");
                sprintf(num_buf, "%d", 0);
                bufcpy(token, num_buf);
                q_token(token, TT_NUMBER);
            }
        }

    }
    else if (type == TT_FILE_OPEN)
    {
        fp = (FILE *)NULL;
        if ((fp = fopen(bufdata(&arg1), bufdata(&arg0))) == 0)
        {
            ERROR ("Can't open file <%s>\n", bufdata(&arg1));
            iret = 1;
        }
        else
        {
            DEBUG (4, DEBUG_HDR, "File opened <%s>\n", bufdata(&arg1));
//            sprintf(num_buf, "%d", fp);
            sprintf(num_buf, "%p", (void *)fp);
            bufcpy(token, num_buf);
            q_token(token, TT_NUMBER);
        }

    }
    else if (type == TT_FILE_CLOSE)
    {
        fp = (FILE *)NULL;
//        sscanf(bufdata(&arg0), "%d", &fp);
        sscanf(bufdata(&arg0), "%p", (void **)&fp);
        fclose(fp);
        DEBUG (4, DEBUG_HDR, "File closed\n");
        sprintf(num_buf, "%d", 0);
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_FILE_WRITE)
    {
        fp = (FILE *)NULL;
//        sscanf(bufdata(&arg1), "%d", &fp);
        sscanf(bufdata(&arg1), "%p", (void **)&fp);
        if (fputs(bufdata(&arg0), fp) < 0)
        {
            ERROR ("Error while writing to file\n");
            bufcpy(token, "");
            iret = 1;

        }
        else
        {
            fputs("\n", fp);  /* Add a newline on the end */
            DEBUG (4, DEBUG_HDR, "Wrote <%s> to file\n", bufdata(&arg0));
            sprintf(num_buf, "%d", 0);
            bufcpy(token, num_buf);
            q_token(token, TT_NUMBER);
        }

    }
    else if (type == TT_PARSE)
    {
        j = 0;
        bufcpy(token, "");
        if (bufsize(&arg2))
        {  /* 3 arguments */
            p = strtok(bufdata(&arg2), bufdata(&arg1));
            while(p != (char *)NULL)
            {
                j++;
                sprintf(num_buf, "[%d]", j);
                bufcpy(token, bufdata(&arg0));
                bufcat(token, num_buf);
                variable_put(bufdata(token), p, 0, VT_USER);
                p = strtok('\0', bufdata(&arg1));
            }

        }
        else
        {  /* Only 2 arguments, only get the count */
            p = strtok(bufdata(&arg1), bufdata(&arg0));
            while(p != (char *)NULL)
            {
                j++;
                p = strtok('\0', bufdata(&arg0));
            }
        }

        sprintf(num_buf, "%d", j);
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_T_PARSE)
    {
        j = 0;
        bufcpy(token, "");
        p = strtok(bufdata(&arg2), bufdata(&arg1));
        while(p != (char *)NULL)
        {
            j++;
            sprintf(num_buf, "[%d]", j);
            bufcpy(token, bufdata(&arg0));
            bufcat(token, num_buf);
            tmp_variable_put(bufdata(token), p, (uint32_t)strlen(p));
            p = strtok('\0', bufdata(&arg1));
        }

        sprintf(num_buf, "%d", j);
        bufcpy(token, num_buf);
        q_token(token, TT_NUMBER);

    }
    else if (type == TT_FILE_READ)
    {
        fp = (FILE *)NULL;
//        sscanf(bufdata(&arg0), "%d", &fp);
        sscanf(bufdata(&arg0), "%p", (void **)&fp);
        bufcpy(token, "");
        while (1)
        {
            if (fgets(buf, sizeof(buf), fp) == (char *)NULL)
            {
                bufcpy(token, "EOF");
                DEBUG (4, DEBUG_HDR, "Found end of file\n");
                break;

            }
            else
            {
                DEBUG (4, DEBUG_HDR, "Read <%s> from file\n", buf);
//                if (strlen(buf) == sizeof(buf)-1)
//                {
//                    bufcat(token, buf);
//                    continue;
//
//                }
//                else
                {
//                    bufncat(token, buf, (uint32_t)(strlen(buf)-1));
                    bufncat(token, buf, (uint32_t)strlen(buf));
                    break;
                }
            }
        }
        DEBUG (4, DEBUG_HDR, "All data read <%s> from file\n", bufdata(token));
        q_token(token, TT_STRING);
    }
    else
    {
    	;
    }
 
    buffree(&arg0);
    buffree(&arg1);
    buffree(&arg2);
    DEBUG (4, DEBUG_HDR, "Returning token <%s>, type <%s>\n", bufdata(token), show_type(type));
    return iret;
}

/********************************************/

static uint32_t ones_complement(struct buffer *token)
{
	// TODO see if i should be unsigned
	int i = 0;
	uint32_t iret = 1;
	char num_buf[64];

#undef NAME
#define NAME "ones_complement()"

	num_buf[0] = 0;
    if (isint(bufdata(token)))
    {
        sscanf(bufdata(token), "%d", &i);
        i = ~i;
        sprintf(num_buf, "%d", i); 
        bufcpy(token, num_buf); 
        iret = 0;

    }
    else
    {
        ERROR ("Not a valid integer for one's complement <%s>\n", bufdata(token));
        iret = 1;
    }

    return iret;
}

/********************************************/

static uint32_t add_sub_mult_div(enum token_types type, char *arg1, char *arg2, struct buffer *token)
{
	char num_buf[64];

#undef NAME
#define NAME "add_sub_mult_div()"

	num_buf[0] = 0;

    DEBUG (4, DEBUG_HDR, "Called, op <%s>,  arg1 <%s>, arg2 <%s>\n", 
        show_type(type), arg1, arg2);

    if (g_syntax_check)
    {
        sprintf(num_buf, "%d", 0); 
        bufcpy(token, num_buf); 
        return 0;
    }

    if (!isnumber(arg1))
    {
        WARNING ("Not a valid number <%s>.  Will be converted to 0.\n", arg1);
    }

    if (!isnumber(arg2))
    {
        WARNING ("Not a valid number <%s>.  Will be converted to 0.\n", arg2);
    }

    if (isint(arg1) && isint(arg2))
    {
    	int i1 = 0;
    	int i2 = 0;
    	int i3 = 0;

        DEBUG (4, DEBUG_HDR, "Doing int math\n");
        sscanf(arg1, "%d", &i1);
    
        sscanf(arg2, "%d", &i2);

        if (type == TT_PLUS)
        {
            i3 = i1 + i2; 
            sprintf(num_buf, "%d", i3); 
            DEBUG (4, DEBUG_HDR, "<%d> + <%d>\n", i1, i2);

        }
        else if (type == TT_MINUS)
        {
            i3 = i2 - i1; 
            sprintf(num_buf, "%d", i3); 
            DEBUG (4, DEBUG_HDR, "<%d> - <%d>\n", i2, i1);

        }
        else if (type == TT_MULT)
        {
            i3 = i1 * i2; 
            sprintf(num_buf, "%d", i3); 
            DEBUG (4, DEBUG_HDR, "<%d> * <%d>\n", i1, i2);
 
        }
        else if (type == TT_DIVIDE)
        {
            if (i1 == 0)
            {
                ERROR ("Divide by zero detected\n");
                bufcpy(token, "");
                return 1;
            }
            i3 = i2 / i1; 
            sprintf(num_buf, "%d", i3); 
            DEBUG (4, DEBUG_HDR, "<%d> / <%d>\n", i2, i1);

        }
        else
        {
        	;
        }
        DEBUG (4, DEBUG_HDR, "Result i3 (%d), num_buf <%s>\n", i3, num_buf);

    }
    else
    {
    	float f1 = 0.0;
    	float f2 = 0.0;
    	float f3 = 0.0;

        DEBUG (4, DEBUG_HDR, "Doing float math\n");
        f1 = 0;
        sscanf(arg1, "%g", &f1);
    
        f2 = 0;
        sscanf(arg2, "%g", &f2);

        if (type == TT_PLUS)
        {
            f3 = f1 + f2; 
            sprintf(num_buf, "%G", f3); 
            DEBUG (4, DEBUG_HDR, "<%g> + <%g>\n", f1, f2);

        }
        else if (type == TT_MINUS)
        {
            f3 = f2 - f1; 
            sprintf(num_buf, "%G", f3); 
            DEBUG (4, DEBUG_HDR, "<%g> - <%g>\n", f2, f1);

        }
        else if (type == TT_MULT)
        {
            f3 = f1 * f2; 
            sprintf(num_buf, "%G", f3); 
            DEBUG (4, DEBUG_HDR, "<%g> * <%g>\n", f1, f2);
 
        }
        else if (type == TT_DIVIDE)
        {
            if (f1 == 0)
            {
                ERROR ("Divide by zero detected\n");
                bufcpy(token, "");
                return 1;
            }
            f3 = f2 / f1; 
            sprintf(num_buf, "%G", f3); 
            DEBUG (4, DEBUG_HDR, "<%g> / <%g>\n", f2, f1);

        }
        else
        {
        	;
        }
        DEBUG (4, DEBUG_HDR, "Result f3 (%g), num_buf <%s>\n", f3, num_buf);
    }
   
    bufcpy(token, num_buf); 

    return 0;
}

/********************************************/

static void concat(struct buffer *arg1, struct buffer *arg2, struct buffer *token, char *delimiter)
{
#undef NAME
#define NAME "concat()"

    DEBUG (4, DEBUG_HDR, "Called,  arg1 <%s>, arg2 <%s>, delimiter <%s>\n", 
        bufdata(arg1), bufdata(arg2), delimiter);

    bufncpy(token, bufdata(arg2), bufsize(arg2)); 
    bufcat(token, delimiter); 
    bufncat(token, bufdata(arg1), bufsize(arg1)); 

    DEBUG (4, DEBUG_HDR, "Returning concat-ed token <%s>\n", bufdata(token));
    return;
}

/********************************************/

static uint32_t logical_bitwise(enum token_types type, struct buffer *arg1,
    struct buffer *arg2, struct buffer *token)
{
	char num_buf[64];

#undef NAME
#define NAME "logical_bitwise()"

	num_buf[0] = 0;

    DEBUG (4, DEBUG_HDR, "Called, type <%s>,  arg1 <%s>, arg2 <%s>\n", 
        show_type(type), bufdata(arg1), bufdata(arg2));

    if (g_syntax_check)
    {
        sprintf(num_buf, "%d", 0); 
        bufcpy(token, num_buf); 
        return 0;
    }

    if (!isnumber(bufdata(arg1)))
    {
        WARNING ("Not a valid number <%s>.  Will be converted to 0.\n", bufdata(arg1));
    }

    if (!isnumber(bufdata(arg2)))
    {
        WARNING ("Not a valid number <%s>.  Will be converted to 0.\n", bufdata(arg2));
    }

    if (isint(bufdata(arg1)) && isint(bufdata(arg2)))
    {
    	// TODO probably should force type to int32_t at least
    	int i1 = 0;
    	int i2 = 0;

        DEBUG (4, DEBUG_HDR, "Doing int math\n");
        i2 = 0;
        sscanf(bufdata(arg1), "%d", &i2);
    
        i1 = 0;
        sscanf(bufdata(arg2), "%d", &i1);

        DEBUG (4, DEBUG_HDR, "i1 <%d>, i2 <%d>\n", i1, i2);

        if (type == TT_LOGICAL_AND)
        {
            sprintf(num_buf, "%d", i1 && i2); 

        }
        else if (type == TT_LOGICAL_OR)
        {
            sprintf(num_buf, "%d", i1 || i2); 

        }
        else if (type == TT_BITWISE_AND)
        {
            sprintf(num_buf, "%d", i1 & i2); 

        }
        else if (type == TT_BITWISE_OR)
        {
            sprintf(num_buf, "%d", i1 | i2); 

        }
        else if (type == TT_BITWISE_XOR)
        {
            sprintf(num_buf, "%d", i1 ^ i2); 

        }
        else if (type == TT_LESS_THAN)
        {
            sprintf(num_buf, "%d", i1 < i2 ? 1 : 0); 
        }
        else if (type == TT_GREATER_THAN)
        {
            sprintf(num_buf, "%d", i1 >i2 ? 1 : 0); 

        }
        else if (type == TT_EQUAL)
        {
            sprintf(num_buf, "%d", i1 == i2 ? 1 : 0); 

        }
        else if (type == TT_NOT_EQUAL)
        {
        sprintf(num_buf, "%d", i1 != i2 ? 1 : 0); 

        }
        else if (type == TT_GREATER_THAN_EQUAL)
        {
            sprintf(num_buf, "%d", i1 >= i2 ? 1 : 0); 

        }
        else if (type == TT_LESS_THAN_EQUAL)
        {
            sprintf(num_buf, "%d", i1 <= i2 ? 1 : 0); 

        }
        else
        {
        	;
        }
    }
    else
    {
    	float f1 = 0.0;
    	float f2 = 0.0;

        DEBUG (4, DEBUG_HDR, "Doing float math\n");
        f2 = 0;
        sscanf(bufdata(arg1), "%g", &f2);
    
        f1 = 0;
        sscanf(bufdata(arg2), "%g", &f1);

        DEBUG (4, DEBUG_HDR, "f1 <%g>, f2 <%g>\n", f1, f2);

        if (type == TT_LOGICAL_AND)
        {
            sprintf(num_buf, "%G", (double)(f1 && f2));

        }
        else if (type == TT_LOGICAL_OR)
        {
            sprintf(num_buf, "%G", (double)(f1 || f2));

        }
        else if (type == TT_BITWISE_AND)
        {
            /* sprintf(num_buf, "%G", f1 & f2); */
            ERROR ("Floats can't be used with bitwise and-ing <%s> and <%s>\n", 
                bufdata(arg2), bufdata(arg1));
            bufcpy(token, "");
            return 1;
        }
        else if ((type == TT_BITWISE_OR) || (type == TT_BITWISE_XOR))
        {
            /* sprintf(num_buf, "%G", f1 | f2); */
            ERROR ("Floats can't be used with bitwise or-ing <%s> and <%s>\n", 
                bufdata(arg2), bufdata(arg1));
            bufcpy(token, "");
            return 1;
        }
        else if (type == TT_LESS_THAN)
        {
            sprintf(num_buf, "%d", f1 < f2 ? 1 : 0); 
        }
        else if (type == TT_GREATER_THAN)
        {
            sprintf(num_buf, "%d", f1 > f2 ? 1 : 0); 
        }
        else if (type == TT_EQUAL)
        {
            sprintf(num_buf, "%d", f1 == f2 ? 1 : 0); 
        }
        else if (type == TT_NOT_EQUAL)
        {
        	sprintf(num_buf, "%d", f1 != f2 ? 1 : 0);
        }
        else if (type == TT_GREATER_THAN_EQUAL)
        {
            sprintf(num_buf, "%d", f1 >= f2 ? 1 : 0); 
        }
        else if (type == TT_LESS_THAN_EQUAL)
        {
            sprintf(num_buf, "%d", f1 <= f2 ? 1 : 0); 
        }
        else
        {
        	;
        }
    }
    bufcpy(token, num_buf); 
    DEBUG (4, DEBUG_HDR, "Result (%s)\n", bufdata(token));

    return 0;
}

/********************************************/

static void str_cmp(enum token_types type, const struct buffer *arg1, const struct buffer *arg2, struct buffer *token)
{
	int x = 0;

#undef NAME
#define NAME "str_cmp()"

    DEBUG (4, DEBUG_HDR, "Called, type <%s>,  arg1 <%s>, arg2 <%s>\n", show_type(type), bufdata(arg1), bufdata(arg2));

    x = strcmp(bufdata(arg2), bufdata(arg1));

    if (type == TT_STR_EQUAL)
    {
        if (x)
        {
            bufcpy(token, "0"); 
        }
        else
        {
            bufcpy(token, "1"); 
        }
    }
    else if (type == TT_STR_NOT_EQUAL)
    {
        if (x)
        {
            bufcpy(token, "1"); 
        }
        else
        {
            bufcpy(token, "0"); 
        }

    }
    else
    {
    	;
    }

    return;
}

/********************************************/

static void q_token(const struct buffer *token, enum token_types type)
{
	uint32_t length = 0;
	char *p = (char *)NULL;

#undef NAME
#define NAME "q_token()"

    DEBUG (4, DEBUG_HDR, "Called, token is <%s>, type <%s>, q_idx <%d>\n", 
        bufdata(token), show_type(type), q_idx);
    if (q_idx >= MAX_Q)
    {
        FATAL ("Expression queue overflow\n");
    }

    q_of_tokens[q_idx] = (char *) malloc(bufsize(token) + sizeof(length));   

    if (q_of_tokens[q_idx] == NULL)
    {
        FATAL ("Can't malloc more space\n");
    }

    length = bufsize(token);

    p = q_of_tokens[q_idx];
    memcpy(p, &length, sizeof(length));
    p+=sizeof(length);
    memcpy(p, bufdata(token), length);

    q_of_types[q_idx] = type;
    q_idx++;

    DEBUG (4, DEBUG_HDR, "Returning, q_idx <%d>\n", q_idx);
    return;
}

/********************************************/

static void dq_token(struct buffer *token, enum token_types *type)
{
	uint32_t length = 0;
	char *p = (char *)NULL;

#undef NAME
#define NAME "dq_token()"

    if (q_idx == 0)
    {
        DEBUG (4, DEBUG_HDR, "Expression queue is empty\n");
        bufcpy(token, "");
        *type = TT_NULL_TOKEN;
        DEBUG (4, DEBUG_HDR, "Returning token <%s>, type <%s>, q_idx <%d>\n", 
            bufdata(token), show_type(*type), q_idx);
        return;
    }

    q_idx--;
    p = q_of_tokens[q_idx];
    memcpy(&length, p, sizeof(length)); 
    p+=sizeof(length);
    bufncpy(token, p, length);

    *type = q_of_types[q_idx];
    free(q_of_tokens[q_idx]);
    q_of_tokens[q_idx] = '\0';

    DEBUG (4, DEBUG_HDR, "Returning token <%s>, type <%s>, q_idx <%d>\n", 
        bufdata(token), show_type(*type), q_idx);
    return;
}

/********************************************/

static void q_init()
{
#undef NAME
#define NAME "q_init()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    q_idx = 0;

    for (int i=0; i<MAX_Q; i++)
    {
        q_of_tokens[i] = '\0';
        q_of_types[i] = 0;
    }
}

/********************************************/

static void q_free()
{
#undef NAME
#define NAME "q_free()"

    DEBUG (4, DEBUG_HDR, "Called\n");

    for (int i=0; i<MAX_Q; i++)
    {
        if (q_of_tokens[i] != (char *)NULL)
        {
            free(q_of_tokens[i]);
        }
        q_of_types[i] = 0;
    }
}

/********************************************/

static int isint(const char *p)
{
	int i = 0;
	int iret = 0;
	char junk[32];

    junk[0] = '\0';

    iret = sscanf(p, "%d%10s", &i, junk);
    if ((iret == 1) && (strlen(junk) == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/********************************************/
static int isnumber(const char *p)
{
	int iret = 0;;
	float f = 0.0;
	char junk[32];

    junk[0] = '\0';

    iret = sscanf(p, "%g%10s", &f, junk);
    if ((iret == 1) && (strlen(junk) == 0))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/********************************************/

//static void q_dump()
//{
//int i, j, l;
//char *p;
//#undef NAME
//#define NAME "q_dump()"
//
//    printf ("Queue dump, num items <%d>\n", q_idx);
//    for (i=0; i<q_idx; i++) {
//        p = q_of_tokens[i];
//        memcpy(&l, p, sizeof(l));
//        printf ("  Len <%d> \"", l);
//        p+=sizeof(l);
//        for (j=0; j<l; j++) {
//            if (isprint(*p)) {
//                printf ("%c ", *p);
//            } else {
//                printf ("{%.2X} ", *p);
//            }
//            p++;
//        }
//        printf ("\" type <%s>\n",show_type(q_of_types[i]));
//    }
//
//#ifdef oldcode
//    if (g_debug >= 5) {
//        DEBUG (4, DEBUG_HDR, "Queue dump\n");
//        for (i=0; i<q_idx; i++) {
//            DEBUG (4, DEBUG_HDR, "<%s> <%s>\n", q_of_tokens[i], show_type(q_of_types[i]));
//        }
//    }
//#endif
//}

/********************************************/

static bool has_null(const struct buffer *token)
{
	char *p = (void *)NULL;

#undef NAME
#define NAME "has_null()"

    p = bufdata(token);
    for (uint32_t i=0; i<bufsize(token); i++)
    {
        if (*p == '\0')
        {
            return true;
        }
        p++;
    }
    return false;
}

