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
#include <stdlib.h>
#include <stdint.h>
#include "include.h"


extern void log_trace(const char *fmt, ...);

struct variable
{
    char *name;
    char *value;
    uint32_t length;
    uint32_t hash;
};

//static struct variable *p_idx 			= (struct variable *)NULL;
static struct variable *variable_root 	= (struct variable *)NULL;
static uint32_t			variable_count 	= 0;

static int name_cmp(const void *, const void *);
static uint32_t get_hash(const char *);

/**********************************************/

void tmp_variable_put(const char *name, const char *value, uint32_t length)
{
	uint32_t size = 0;;
	uint32_t idx = 0;
/*char c;*/
//struct variable *v, tmp, a, b, *ret;
	struct variable tmp, a, *ret;
	struct buffer tmp_name = INIT_BUFFER;
	struct buffer tmp_value = INIT_BUFFER;

#undef NAME
#define NAME "tmp_variable_put()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, value <%s>\n", name, value);

    bufcpy(&tmp_value, value);
    bufcpy(&tmp_name, name);

    a.name = bufdata(&tmp_name);
    a.hash = get_hash(a.name);
//    b.name = "";
//    b.hash = 0;

    /* if (length == 0) { */
//    if (length == -1)
//    {
//        length = strlen(value);
//    }
//
    if (variable_count == 0)
    {
        DEBUG (2, DEBUG_HDR, "    First put ever, create new variable\n"); 
        variable_root = (struct variable *) malloc(sizeof(struct variable));

        /* Name */
        variable_root[0].name = malloc(bufsize(&tmp_name)+1);
        strcpy(variable_root[0].name, bufdata(&tmp_name));

        /* Value */
        variable_root[0].value = malloc(length);
        memcpy(variable_root[0].value, bufdata(&tmp_value), length);

        variable_root[0].length = length;

        variable_root[0].hash = a.hash;

        variable_count++;
    }
    else if ((ret = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
        sizeof(struct variable), name_cmp)) != NULL)
    {
        DEBUG (2, DEBUG_HDR, "    Already exists, so just update it\n");
        ret->value = (char *) realloc(ret->value, strlen(value)+1);
        strcpy(ret->value, value);
    }
    else
    {
        /* Does not exist so make a new one */

    	DEBUG (3, DEBUG_HDR, "Making new one\n");

//        v = p_idx;
//        idx = (uint32_t)((p_idx - variable_root) / (uint32_t)sizeof(struct variable));
//
//        DEBUG (3, DEBUG_HDR, "Inserting near index <%d>, p_idx <%d>, variable_root <%d>, sizof struct variable <%d>\n",
//            idx, p_idx, variable_root, sizeof(struct variable));
//
//        if (name_cmp(v, &a) < 0)
//        {
//            idx++;
//        }
        // Find the right spot to put this in the variable list
        idx = 0;
        for (uint32_t i = 0; i < variable_count; i++)
        {

        	if (name_cmp(&variable_root[i], &a) < 0)
        	{
        		idx++;
        	}
        	else
        	{
        		break;
        	}
        }


        DEBUG (3, DEBUG_HDR, "Inserting it at index <%d>\n", idx);

        /* Add some room on the end */
        variable_root = (struct variable *)
            realloc(variable_root, sizeof(struct variable) * (variable_count + 1));
        variable_count++; 

        /* tmp is the one to be added in */
        tmp.name = malloc(bufsize(&tmp_name)+1);
        strcpy(tmp.name, bufdata(&tmp_name));

        /* Value */
        tmp.value = malloc(length);
        memcpy(tmp.value, bufdata(&tmp_value), length);
        tmp.length = length;

        tmp.hash = a.hash;

        DEBUG (3, DEBUG_HDR, "Tmp name <%s>, value <%s>\n", tmp.name, tmp.value);

        /* Open up the array to make room for the new one */

        size = (variable_count-idx-1) * (uint32_t)sizeof(struct variable);
        memmove(&variable_root[idx+1], &variable_root[idx], size);

        /* Copy in the new value */
        memcpy(&variable_root[idx], &tmp, sizeof(struct variable));

        /****************
        printf ("Variable dump\n");
        for (i=0; i<variable_count; i++) {
            printf ("    Name <%s>, value <%s>, hash <%d>\n", 
                variable_root[i].name, variable_root[i].value, variable_root[i].hash);
        }
        ****************/
    }

    TRACE ("    Writing (tmp) <%s> = <%s>\n", name, strmkprint(value, length)); 

    buffree(&tmp_name);
    buffree(&tmp_value);

    DEBUG (3, DEBUG_HDR, "Done, num variables <%d>\n", variable_count);
    return;
}

