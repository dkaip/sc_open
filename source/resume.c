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
#include "include.h"

/********************************************/

uint32_t resume_via_program_label(const char *program_label)
{
	uint32_t iret = 0;

#undef NAME
#define NAME "resume_via_program_label()"

    if (program_label == (char *)NULL)
    {
    	FATAL ("Null passed for program_label\n");
    }

    if (strlen(program_label) == 0)
    {
    	FATAL ("Empty string passed for program_label\n");
    }

    LOG ("Resuming at label <%s>\n", program_label);

    DEBUG (3, DEBUG_HDR, "Using passed label <%s>\n", program_label);

	struct buffer value = INIT_BUFFER;
    if (variable_get(program_label, &value, VT_LABEL))
    {
        ERROR ("Can't process label <%s>, label not found in program <%s>\n",
        		program_label, get_program_name());
        iret = 1;
    }
    else
    {
    	uint32_t program_index = 0;
        sscanf(bufdata(&value), "%u", &program_index);
        set_program_token_idx(program_index);

    	struct buffer token = INIT_BUFFER;
    	enum token_types token_type = TT_NOT_ASSIGNED_YET;
    	get_program_token(&token, &token_type);
        iret = execute(&token);
    }

	return iret;
}

uint32_t resume_via_program_idx(uint32_t program_index)
{
	uint32_t iret = 1;

#undef NAME
#define NAME "resume_via_program_idx()"

    DEBUG (3, DEBUG_HDR, "set_program_token_idx with program_idx of <%u>\n", program_index);
    set_program_token_idx(program_index);

	struct buffer token = INIT_BUFFER;
	enum token_types token_type = TT_NOT_ASSIGNED_YET;
	get_program_token(&token, &token_type);
    iret = execute(&token);

    return iret;
}

//uint32_t resume(const char *program_label, uint32_t program_idx)
//{
//	uint32_t num = 0;
//	uint32_t iret = 0;/*, type*/;
////	struct buffer token = INIT_BUFFER;
//	struct buffer value = INIT_BUFFER;
//
//#undef NAME
//#define NAME "resume()"
//
//    DEBUG (3, DEBUG_HDR, "Called, label <%s>, program_idx <%d>\n", program_label, program_idx);
//
//    iret = 0;
//
//    /* LOG ("Resuming at label <%s> or program_idx <%u>\n", label, program_idx); */
////    if (strlen(label))
////    {
////        LOG ("Resuming at label <%s>\n", label);
////    }
//
//    if (strlen(program_label))
//    {
//        LOG ("Resuming at label <%s>\n", program_label);
//
//        DEBUG (3, DEBUG_HDR, "Using passed label <%s>\n", program_label);
//
//        if (variable_get(program_label, &value, VT_LABEL))
//        {
//            ERROR ("Can't process label <%s>, label not found in program <%s>\n",
//            		program_label, get_program_name());
//            iret = 1;
//        }
//        else
//        {
//            sscanf(bufdata(&value), "%u", &num);
//            set_program_token_idx(num);
//        }
//    }
////    else if (program_idx == -1)
////    {
////        FATAL ("No label passed and program_idx is -1, so nothing to do\n");
////    }
//    else
//    {
//        DEBUG (3, DEBUG_HDR, "Using passed program_idx <%d>\n", program_idx);
//        set_program_token_idx(program_idx);
//    }
//
//    if (iret)
//    {
//        /* Do nothing */
//    	;
//    }
//    else
//    {
//        /* make_tmp_variables(buffer); */
//        iret = execute(&token);
//        if (iret)
//        {
//            ERROR ("Error while processing at label\n");
//        }
//    }
//
//    buffree(&token);
//    buffree(&value);
//    DEBUG (3, DEBUG_HDR, "Returning iret <%u>\n", iret);
//
//    return iret;
//}
//
