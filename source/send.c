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
#include <stdlib.h>
#include <stdint.h>

#include "include.h"

//extern int cmd_send(int *wait_flag);

/********************************************/

uint32_t cmd_send(bool *wait_flag)
{
	uint32_t iret = 0;
	int fd = 0;
	int client_fd = 0;
	enum protos proto = PROTO_UNKNOWN;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;
	char name[128], options[128];

#undef NAME
#define NAME "cmd_send()"

	name[0] = 0;
	options[0] = 0;
    DEBUG (3, DEBUG_HDR, "Called\n");

//    iret = 0;
//    strcpy(name, "");
//    strcpy(options, "");

    while (1)
    {
        get_program_token(&token, &type);
        if (bufcmp(&token, "name") == 0)
        {
            get_program_token(&token, &type);
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);

                if (strlen(bufdata(&token)) >= sizeof(name))
                {
                    ERROR ("Name <%s> is too long, limited to <%d>\n", sizeof(name));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(name, bufdata(&token));
                    if (!g_syntax_check)
                    {
                        fd = get_fd(name);
                    }
                }
            }
            else
            {
                put_program_token();
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1;
                break;
            }
        }
        else if (bufcmp(&token, "client") == 0)
        {
            get_program_token(&token, &type);
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);
                client_fd = atoi(bufdata(&token));
            }
            else
            {
                put_program_token();
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1;
                break;
            }
        }
        else if (bufcmp(&token, "options") == 0)
        {
            get_program_token(&token, &type);
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);

                if (strlen(bufdata(&token)) >= sizeof(options))
                {
                    ERROR ("Name <%s> is too long, limited to <%d>\n", sizeof(options));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(options, bufdata(&token));
                }
            }
            else
            {
                put_program_token();
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1;
                break;
            }
        }
        else
        {
            put_program_token();
            break;
        }
    }

    if (client_fd != 0)
    {
        fd = client_fd;
    }

    if (!g_syntax_check)
    {
        proto = get_proto(name);
        override_options(name, options); 
    }

    if (g_syntax_check)
    {

    } else if (proto == PROTO_SECS)
    {
        iret = send_secs(name, fd, wait_flag);
        if (iret)
        {
            ERROR ("Error sending secs message\n");;
        }
    }
    else if (proto == PROTO_HSMS)
    {
        iret = send_hsms(name, fd, wait_flag);
        if (iret)
        {
            ERROR ("Error sending hsms message\n");;
        }
    }
    else if (proto == PROTO_STANDARD)
    {
        iret = send_standard(name, fd);
        if (iret)
        {
            ERROR ("Error sending standard message\n");;
        }
    }
    else if (proto == PROTO_XML)
    {
        iret = send_xml(name, fd);
        if (iret)
        {
            ERROR ("Error sending xml message\n");;
        }
    }
    else if (proto == PROTO_MBX)
    {
        iret = mbx_send(name, proto);
    }
    else if (proto == PROTO_MBX_VFEI_20)
    {
        iret = mbx_send(name, proto);
    }
    else
    {
        FATAL ("No handler command defined for fd, <%d>, proto <%s>\n", 
            fd, show_proto(proto));
    }

    /* override_options(name, ""); */
    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t cmd_send_reply()
{
	uint32_t iret = 0;
	int fd = 0;
	char name[128];
	enum protos proto = PROTO_UNKNOWN;
	enum token_types type = TT_NOT_ASSIGNED_YET;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "cmd_send_reply()"

	name[0] = 0;
    DEBUG (3, DEBUG_HDR, "Called\n");

    while (1)
    {
        get_program_token(&token, &type);
        if (bufcmp(&token, "name") == 0)
        {
            get_program_token(&token, &type);
            if (type == TT_ASSIGN)
            {
                get_program_token(&token, &type);

                if (strlen(bufdata(&token)) >= sizeof(name))
                {
                    ERROR ("Name <%s> is too long, limited to <%d>\n", sizeof(name));
                    iret = 1;
                    break;
                }
                else
                {
                    strcpy(name, bufdata(&token));
                }
            }
            else
            {
                put_program_token();
                ERROR ("Expecting \"=\", found <%s>\n", bufdata(&token));
                iret = 1;
                break;
            }
        }
        else
        {
            put_program_token();
            break;
        }
    }

    if (!g_syntax_check)
    {
        proto = get_proto(name);
    }

    if (proto == PROTO_SECS)
    {
        iret = send_secs_reply(name, get_client_fd(name));
    }
    else if (proto == PROTO_HSMS)
    {
        iret = send_hsms_reply(name, get_client_fd(name));
    }
    else if (proto == PROTO_STANDARD)
    {
        iret = send_standard_reply(name, get_client_fd(name));
    }
    else if (proto == PROTO_XML)
    {
        iret = send_xml_reply(name, get_client_fd(name));
    }
    else if (proto == PROTO_MBX)
    {
        iret = mbx_send_reply(name, proto);
    }
    else if (proto == PROTO_MBX_VFEI_20)
    {
        iret = mbx_send_reply(name, proto);
    }
    else
    {
        FATAL ("No reply command defined for fd, <%d>, proto <%s>\n", 
            fd, show_proto(proto));
    }

    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    buffree(&token);
    return iret;
}

/********************************************/