/**********************************************/

//int tmp_variable_get_all(struct buffer *value)
void tmp_variable_get_all(struct buffer *value)
{
//int iret, i;
struct buffer tmp = INIT_BUFFER;
#undef NAME
#define NAME "variable_get_all_tmp()"

    DEBUG (3, DEBUG_HDR, "Getting all tmp variables\n");

    bufcpy(value, "");
    DEBUG (3, DEBUG_HDR, "Looking for <%s>\n", bufdata(&tmp));

    for (uint32_t i=0; i<variable_count; i++)
    {
        DEBUG (4, DEBUG_HDR, "comparing <%s> with <%s>\n", variable_root[i].name, bufdata(&tmp));
        if (strncmp(variable_root[i].name, bufdata(&tmp), bufsize(&tmp)) == 0)
        {

#ifdef oldcode
            if (strlen(variable_root[i].value) == 0) {
                continue;  /* Skip over the empty ones */
            }
#endif

            bufcat(value, "\n");
            bufcat(value, variable_root[i].name+bufsize(&tmp));  /* Lob off the #(x) stuff */
            bufcat(value, "=");
            bufncat(value, variable_root[i].value, variable_root[i].length);
        }
    }
    buffree(&tmp);

 //   DEBUG (3, DEBUG_HDR, "Returning iret <%d>, value <%s>\n", iret, bufdata(value));
 //   return iret;
    return;
}

/************************************/

void tmp_variable_add(const char *name, const char *value, int length)
{
/*int i, idx, size;*/
/*char c;*/
	size_t local_length = (size_t)0;
	static uint32_t variable_count_max = 0;
//struct variable /**v, tmp, */a/*, b, *ret*/;
#define NUM_TMP_VARS 1000  /* Number of tmp variables to create each time */

#undef NAME
#define NAME "tmp_variable_add()"

    /* Just add it to the end of the list, don't sort, will do this later */

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, length <%d>\n", name, length);

    /* if (length == 0) { */
    /* If the length is -1, assume a null terminated string, otherwise use the passed length... 
       remember, the value could have null's and other non-printing chars */
    if (length < 0)
    {
        local_length = strlen(value);
        DEBUG (3, DEBUG_HDR, "Value <%s>, length <%d>\n", value, length);
    }
    else
    {
    	local_length = (size_t)length;
    }

    /* bufcpy(&tmp_value, value); */

//    a.name = name;
//    a.hash = get_hash(a.name);
//    b.name = "";
//    b.hash = 0;

    if (variable_count == 0)
    {
        DEBUG (2, DEBUG_HDR, "    First put ever, create new variable\n"); 
        variable_root = (struct variable *) malloc(sizeof(struct variable) * NUM_TMP_VARS);
        variable_count_max = NUM_TMP_VARS;

    }
    else if (variable_count >= variable_count_max)
    {
        variable_root = (struct variable *)
            realloc(variable_root, sizeof(struct variable) * (variable_count + NUM_TMP_VARS));
        variable_count_max = variable_count_max + NUM_TMP_VARS;
    }

    /* Name */
    variable_root[variable_count].name = malloc(strlen(name)+1);
    strcpy(variable_root[variable_count].name, name);

    /* Value */
    variable_root[variable_count].value = malloc((local_length+1));
    memcpy(variable_root[variable_count].value, value, local_length);
    variable_root[variable_count].length = (uint32_t)local_length;
   
    variable_root[variable_count].hash = get_hash(name);
    variable_count++;

    TRACE ("    Writing (adding tmp) <%s> = <%s>\n", name, strmkprint(value, (uint32_t)local_length));

    DEBUG (3, DEBUG_HDR, "Done, num variables <%d>\n", variable_count);
    return;
}

