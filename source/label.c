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

struct label
{
    int lower;
    int upper;
    char *name;
};

static struct label *label_root = (struct label *)NULL;
static uint32_t label_count     = 0;

//static void label_dump();
static void label_clear();
static void label_put(char *, int, int);
static int idx_cmp(const void *, const void *);

/**********************************************/

uint32_t label_build_idx_table()
{
	uint32_t iret = 0;
	int32_t lower_idx = 0;
	int32_t upper_idx = 0;
	struct buffer token = INIT_BUFFER;
	struct buffer name = INIT_BUFFER;
	enum token_types type = TT_NOT_ASSIGNED_YET;

#undef NAME
#define NAME "label_build_idx_table()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    set_program_token_idx(0);
    lower_idx = upper_idx = -1;

    label_clear();  /* Clear the table of label idx's */

    while (1)
    {
        get_program_token_no_eval(&token, &type);

        if ((type == TT_AFTER) || (type == TT_LABEL))
        {
            get_program_token_no_eval(&token, &type);
            if (type == TT_IDENTIFIER)
            {
                if (lower_idx == -1)
                {
                    /* This is the first one */
                    buf2bufcpy(&name, &token);
                    lower_idx = (int32_t)get_program_token_idx();

                }
                else
                {
                    upper_idx = (int32_t)get_program_token_idx();
                    label_put(bufdata(&name), lower_idx, upper_idx);
                    lower_idx = upper_idx;
                    buf2bufcpy(&name, &token);
                }

            }
            else
            {
                ERROR ("Expecting IDENTIFIER, found <%s>, type <%s> at line <%d>\n", 
                    bufdata(&token), show_type(type), get_line_number());
                iret = 1;
                break;
            }

        /************************************
            Need to ignore return's because you can have multiple
            returns after a single label... this is allowed 
        } else if(type == TT_RETURN) {
            if (lower_idx == -1) {

            } else {
                upper_idx = get_program_token_idx();
                label_put(bufdata(&name), lower_idx, upper_idx);
                lower_idx = upper_idx = -1;
            }
        ************************************/
        
        }
        else if(type == TT_END)
        {
            if (lower_idx == -1)
            {
            	;
            }
            else
            {
                upper_idx = (int32_t)get_program_token_idx();
                label_put(bufdata(&name), lower_idx, upper_idx);
                lower_idx = upper_idx = -1;
            }
        }
        else if(type == TT_NULL_TOKEN)
        {
            if (lower_idx == -1)
            {

            }
            else
            {
                upper_idx = (int32_t)get_program_token_idx();
                label_put(bufdata(&name), lower_idx, upper_idx);
                lower_idx = upper_idx = -1;
            }
            break; 
        }
    }

    /* label_dump(); */

    /***** Debug code ***********
    int idx;
    set_program_token_idx(0);
    label_dump(); 
    while (1) {
        get_program_token_no_eval(&token, &type);
        idx = get_program_token_idx();
        if (type == TT_NULL_TOKEN) {
            break;
        }
        label_get(99);
        label_get(150);
        printf("Token <%s> <%s>, Idx <%d>, label <%s>\n", 
            bufdata(&token), show_type(type), idx, label_get(idx));
    }
    ************************************/

    buffree(&token);
    buffree(&name);

    return iret;
}

/**********************************************/

//static void label_dump()
//{
//	uint32_t i;
//
//#undef NAME
//#define NAME "label_dump()"
//
//    DEBUG (3, DEBUG_HDR, "Called\n");
//
//    /* printf("Dump of labels <%d>:\n", label_count); */
//    for (i=0; i<label_count; i++)
//    {
//        printf("Label dump i = [%d] <%s>, lower <%d>, upper <%d>\n",
//            i, label_root[i].name, label_root[i].lower, label_root[i].upper);
//    }
//
//    printf("\n");
//    return;
//}
//
/**********************************************/

static void label_clear() 
{
#undef NAME
#define NAME "label_clear()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    for (uint32_t i=0; i<label_count; i++)
    {
        free(label_root[i].name);
        label_root[i].name = (char *)NULL;
    }

    free(label_root);
    label_root = (struct label *)NULL;
    label_count = 0;

    DEBUG (3, DEBUG_HDR, "Returning\n");
    return;
}

/**********************************************/

static void label_put(char *name, int lower, int upper)
{
#undef NAME
#define NAME "label_put()"

    DEBUG (3, DEBUG_HDR, "Called for label <%s> u = %d, l = %d\n", name, upper, lower);

    if (label_count == 0)
    {
        label_root = (struct label *) malloc(sizeof(struct label));

    }
    else
    {
        label_root = (struct label *)
            realloc(label_root, sizeof(struct label) * (label_count + 1));
    }

    label_root[label_count].name = malloc(strlen(name)+1);
    strcpy(label_root[label_count].name, name);
    label_root[label_count].lower = lower;
    label_root[label_count].upper = upper;
    label_count++; 

//    label_dump();
//
    return;
}

/************************************/

char *label_get(uint32_t idx)
{
	struct label a, *ret = (struct label *)NULL;
	char *p_ret = (char *)NULL;

#undef NAME
#define NAME "label_get()"

    DEBUG (3, DEBUG_HDR, "Called, idx <%d>, label_count <%d>\n", idx, label_count);

    a.lower = (int32_t)idx;
    a.upper = 0;
    a.name = "";

    if (label_count == 0)
    {
        p_ret = "";
    }
    else
    {
        ret = (struct label *) bsearch((void *) &a, (void *) label_root, 
            label_count, sizeof(struct label), idx_cmp);

        if (ret == NULL)
        {
            p_ret = "";
        }
        else
        {
            p_ret = ret->name;
        }
    }

    DEBUG (3, DEBUG_HDR, "Returning <%s>\n", p_ret);
    return p_ret;
}

/**********************************************/

static int idx_cmp(const void *i, const void *j)
{
	int idx = 0;

#undef NAME
#define NAME "idx_cmp()"

    /***********************
    printf ("????? idx_cmp() called\n");
    printf ("????? Cmping j lower <%d>, upper <%d>, name <%s>\n", 
        ((struct label *) j)->lower, ((struct label *) j)->upper, ((struct label *) j) ->name);
    printf ("?????   with i lower <%d>\n", ((struct label *) i)->lower);
    ***********************/

    idx = ((struct label *)i)->lower;

    /* i.lower has the idx  */
    
    if (idx < ((struct label *) j)->lower)
    	return -1;

    if (idx > ((struct label *) j)->upper)
    	return 1;

    /* printf("????? Match, i name <%s>, j name <%s>\n", 
        ((struct label *) i)->name, ((struct label *) j)->name); */
    return 0;
}

/**********************************************/










