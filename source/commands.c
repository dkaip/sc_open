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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "include.h"
#include "version.h"

/********************************************/

uint32_t commands(struct buffer *buffer)
{
	uint32_t iret = 0;
	uint32_t len = 0;
	enum parse_types parse_type = PT_NOT_ASSIGNED_YET;
	char num[64], tmp[512], *buf = (char *)NULL;
	char name[512], value[512];
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "commands()"

	num[0] = 0;
	tmp[0] = 0;
	name[0] = 0;
	value[0] = 0;
    DEBUG (3, DEBUG_HDR, "Called, buf <%s>\n", bufdata(buffer));

    LOG ("(%s) Running command <%s>...\n", show_proto(PROTO_COMMANDS), bufdata(buffer));
 
    buf = bufdata(buffer);
    len = parse(&token, buf, &parse_type, "", false, false);
    buf += len;
    buftolower(&token);   
    iret = 0;

    if (bufsize(&token) == 0)
    {
        DEBUG (3, DEBUG_HDR, "Nothing to do\n");
        bufcpy(buffer, SC_CMD_HELP);

    }
    else if (bufncmp(&token, "debug", bufsize(&token)) == 0)
    {
        len = parse(&token, buf, &parse_type, "", false, false);
        buf+=len;
        if (len)
        {
            g_debug = (uint32_t)atoi(bufdata(&token));
        }
        bufcat(buffer, "Debug level set to ");
        sprintf(num, "%d\n", g_debug); 
        bufcat(buffer, num);

    }
    else if (bufncmp(&token, "dump", bufsize(&token)) == 0)
    {
        bufcpy(buffer, "");
        len = parse(&token, buf, &parse_type, "", false, false);
        buf+=len;
        if (parse_type == PT_END_OF_LINE)
        {
            variable_dump_all(buffer);
            tmp_variable_dump_all(buffer);
            dump_server(buffer);
            dump_handler(buffer);
            dump_pragma(buffer);
            dump_mbx(buffer);

        }
        else
        {
            variable_dump(buffer, bufdata(&token));
            tmp_variable_dump(buffer, bufdata(&token));
        }
        /* mbx_dump(); */
        buffilecpywrite(buffer, "dump.out");

    }
    else if (bufncmp(&token, "event", bufsize(&token)) == 0)
    {
        len = parse(&token, buf, &parse_type, "", false, false);
        buf+=len;
        make_tmp_variables(buf, "", 1);
        iret = resume_via_program_label(bufdata(&token));
        if (iret)
        {
            bufcat(buffer, "Error\n");
        }
        else
        {
            bufcat(buffer, "Success with event\n");
        }
    }
    else if (bufncmp(&token, "set", bufsize(&token)) == 0)
    {
        len = parse(&token, buf, &parse_type, "", false, false);
        buf+=len;
        strncpy(name, bufdata(&token), sizeof(name)-1);

        len = parse(&token, buf, &parse_type, "", false, false);
        buf+=len;
        strncpy(value, bufdata(&token), sizeof(value)-1);

        variable_put(name, value, 0, VT_USER_GLOBAL);
        bufcat(buffer, "Success with set\n");

    }
    else if (bufncmp(&token, "help", bufsize(&token)) == 0)
    {
        bufcpy(buffer, SC_CMD_HELP);
    }
    else if (bufncmp(&token, "kill", bufsize(&token)) == 0)
    {
        kill_server();
        LOG ("(%s) Exiting by request\n", show_proto(PROTO_COMMANDS));
        exit (0);
    }
    else if (bufncmp(&token, "ping", bufsize(&token)) == 0)
    {
        bufcpy(buffer, "Server is running\n");
    }
    else if ((bufncmp(&token, "read", bufsize(&token)) == 0) ||
        (bufncmp(&token, "load", bufsize(&token)) == 0))
    {
//    	while(isspace(*buf))
//    	{
//    		/*
//    		 * We are having whitespace issues between the read command and
//    		 * the file specification.  Let's try ato address that here.
//    		 */
//    		buf++;
//    	}

        iret = read_program(buf);

        bufcpy(buffer, "");
        if (iret)
        {
            bufcat(buffer, "Read or Load error\n");
        }
        else
        {
            bufcat(buffer, "Success with read\n");
        }

    }
    else if (bufncmp(&token, "tokens", bufsize(&token)) == 0)
    {
        dump_program_tokens(buffer);
        buffilecpywrite(buffer, "program.tokens");
        bufcat(buffer, "\n");
    }
    else if (bufncmp(&token, "list", bufsize(&token)) == 0)
    {
        list_program(buffer);
        buffilecpywrite(buffer, "program.list");
        bufcat(buffer, "\n");
    }
    else if (bufncmp(&token, "logging", bufsize(&token)) == 0)
    {
        bufcpy(buffer, "");
        len = parse(&token, buf, &parse_type, "", false, false);
        buf+=len;
        if (parse_type == PT_END_OF_LINE)
        {
            /* No parameters, so show all logging */
            show_logging(buffer);
        }
        else
        {
            strcpy(tmp, bufdata(&token));  /* This is the name parameter */    
            /* Now get the value parameter */
            len = parse(&token, buf, &parse_type, "", false, false);
            buf+=len;
            if (get_fd(tmp))
            {
                set_logging(tmp, atoi(bufdata(&token)));
                show_logging(buffer);
            }
            else
            {
                bufcpy(buffer, "Not a valid name for logging <");
                bufcat(buffer, tmp);
                bufcat(buffer, ">\n");
                show_logging(buffer);
            }
        }

    }
    else if (bufncmp(&token, "version", bufsize(&token)) == 0)
    {
        bufcpy(buffer, "sc version " ); 
        bufcat(buffer, SC_VERSION); 
        bufcat(buffer, " "); 
        bufcat(buffer, SC_DATE); 
        bufcat(buffer, "\n");

    }
    else if (parse_type == PT_END_OF_LINE)
    {
        bufcpy(buffer, SC_CMD_HELP);
    }
    else if (parse_type == PT_NULL)
    {
        bufcpy(buffer, SC_CMD_HELP);
    }
    else
    {
        bufcpy(buffer, "Invalid command <");
        bufcat(buffer, bufdata(&token));
        bufcat(buffer, ">\n");
    }
      
    buffree(&token); 
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

