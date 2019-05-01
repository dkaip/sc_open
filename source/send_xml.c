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

/********************************************/

uint32_t send_xml(const char *name, int fd)
{
	uint32_t iret = 0;
	char *option = (char *)NULL;
	struct buffer token = INIT_BUFFER;
	struct buffer reply = INIT_BUFFER;

#undef NAME
#define NAME "send_xml()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    iret = expression(&token);
    if (iret)
    {
        ERROR ("Error with expression in SEND statement\n");

    }
    else
    {
        LOG ("Send to <%s>, <%s>\n", get_name(fd), bufdata(&token));
        if (my_write(name, fd, (const uint8_t *)bufdata(&token), bufsize(&token)))
        {
            ERROR ("Error sending reply <%d>, <%s>\n", fd, bufdata(&token));
            iret = 1;
        }
    }

    if (iret)
    {
        ;
    }
    else
    {
        /* See if a reply is expected */
        option = get_option("wait_for_reply=", name);
        if (strcmp(option, "no") == 0)
        {
            /* No reply is expected */
        	;
        }
        else
        {
            bufcpy(&reply, "");
            iret = recv_xml(name, fd, &reply);
            LOG ("Recv reply, <%s>\n", bufdata(&reply));
            /* DEBUG (1, DEBUG_HDR, "Received reply iret <%d>, <%s>\n", iret, bufdata(&reply)); */
            if (iret)
            {
            	;
            }
            else
            {
                /* variable_put("ARGS", bufdata(&reply), VT_TMP); */
            }
        }
    }

    buffree(&token);
    buffree(&reply);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t send_xml_reply(const char *name, int fd)
{
	uint32_t iret = 0;
	struct buffer token = INIT_BUFFER;

#undef NAME
#define NAME "send_xml_reply()"

    DEBUG (3, DEBUG_HDR, "Called\n");

    iret = expression(&token);
    if (iret)
    {
        ERROR ("Error with expression in SEND_REPLY statement\n");
    }
    else
    {
        LOG ("Send reply to <%d>, <%s>\n", fd, bufdata(&token));
        if (my_write(name, fd, (const uint8_t *)bufdata(&token), bufsize(&token)))
        {
            ERROR ("Error sending reply <%d>, <%s>\n", fd, bufdata(&token));
            iret = 1;
        }
    }

    buffree(&token);
    DEBUG (3, DEBUG_HDR, "Returning <%d>\n", iret);
    return iret;
}

/********************************************/

uint32_t recv_xml(const char *name, int fd, struct buffer *reply)
{
	uint32_t iret = 0;
	uint32_t timeout = 0;
	uint32_t tag_state = 0;
	char *option = (char *)NULL;
	uint8_t c = 0;
	char tag[256];
	static jmp_buf timer;

#undef NAME
#define NAME "recv_xml()"

	tag[0] = 0;

    DEBUG (3, DEBUG_HDR, "Called, fd <%d>\n", fd);

    // TODO need a bufclear...
    bufcpy(reply, "");

    option = get_option("timeout=", name);
    if (strlen(option))
    {
        timeout = (uint32_t)atoi(option);
        DEBUG (3, DEBUG_HDR, "Timeout specififed <%d>\n", timeout);
    }
    else
    {
        timeout = 60;  /* Default timeout, avoids waiting forever */
        DEBUG (3, DEBUG_HDR, "Timeout not specified, using default value <%d>\n", timeout);
    }

    DEBUG (3, DEBUG_HDR, "Waiting for message...\n");

//    if (sigsetjmp(timer, 1) != 0)
    if (do_the_setsigjump(timer, 1) != 0)
    {
        ERROR ("Timeout detected receiving <%s> from <%d>\n", bufdata(reply), fd);
        bufcpy(reply, get_option("timeout_msg=", name));
        return 0;
    }
    start_proto_timer(timeout, 0, &timer);

    tag_state = 0;
    while (1)
    {
        DEBUG (3, DEBUG_HDR, "Reading...\n");
        iret = my_readc(name, fd, &c);  /* Read 1 char at a time */
        if (iret)
        {
            break;
        }
        else
        {
            DEBUG (3, DEBUG_HDR, "Read <%s>\n", bufdata(reply));
            if (tag_state == 0)
            {
                if (c == '<')
                {
                    tag_state = 1;
                    strcpy(tag, "</");
                    bufncat(reply, (char *)&c, 1);
                }
                else
                {
                    DEBUG (3, DEBUG_HDR, "Tossing char <%x>\n", c);
                    continue;
                }
            }
            else if (tag_state == 1)
            {
                if (c == ' ')
                {
                    tag_state = 2;
                    strcat(tag, ">");
                    DEBUG (3, DEBUG_HDR, "Tag is <%s>\n", tag);
                }
                else if (c == '>')
                {
                    tag_state = 2;
                    strcat(tag, ">");
                    DEBUG (3, DEBUG_HDR, "Tag is <%s>\n", tag);
                }
                else
                {
                    strncat(tag, (char *)&c, 1);
                }
                bufncat(reply, (char *)&c, 1);
            }
            else
            {
                bufncat(reply, (char *)&c, 1);
                if (strstr(bufdata(reply), tag) == '\0')
                {
                    /* Not found yet */
                	;
                }
                else
                {
                    break;
                }
            }
        }
    }

    stop_proto_timer(0);

    DEBUG (3, DEBUG_HDR, "Returning <%d>, <%s>\n", iret, bufdata(reply));
    return iret;
}

/********************************************/


