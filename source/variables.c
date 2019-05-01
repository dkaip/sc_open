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
#include <stdbool.h>
#include "include.h"

extern void log_trace(const char *fmt, ...);

struct variable
{
    char *name;
    char *value;
    uint32_t length;
    enum var_types type;
    uint32_t hash;
};

enum choice
{
	ADD_IT = 1,
	UPDATE_IT,
	DO_NOTHING
};

//static struct variable *p_idx 			= (struct variable *)NULL;

/*
 * The following is a sorted array.  It is not, however, ordered by variable
 * names, but, instead it is ordered by the variable hash values.  If there is
 * a has collision a string comparison on the variable name is used for
 * resolution.
 */
static struct variable *variable_root 	= (struct variable *)NULL;
static uint32_t variable_count 			= 0;

static int name_cmp(const void *, const void *);
static const char *show_var_type(enum var_types);
static uint32_t get_hash(const char *);

/**********************************************/

void variable_put(const char *name, const char *value, uint32_t length, enum var_types type)
{
	uint32_t size = 0;
//	bool update_it = false;
//	bool add_it = false;
	uint32_t idx = 0;
	enum choice what_to_do = ADD_IT;
	char /*c, */tmp_name_prefix[128], *label_ptr;
	struct variable tmp, a, *ret;
	struct buffer tmp_name = INIT_BUFFER;
	struct buffer tmp_value = INIT_BUFFER;

#undef NAME
#define NAME "variable_put()"

    DEBUG (3, DEBUG_HDR, "Called, name <%s>, value <%s>, length <%d>, type <%s>\n", 
        name, value, length, show_var_type(type)); 

    if (length == 0)
    {
        /* Assumes a null terminated value */
        length = (uint32_t)strlen(value);
    }
    bufncpy(&tmp_value, value, length);

    label_ptr = label_get(get_program_token_idx());
//    update_it = add_it = 0;  /* Init the flags */


    while (1)
    {
        if (variable_count == 0)
        {
            /* First one */
            sprintf(tmp_name_prefix, "%s(%s)", var_prefix[type], get_program_name());
            bufcpy(&tmp_name, tmp_name_prefix); 
            bufcat(&tmp_name, name); 
            a.name = bufdata(&tmp_name);
            a.hash = get_hash(a.name);
            DEBUG (3, DEBUG_HDR, "First one added, name <%s>\n", a.name);
            TRACE ("    Writing (first time) <%s> = (%d) <%s>, type <%s>\n", 
                bufdata(&tmp_name), length, strmkprint(value, length), show_var_type(type)); 

            /* First one */
            variable_root = (struct variable *) malloc(sizeof(struct variable));

            /* Name */
            variable_root[variable_count].name = malloc(bufsize(&tmp_name)+1);
            strcpy(variable_root[variable_count].name, bufdata(&tmp_name));

            /* Value */
            variable_root[variable_count].value = malloc(length);
            memcpy(variable_root[variable_count].value, bufdata(&tmp_value), length);
            variable_root[variable_count].length = length;
 
            variable_root[variable_count].type = type;
            variable_root[variable_count].hash = a.hash;
            variable_count++;

            what_to_do = DO_NOTHING;
            break;
        }

        if (type == VT_USER_GLOBAL)
        {
            sprintf(tmp_name_prefix, "%s(%s)", var_prefix[type], get_default_program_name());
            bufcpy(&tmp_name, tmp_name_prefix); 
            bufcat(&tmp_name, name); 
            a.name = bufdata(&tmp_name);
            a.hash = get_hash(a.name);
            DEBUG (3, DEBUG_HDR, "User global, name is <%s>\n", a.name);
            ret = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
                sizeof(struct variable), name_cmp);
            if (ret == NULL)
            {
                /* Not found */
//                add_it = true;
                what_to_do = ADD_IT;
            }
            else
            {
//                update_it = true;
                what_to_do = UPDATE_IT;
            }
            break;
        }

        if (type == VT_USER_LOCAL)
        {
            if (strlen(label_ptr))
            {
                sprintf(tmp_name_prefix, "%s(%s.%s)", var_prefix[type], get_program_name(), label_ptr);
            }
            else
            {
                sprintf(tmp_name_prefix, "%s(%s)", var_prefix[type], get_program_name());
            }
            bufcpy(&tmp_name, tmp_name_prefix); 
            bufcat(&tmp_name, name); 
            a.name = bufdata(&tmp_name);
            a.hash = get_hash(a.name);
            DEBUG (3, DEBUG_HDR, "User local, name is <%s>\n", a.name);
            ret = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
                sizeof(struct variable), name_cmp);
            if (ret == NULL)
            {
                /* Not found */
//                add_it = true;
                what_to_do = ADD_IT;
            }
            else
            {
//                update_it = true;
                what_to_do = UPDATE_IT;
            }
            break;
        }

        /* If within a label, look here first */

        if (strlen(label_ptr))
        {
            sprintf(tmp_name_prefix, "%s(%s.%s)", var_prefix[type], get_program_name(), label_ptr);
            bufcpy(&tmp_name, tmp_name_prefix); 
            bufcat(&tmp_name, name); 
            a.name = bufdata(&tmp_name);
            a.hash = get_hash(a.name);
            DEBUG (3, DEBUG_HDR, "Looking for name <%s>\n", a.name);
            ret = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
                sizeof(struct variable), name_cmp);
            if (ret == NULL)
            {
                /* Not found, but keep on trying... maybe */
                if (type == VT_USER_REF)
                {
//                    add_it = true;
                    what_to_do = ADD_IT;
                    break;
                }
            }
            else
            {
//                update_it = true;
                what_to_do = UPDATE_IT;
                break;
            }
        }

        /* Look at the current program level */

        sprintf(tmp_name_prefix, "%s(%s)", var_prefix[type], get_program_name());
        bufcpy(&tmp_name, tmp_name_prefix); 
        bufcat(&tmp_name, name); 
        a.name = bufdata(&tmp_name);
        a.hash = get_hash(a.name);
        DEBUG (3, DEBUG_HDR, "Looking for name <%s>\n", a.name);
        ret = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
            sizeof(struct variable), name_cmp);
        if (ret == NULL)
        {
            /* Not found */
//            add_it = true;
            what_to_do = ADD_IT;
        }
        else
        {
//            update_it = true;
            what_to_do = UPDATE_IT;
        }

        break;
    }  /* End of while */

    /* Done searching */

    if (what_to_do == UPDATE_IT)
    {

        if (type == VT_USER_REF)
        {
            TRACE ("    Writing (updating) <%s> = (%d) <%s>, type <%s>\n", 
                bufdata(&tmp_name), length, strmkprint(value, length), show_var_type(ret->type)); 

            ret->value = (char *) realloc(ret->value, length);
            memcpy(ret->value, value, length);
            ret->length = length;
            /* ret->type = type; DO NOT CHANGE THE TYPE */

        }
        else if (ret->type == VT_USER_REF)
        {
            bufncpy(&tmp_name, ret->value, ret->length); 
            a.name = bufdata(&tmp_name);
            a.hash = get_hash(a.name);

            /* TRACE ("    Reading <%s> = (%d) <%s>, type <%s>\n", 
                bufdata(&tmp_name), bufsize(value), bufmkprint(value), show_var_type(b->type));  */

            ret = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
                sizeof(struct variable), name_cmp);
            if (ret == NULL)
            {
                ERROR ("Variable referenced <%s> not found\n", a.name);
            }

            TRACE ("    Writing (updating) <%s> = (%d) <%s>, type <%s>\n", 
                bufdata(&tmp_name), length, strmkprint(value, length), show_var_type(ret->type)); 

            ret->value = (char *) realloc(ret->value, length);
            memcpy(ret->value, value, length);
            ret->length = length;

        }
        else if (ret->type == VT_USER_DEFINE)
        {
            ERROR ("Can't modify previously \"defined\" variable <%s>\n", ret->name);

        }
        else if (ret->type == VT_LABEL)
        {
            ERROR ("Label <%s> already defined\n", ret->name);
            FATAL ("Duplicate labels are not recommended\n");

        }
        else
        {
            TRACE ("    Writing (updating) <%s> = (%d) <%s>, type <%s>\n", 
                bufdata(&tmp_name), length, strmkprint(value, length), show_var_type(ret->type)); 

            ret->value = (char *) realloc(ret->value, length);
            memcpy(ret->value, value, length);
            ret->length = length;
            /* ret->type = type; DO NOT CHANGE THE TYPE */
        }
    }
    else
    if (what_to_do == ADD_IT)
    {
        /* Does not exist so make a new one */
        TRACE ("    Writing (adding) <%s> = (%d) <%s>, type <%s>\n", 
            bufdata(&tmp_name), length, strmkprint(value, length), show_var_type(type)); 

//        v = p_idx;
//        idx = (uint32_t)((p_idx - variable_root) / (uint32_t)sizeof(struct variable));
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

        /* Add some room on the end */
        variable_root = (struct variable *)
            realloc(variable_root, sizeof(struct variable) * (variable_count + 1));
        variable_count++; 

        /* tmp is the one to be added in */

        /* Name */
        tmp.name = malloc(bufsize(&tmp_name)+1);
        strcpy(tmp.name, bufdata(&tmp_name));

        /* Value */
        tmp.value = malloc(length);
        memcpy(tmp.value, bufdata(&tmp_value), length);
        tmp.length = length;

        tmp.hash = a.hash;
        tmp.type = type;

        /* Open up the array to make room for the new one */

        size = (variable_count-idx-1) * (uint32_t)sizeof(struct variable);
        memmove(&variable_root[idx+1], &variable_root[idx], size);

        /* Copy in the new value */
        memcpy(&variable_root[idx], &tmp, sizeof(struct variable));
    }
    else
    if (what_to_do == DO_NOTHING)
    {
    	;
    }
    else
    {
    	FATAL ("what_to_do has an unexpected value of %d", what_to_do);
    }

    buffree(&tmp_name);
    buffree(&tmp_value);