/**********************************************/

void tmp_variable_sort()
{
#undef NAME
#define NAME "tmp_variable_sort()"

    DEBUG (3, DEBUG_HDR, "Called\n");
    TRACE ("    Sorting (tmp)\n"); 

    qsort(variable_root, variable_count, sizeof(variable_root[0]), name_cmp);

    return;
}

/************************************/

uint32_t tmp_variable_get(const char *name, struct buffer *value)
{
	uint32_t iret = 0;
	struct variable a, *b;
	struct buffer tmp = INIT_BUFFER;

#undef NAME
#define NAME "tmp_variable_get()"

    DEBUG (3, DEBUG_HDR, "Getting variable name <%s>\n", name);

    bufcpy(value, "");

    bufcpy(&tmp, name);
    DEBUG (3, DEBUG_HDR, "Looking for variable <%s>\n", bufdata(&tmp));
    a.name = bufdata(&tmp);
    a.hash = get_hash(a.name);

    b = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
        sizeof(struct variable), name_cmp);
    if (b == NULL)
    {
        iret = 1;
        DEBUG (3, DEBUG_HDR, "Variable does not exist <%s>\n", bufdata(&tmp));
        /* ERROR ("Variable <%s> does not exist\n", bufdata(&tmp)); */
    }
    else
    {
        bufncpy(value, b->value, b->length);
        iret = 0;
    }

    buffree(&tmp);
    TRACE ("    Reading (tmp) <%s> = <%s>\n", name, bufmkprint(value)); 

    DEBUG (3, DEBUG_HDR, "Returning iret <%d>, value <%s>\n", iret, bufdata(value));
    return iret;
}

/**********************************************/

void make_tmp_variables(const char *buffer, const char *no_parse, uint32_t args)
{
	uint32_t /*i, */arg_num = 0;
	uint32_t len = 0;
enum parse_types parse_type = PT_NOT_ASSIGNED_YET;
const char *buf;
char num[64];
struct buffer x = INIT_BUFFER;
struct buffer token = INIT_BUFFER;
#undef NAME
#define NAME "tmp_make_variables()"

    DEBUG (3, DEBUG_HDR, "Called, <%s>, no parse <%s>\n", buffer, no_parse);

    arg_num = 0;
    buf = buffer; 

    tmp_variable_delete_all();  /* Delete all tmp variables */

    if (args)
    {
        /* Variable ARG is the entire buffer */
        tmp_variable_put("ARGS", buffer, (uint32_t)strlen(buffer));
    }

    while (1) {
        len = parse(&token, buf, &parse_type, no_parse, false, false);
        buf+=len;
        DEBUG (4, DEBUG_HDR, "Parsed <%s>, type <%s>\n", bufdata(&token), show_parse_type(parse_type));

        if ((parse_type == PT_NULL) || (parse_type == PT_END_OF_LINE)) {
            break;
        }
        bufcpy(&x, bufdata(&token));

        len = parse(&token, buf, &parse_type, no_parse, false, false);
        DEBUG (4, DEBUG_HDR, "Got token <%s>, type <%s>\n", bufdata(&token), show_parse_type(parse_type));

        if (strcmp(bufdata(&token), "=") == 0) {
            /* Name=Value */
            buf+=len;
            len = parse(&token, buf, &parse_type, no_parse, false, false);
            buf+=len;
            DEBUG (4, DEBUG_HDR, "Parsed <%s>, type <%s>\n", bufdata(&token), show_parse_type(parse_type));
            tmp_variable_put(bufdata(&x), bufdata(&token), bufsize(&token)); 
        
        } else {
            /* Just a value by itself */
            bufcpy(&token, "ARG");
            sprintf(num, "%d", arg_num);
            bufcat(&token, num);
            tmp_variable_put(bufdata(&token), bufdata(&x), bufsize(&x)); 
            arg_num++;
        }
    }

    bufcpy(&x, "NUM_ARG");
    sprintf(num, "%d", arg_num);
    bufcpy(&token, num);
    tmp_variable_put(bufdata(&x), bufdata(&token), bufsize(&token)); 

    buffree(&token);
    buffree(&x);

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/**********************************************/

void tmp_variable_dump(struct buffer *buffer, const char *variable)
{

#undef NAME
#define NAME "tmp_variable_dump()"

    DEBUG (3, DEBUG_HDR, "Called, variable <%s>\n", variable);

    bufcat(buffer, "Temp variables: <variable>=<value>\n");
    for (uint32_t i=0; i<variable_count; i++)
    {
        if (0)
        {
        /* if (*variable_root[i].name == '#') { */
        /*     continue; */
        	;
        }
        else if (strstr(variable_root[i].name, variable) != (char *)0)
        {
            bufcat(buffer, "           ");
            bufcat(buffer, variable_root[i].name);
            bufcat(buffer, "=<");
            bufncat(buffer, variable_root[i].value, variable_root[i].length); 
            bufcat(buffer, ">\n");
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/**********************************************/

void tmp_variable_dump_all(struct buffer *buffer)
{
	char tmp[16];

#undef NAME
#define NAME "tmp_variable_dump_all()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    sprintf(tmp, "%d ", variable_count);
    bufcat(buffer, tmp);
    bufcat(buffer, "Temp variables: <variable>=<value>\n");
    for (uint32_t i=0; i<variable_count; i++)
    {
        bufcat(buffer, "           ");
        bufcat(buffer, variable_root[i].name);
        bufcat(buffer, "=<");
        bufncat(buffer, variable_root[i].value, variable_root[i].length); 
        bufcat(buffer, ">\n");
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/**********************************************/

void tmp_variable_delete_all()
{

#undef NAME
#define NAME "tmp_variable_delete_all()"

    TRACE ("    Deleting (all tmp)\n"); 
    DEBUG (3, DEBUG_HDR, "Called\n");

    for (uint32_t i=0; i<variable_count; i++)
    {
        free(variable_root[i].name);
        free(variable_root[i].value);

        variable_root[i].name = (char *)NULL;
        variable_root[i].value = (char *)NULL;
    }

    free(variable_root); 
    variable_root = (struct variable *)NULL;
    variable_count = 0;

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/**********************************************/

static int name_cmp(const void *i, const void *j)
{
#undef NAME
#define NAME "name_cmp()"

    /***************************
    printf ("    Cmping i <%s>, j <%s>, ", 
        ((struct variable *) i)->name, ((struct variable *) j)->name);

    printf ("hash values i <%d>, j <%d>\n", 
        ((struct variable *) i)->hash, ((struct variable *) j)->hash);
    ***************************/

    /* Note that the j is the constant side, and i is the variable side */

//    p_idx = (struct variable *) j;

    if (((struct variable *) i)->hash < ((struct variable *) j)->hash) return -1;
    if (((struct variable *) i)->hash > ((struct variable *) j)->hash) return 1;

    /* Hash values are equal, so compare the names */

    /***************************
    DEBUG (5, DEBUG_HDR, "    Doing string cmp\n");
    ***************************/

    return strcmp(((struct variable *) i)->name, ((struct variable *) j)->name);

}

/**********************************************/

static uint32_t get_hash(const char *name)
{
	uint32_t i_hash = 0;

#undef NAME
#define NAME "get_hash()"

    /* This one has lots of duplicates, good for testing */
    /* for (i=0; i<strlen(name); i++) i_hash = i_hash + (unsigned char) name[i]; */

    /* This one, although not perfect, is very good */
    for (uint32_t i=0; i<strlen(name); i++)
    {
    	i_hash = (uint8_t)name[i] + (31 * i_hash);
    }

    DEBUG (3, DEBUG_HDR, "Hash <%s>, <%d>\n", name, i_hash);
    return i_hash;
}

/**********************************************/