//    for (uint32_t i = 0; i < variable_count; i++)
//    {
//    	printf("Index %u, Hash %u, Variable name = \"%s\", variable value = \"%s\"\n", i, variable_root[i].hash, variable_root[i].name, variable_root[i].value);
//    }

    DEBUG (3, DEBUG_HDR, "Done, num variables <%d>\n", variable_count);
    return;
}

/************************************/

uint32_t variable_get(const char *name, struct buffer *value, enum var_types type)
{
	uint32_t iret = 0;
	bool found_it = false;
	char *label_ptr = (char *)NULL;
	char tmp_name_prefix[128];
	struct variable a, *b;
	struct buffer tmp = INIT_BUFFER;

#undef NAME
#define NAME "variable_get()"

    DEBUG (3, DEBUG_HDR, "Getting variable name <%s>, <%s>\n", name, show_var_type(type));

    iret = 0;
    bufcpy(value, "");
    label_ptr = label_get(get_program_token_idx());

    found_it = false;
    while (1)
    {
        /* If within a label, look here first */

//        if (strlen(label_ptr))
        if (label_ptr[0] != 0)
        {
            /* Within a label */
            sprintf(tmp_name_prefix, "%s(%s.%s)", var_prefix[type], get_program_name(), label_ptr);
            bufcpy(&tmp, tmp_name_prefix);
            bufcat(&tmp, name);
            a.name = bufdata(&tmp);
            a.hash = get_hash(a.name);
            b = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
                sizeof(struct variable), name_cmp);
            if (b == NULL)
            {
                /* Not found, but keep on trying */
            	;
            }
            else
            {
                found_it = true;
                break;
            }
        }

        /* Look at the current program level */

        sprintf(tmp_name_prefix, "%s(%s)", var_prefix[type], get_program_name());
        bufcpy(&tmp, tmp_name_prefix);
        bufcat(&tmp, name);
        a.name = bufdata(&tmp);
        a.hash = get_hash(a.name);
        b = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
            sizeof(struct variable), name_cmp);
        if (b == NULL)
        {
            /* Not found, but keep on trying */
        	;
        }
        else
        {
            found_it = true;
            break;
        }

        /* Look in the default program */

        sprintf(tmp_name_prefix, "%s(%s)", var_prefix[type], get_default_program_name());
        bufcpy(&tmp, tmp_name_prefix);
        bufcat(&tmp, name);
        a.name = bufdata(&tmp);
        a.hash = get_hash(a.name);
        b = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
            sizeof(struct variable), name_cmp);
        if (b == NULL)
        {
            /* Not found */
            iret = 1;
        }
        else
        {
            found_it = true;
            break;
        }

        break;
    }

    if (found_it == true)
    {
        if (type == VT_USER_REF)
        {
            /* Return the variable's name, not its value */
            bufncpy(value, bufdata(&tmp), bufsize(&tmp));
        }
        else if (b->type == VT_USER_REF)
        {
            bufncpy(&tmp, b->value, b->length);
            a.name = bufdata(&tmp);
            a.hash = get_hash(a.name);
            b = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
                sizeof(struct variable), name_cmp);
            if (b == NULL)
            {
                iret = 1;
            }
            else
            {
                bufncpy(value, b->value, b->length);
            }
        }
        else
        {
            bufncpy(value, b->value, b->length);
        }

        TRACE ("    Reading <%s> = (%d) <%s>, type <%s>\n", 
            bufdata(&tmp), bufsize(value), bufmkprint(value), show_var_type(b->type)); 
    }

    DEBUG (3, DEBUG_HDR, "Returning iret <%d>, value <%s>\n", iret, bufdata(value));
    buffree(&tmp);

    return iret;
}

/************************************/

void variable_delete(const char *name)
{
	struct variable a;
	struct variable *b = (struct variable *)NULL;
	struct buffer tmp = INIT_BUFFER;

#undef NAME
#define NAME "variable_delete()"

    DEBUG (2, DEBUG_HDR, "Deleting <%s>\n", name);

    bufcpy(&tmp, name);
    bufprefix(&tmp, get_program_name());
    a.name = bufdata(&tmp);
    a.hash = get_hash(a.name);
    DEBUG (3, DEBUG_HDR, "Looking to delete <%s>, last char <%c>\n", 
        a.name, name[strlen(name)-1]);

    if (name[strlen(name)-1] == '*') {
        DEBUG (3, DEBUG_HDR, "Wild card delete\n");
        for (uint32_t i=0; i<variable_count; i++)
        {
            if (strncmp(a.name, variable_root[i].name, strlen(a.name)-1) == 0)
            {
                DEBUG (3, DEBUG_HDR, "Deleted <%s>\n", variable_root[i].name);
                free(variable_root[i].name);
                free(variable_root[i].value);
                variable_root[i].name = "";
                variable_root[i].value = "";
                variable_root[i].hash = 0;
            }
        }
    }
    else
    {
        DEBUG (3, DEBUG_HDR, "Exact match delete\n");
        b = (struct variable *) bsearch((void *) &a, (void *) variable_root, variable_count,
            sizeof(struct variable), name_cmp);
        if (b == (struct variable *)NULL)
        {
            DEBUG (3, DEBUG_HDR, "Can't find it\n");
            WARNING ("Variable <%s> <%s> not deleted, because it was not found\n", name, a.name);
        }
        else
        {
            DEBUG (3, DEBUG_HDR, "Deleted <%s>\n", b->name);
            free(b->name);
            free(b->value);
            b->name = "";
            b->value = "";
            b->hash = 0;
        }
    }

    DEBUG (2, DEBUG_HDR, "    Sorting\n");
    qsort(variable_root, variable_count, sizeof(variable_root[0]), name_cmp);

    buffree(&tmp);
    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/**********************************************/

void variable_dump(struct buffer *buffer, char *variable)
{
	struct buffer tmp_buf = INIT_BUFFER;
	char tmp[128];

#undef NAME
#define NAME "variable_dump()"

    DEBUG (3, DEBUG_HDR, "Called, variable <%s>\n", variable);

    bufcat(buffer, "Variables: <type>(<program>.<label>)<variable> = (len) <value>\n");
    for (uint32_t i=0; i<variable_count; i++)
    {
//        if (0)
//        {
//        /* if (*variable_root[i].name == '#') { */
//        /*     continue; */
//
//        }
//        else if (strstr(variable_root[i].name, variable) != '\0')
        if (strstr(variable_root[i].name, variable) != (char *)0)
        {
            sprintf(tmp, "%8s", show_var_type(variable_root[i].type));
            /* bufcat(buffer, show_var_type(variable_root[i].type)); */
            bufcat(buffer, tmp);
            bufcat(buffer, " ");
            bufcat(buffer, variable_root[i].name);
            bufcat(buffer, " = ");

            /* Size */
            sprintf(tmp, "(%d) ", variable_root[i].length);
            bufcat(buffer, tmp);

            /* Value */
            /* bufcat(buffer, variable_root[i].value);  */
            bufcat(buffer, "<");
            bufncpy(&tmp_buf,  variable_root[i].value, variable_root[i].length); 
            bufncat(buffer, bufdata(&tmp_buf), bufsize(&tmp_buf)); 
            bufcat(buffer, ">\n");
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
    buffree(&tmp_buf);
    return;
}

/**********************************************/

void variable_dump_all(struct buffer *buffer)
{
	struct buffer tmp_buf = INIT_BUFFER;
	char tmp[128];

#undef NAME
#define NAME "variable_dump_all()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    sprintf(tmp, "%d ", variable_count);
    bufcat(buffer, tmp);
    bufcat(buffer, "Variables: <type>(<program>.<label>)<variable> = (len) <value>\n");
    for (uint32_t i=0; i<variable_count; i++)
    {
//    	sprintf(tmp, "Pos %4u ", i);
//        bufcat(buffer, tmp);
        sprintf(tmp, "%8s", show_var_type(variable_root[i].type));
        /* bufcat(buffer, show_var_type(variable_root[i].type)); */
        bufcat(buffer, tmp);
        bufcat(buffer, " ");
        bufcat(buffer, variable_root[i].name);
        bufcat(buffer, " = ");

        /* Size */
        sprintf(tmp, "(%d) ", variable_root[i].length);
        bufcat(buffer, tmp);

        /* Value */
        /* bufcat(buffer, variable_root[i].value);  */
        bufcat(buffer, "<");
        bufncpy(&tmp_buf,  variable_root[i].value, variable_root[i].length); 
        bufncat(buffer, bufdata(&tmp_buf), bufsize(&tmp_buf)); 
        bufcat(buffer, ">\n");
    }

    DEBUG (3, DEBUG_HDR, "Returning\n");
    buffree(&tmp_buf);
    return;
}

/**********************************************/

void variable_delete_all()
{

#undef NAME
#define NAME "variable_delete_all()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    for (uint32_t i=0; i<variable_count; i++)
    {
        free(variable_root[i].name);
        free(variable_root[i].value);
    }

    if (variable_root != (struct variable *)NULL)
    	free(variable_root);

    variable_root = (struct variable *)NULL;
    variable_count = 0;

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/**********************************************/

/*
 * This function implies that it is a name comparator, but, it is not.  This
 * comparator compares the hashes of the variables and only uses the "name"
 * for resolution in the case of a hash collision.
 */
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

//printf("p_idx hash %u, name = %s\n", ((struct variable *) j)->hash, ((struct variable *) j)->name);

    if (((struct variable *) i)->hash < ((struct variable *) j)->hash) return -1;
    if (((struct variable *) i)->hash > ((struct variable *) j)->hash) return 1;

    /* Hash values are equal, so compare the names */

    /***************************
    DEBUG (5, DEBUG_HDR, "    Doing string cmp\n");
    ***************************/

    return strcmp(((struct variable *) i)->name, ((struct variable *) j)->name);

}

/**********************************************/

static const char *show_var_type(enum var_types type)
{
#undef NAME
#define NAME "show_var_type()"

    if (type == VT_USER)             return "USER";
    if (type == VT_USER_REF)         return "REF";
    if (type == VT_SC)               return "SC";
    if (type == VT_LABEL)            return "LABEL";
    if (type == VT_USER_DEFINE)      return "DEFINE";
    if (type == VT_USER_GLOBAL)      return "GLOBAL";
    if (type == VT_USER_LOCAL)       return "LOCAL";

    FATAL ("Invalid variable type <%d>\n", type); 
    return "OOOOps";
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
    for (uint32_t i=0; i<(uint32_t)strlen(name); i++)
    {
    	i_hash = (uint8_t)name[i] + (31 * i_hash);
    }

    DEBUG (3, DEBUG_HDR, "Hash <%s>, <%d>\n", name, i_hash);
    return i_hash;
}

/**********************************************/

